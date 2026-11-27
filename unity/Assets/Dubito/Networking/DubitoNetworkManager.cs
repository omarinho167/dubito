using System;
using System.Collections.Generic;
using Dubito.Core;
using Dubito.Protocol;
using Mirror;
using Mirror.FizzySteam;
using UnityEngine;
using kcp2k;

namespace Dubito.Networking
{
    [RequireComponent(typeof(KcpTransport))]
    public sealed class DubitoNetworkManager : NetworkManager
    {
        [SerializeField] KcpTransport localTransport;
        [SerializeField] FizzySteamworks steamTransport;
        [SerializeField] SteamLobbyService steamLobby;
        HostAuthority authority;
        readonly Dictionary<int, double> closing = new Dictionary<int, double>();
        double lastTick;
        string error;
        bool connected, stopping;
        public NetworkGameSession Session { get; private set; }
        public bool IsHosting => NetworkServer.active;
        public bool IsConnectingOrConnected => NetworkClient.active;
        public bool IsSteam { get; private set; }
        public bool IsSteamReady => steamLobby != null && steamLobby.IsReady;
        public bool IsBusy => IsHosting || IsConnectingOrConnected || (steamLobby != null && steamLobby.IsBusy);
        public ulong SteamLobbyId => steamLobby != null ? steamLobby.LobbyId : 0;
        public int ConnectedPlayers => authority?.PlayerCount ?? 0;
        public bool CanStartMatch => IsHosting && authority != null && authority.CanStart;
        public bool CanRematch => IsHosting && authority != null && authority.IsFinished;
        public int TurnSeconds => authority?.TurnSeconds ?? 45;
        public int MaxPlayers => authority?.MaxPlayers ?? 6;
        public string Status { get; private set; } = "Choisis un mode de jeu.";
        public string LobbyNotice { get; private set; }
        public event Action<NetworkGameSession> SessionCreated;
        public event Action<string> Disconnected;

        public override void Awake()
        {
            autoCreatePlayer = false;
            dontDestroyOnLoad = false;
            maxConnections = 6;
            networkAddress = "127.0.0.1";
            if (localTransport == null) localTransport = GetComponent<KcpTransport>();
            if (steamTransport == null) steamTransport = GetComponent<FizzySteamworks>();
            if (steamLobby == null) steamLobby = GetComponent<SteamLobbyService>();
            localTransport.Port = 7777;
            SelectTransport(false);
            offlineScene = onlineScene = "";
            base.Awake();
        }

        public void HostLocal()
        {
            StopSession();
            SelectTransport(false);
            error = null; connected = false;
            Status = "Démarrage de l'hôte…";
            try { StartHost(); }
            catch (Exception) { Fail("Impossible d'héberger sur le port 7777."); }
        }

        public void JoinLocal()
        {
            StopSession();
            SelectTransport(false);
            error = null; connected = false;
            Status = "Connexion à 127.0.0.1…";
            networkAddress = "127.0.0.1";
            try { StartClient(); }
            catch (Exception) { Fail("Connexion impossible à 127.0.0.1:7777."); }
        }

        public void HostSteam()
        {
            if (steamLobby == null) { Fail(SteamLobbyRules.SteamUnavailable); return; }
            if (!steamLobby.CanBegin()) return;
            StopSession();
            error = null; connected = false;
            IsSteam = true;
            Status = "Création du salon Steam…";
            steamLobby.CreateLobby(MaxPlayers);
        }

        public void JoinSteam(ulong lobbyId)
        {
            if (steamLobby == null) { Fail(SteamLobbyRules.SteamUnavailable); return; }
            if (!steamLobby.CanBegin()) return;
            StopSession();
            error = null; connected = false;
            IsSteam = true;
            Status = "Connexion au salon Steam…";
            steamLobby.JoinLobby(lobbyId);
        }

        public void JoinSteamCode(string code)
        {
            if (steamLobby == null) { Fail(SteamLobbyRules.SteamUnavailable); return; }
            steamLobby.JoinLobby(code);
        }

        public void InviteSteamFriends() => steamLobby?.InviteFriends();

        // The lobby service calls these only after creation/metadata validation.
        internal void StartSteamHost()
        {
            try
            {
                SelectTransport(true);
                Status = "Démarrage de l'hôte Steam…";
                StartHost();
                // Fizzy catches its own startup exceptions; verify it actually opened a server.
                if (!steamTransport.ServerActive()) Fail(SteamLobbyRules.ConnectionFailed);
            }
            catch (Exception) { Fail(SteamLobbyRules.ConnectionFailed); }
        }

        internal void StartSteamClient(ulong hostId)
        {
            try
            {
                SelectTransport(true);
                networkAddress = hostId.ToString(System.Globalization.CultureInfo.InvariantCulture);
                Status = "Connexion à l'hôte Steam…";
                StartClient();
            }
            catch (Exception) { Fail(SteamLobbyRules.ConnectionFailed); }
        }

        void SelectTransport(bool steam)
        {
            if (NetworkServer.active || NetworkClient.active) throw new InvalidOperationException("Transport still active.");
            if (steam && (steamTransport == null || !IsSteamReady)) throw new InvalidOperationException("Steam unavailable.");
            // Fizzy's OnEnable schedules a native relay initialization. Keep it off in local mode.
            if (steamTransport != null)
            {
                steamTransport.CancelInvoke();
                steamTransport.enabled = steam;
            }
            localTransport.enabled = !steam;
            IsSteam = steam;
            transport = steam ? (Transport)steamTransport : localTransport;
            Transport.active = transport;
        }

        public void StartMatch()
        {
            if (!CanStartMatch) return;
            AdvanceHostClock();
            SendPackets(authority.StartGame(new SystemRandomSource()));
        }

        // Host lobby settings (UX §6.2); invalid values are ignored.
        public void SetLobbySettings(int turnSeconds, int maxPlayers)
        {
            if (!IsHosting || authority == null || authority.IsStarted) return;
            try
            {
                int previousTimer = authority.TurnSeconds, previousMax = authority.MaxPlayers;
                var packets = authority.SetSettings(turnSeconds, maxPlayers);
                if (IsSteam && !steamLobby.SetMemberLimit(authority.MaxPlayers))
                {
                    // Keep Steam capacity and the authoritative lobby settings in sync.
                    authority.SetSettings(previousTimer, previousMax);
                    LobbyNotice = "Impossible de mettre à jour les places du salon Steam. Réessaie.";
                    return;
                }
                LobbyNotice = null;
                SendPackets(packets);
            }
            catch (ArgumentOutOfRangeException) { }
        }

        // "Revanche": everyone back to the same lobby.
        public void Rematch()
        {
            if (CanRematch) SendPackets(authority.ReturnToLobby());
        }

        public void StopSession()
        {
            if (stopping) return;
            stopping = true;
            try
            {
                steamLobby?.LeaveLobby();
                if (NetworkServer.active) StopHost();
                else if (NetworkClient.active) StopClient();
                // Also close partially started Fizzy transports after a startup failure.
                if (steamTransport != null)
                {
                    steamTransport.Shutdown();
                    steamTransport.CancelInvoke();
                    steamTransport.enabled = false;
                }
                Session = null;
                IsSteam = false;
                LobbyNotice = null;
                Status = "Choisis un mode de jeu.";
            }
            finally { stopping = false; }
        }

        public override void OnStartServer()
        {
            authority = new HostAuthority();
            authority.SetHostConnection(NetworkConnection.LocalConnectionId); // the host's own player
            closing.Clear();
            lastTick = Time.realtimeSinceStartupAsDouble;
            NetworkServer.RegisterHandler<DubitoPacket>(OnPacket);
        }

        public override void OnStartClient()
        {
            Session = new NetworkGameSession(bytes => NetworkClient.Send(new DubitoPacket { Payload = bytes }));
            Session.VersionMismatch += _ => Fail("Version différente : l'hôte et le client doivent utiliser la même version.");
            Session.ProtocolError += _ => Fail("Message réseau invalide reçu de l'hôte.");
            Session.JoinRefused += reason => Fail(reason == JoinRefusal.LobbyFull
                ? "Salon plein : toutes les places sont prises." : "La partie a déjà commencé.");
            NetworkClient.RegisterHandler<DubitoPacket>(packet => Session?.Receive(packet.Payload));
            SessionCreated?.Invoke(Session);
        }

        public override void OnClientConnect()
        {
            base.OnClientConnect(); // ready, but never creates a player prefab
            connected = true;
            Status = "Connecté, en attente de l'hôte…";
            string name = IsSteam ? steamLobby.LocalPlayerName : "";
            NetworkClient.Send(new DubitoPacket { Payload = ProtocolCodec.Encode(new HelloMessage(ProtocolInfo.Version, name)) });
        }

        void OnPacket(NetworkConnectionToClient connection, DubitoPacket packet)
        {
            if (closing.ContainsKey(connection.connectionId)) return;
            // Charge elapsed host time BEFORE processing the arriving intention (RULES §8).
            AdvanceHostClock();
            SendPackets(authority.Receive(connection.connectionId, packet.Payload));
        }

        public override void OnServerDisconnect(NetworkConnectionToClient connection)
        {
            AdvanceHostClock();
            if (authority != null) SendPackets(authority.Disconnect(connection.connectionId));
            closing.Remove(connection.connectionId);
            base.OnServerDisconnect(connection);
        }

        public override void Update()
        {
            base.Update();
            if (!NetworkServer.active || authority == null) return;
            AdvanceHostClock();
            foreach (int id in new List<int>(closing.Keys))
                if (Time.realtimeSinceStartupAsDouble >= closing[id])
                {
                    closing.Remove(id);
                    if (NetworkServer.connections.TryGetValue(id, out var connection)) connection.Disconnect();
                }
        }

        void AdvanceHostClock()
        {
            if (!NetworkServer.active || authority == null) return;
            double now = Time.realtimeSinceStartupAsDouble;
            SendPackets(authority.Tick(Math.Max(0, now - lastTick)));
            lastTick = now;
        }

        void SendPackets(IReadOnlyList<OutgoingPacket> packets)
        {
            foreach (var packet in packets)
            {
                if (!NetworkServer.connections.TryGetValue(packet.ConnectionId, out var connection)) continue;
                if (packet.Payload != null) connection.Send(new DubitoPacket { Payload = packet.Payload });
                // Allow the reliable VersionMismatch response to flush before closing KCP.
                if (packet.DisconnectAfterSend) closing[packet.ConnectionId] = Time.realtimeSinceStartupAsDouble + .25;
            }
        }

        public override void OnClientDisconnect()
        {
            Session = null;
            if (stopping) return;
            // C-DEC-06: the host left, the game is over for everyone (RULES §7).
            Status = error ?? (connected ? "L'hôte a quitté la partie." : ConnectionFailure);
            steamLobby?.LeaveLobby();
            IsSteam = false;
            Disconnected?.Invoke(Status);
        }

        public override void OnClientError(TransportError transportError, string reason) =>
            Fail(connected ? "La connexion à l'hôte a été perdue." : ConnectionFailure);
        public override void OnServerError(NetworkConnectionToClient connection, TransportError transportError, string reason)
        {
            if (connection == null) Fail(IsSteam ? SteamLobbyRules.ConnectionFailed : "Impossible d'héberger sur le port 7777.");
        }
        public override void OnStopServer() { authority = null; closing.Clear(); }
        string ConnectionFailure => IsSteam ? SteamLobbyRules.ConnectionFailed : "Connexion impossible à 127.0.0.1:7777.";
        internal void SetStatus(string message) => Status = message;
        internal void Fail(string message)
        {
            error = message;
            StopSession();
            Status = message;
            Disconnected?.Invoke(message);
        }

        public override void OnApplicationQuit()
        {
            StopSession();
            base.OnApplicationQuit();
        }
    }
}
