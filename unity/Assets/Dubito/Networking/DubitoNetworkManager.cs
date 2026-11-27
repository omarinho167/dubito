using System;
using System.Collections.Generic;
using Dubito.Core;
using Dubito.Protocol;
using Mirror;
using UnityEngine;
using kcp2k;

namespace Dubito.Networking
{
    [RequireComponent(typeof(KcpTransport))]
    public sealed class DubitoNetworkManager : NetworkManager
    {
        [SerializeField] string playerName = "";
        HostAuthority authority;
        readonly Dictionary<int, double> closing = new Dictionary<int, double>();
        double lastTick;
        string error;
        bool connected, stopping;
        public NetworkGameSession Session { get; private set; }
        public bool IsHosting => NetworkServer.active;
        public bool IsConnectingOrConnected => NetworkClient.active;
        public int ConnectedPlayers => authority?.PlayerCount ?? 0;
        public bool CanStartMatch => IsHosting && authority != null && authority.CanStart;
        public bool CanRematch => IsHosting && authority != null && authority.IsFinished;
        public int TurnSeconds => authority?.TurnSeconds ?? 45;
        public int MaxPlayers => authority?.MaxPlayers ?? 6;
        public string Status { get; private set; } = "Choisis un mode de jeu.";
        public event Action<NetworkGameSession> SessionCreated;
        public event Action<string> Disconnected;

        public override void Awake()
        {
            autoCreatePlayer = false;
            dontDestroyOnLoad = false;
            maxConnections = 6;
            networkAddress = "127.0.0.1";
            transport = GetComponent<KcpTransport>();
            ((KcpTransport)transport).Port = 7777;
            offlineScene = onlineScene = "";
            base.Awake();
        }

        public void HostLocal()
        {
            error = null; connected = false;
            Status = "Démarrage de l'hôte…";
            try { StartHost(); }
            catch (Exception) { Fail("Impossible d'héberger sur le port 7777."); }
        }

        public void JoinLocal()
        {
            error = null; connected = false;
            Status = "Connexion à 127.0.0.1…";
            networkAddress = "127.0.0.1";
            try { StartClient(); }
            catch (Exception) { Fail("Connexion impossible à 127.0.0.1:7777."); }
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
            try { SendPackets(authority.SetSettings(turnSeconds, maxPlayers)); }
            catch (ArgumentOutOfRangeException) { }
        }

        // "Revanche": everyone back to the same lobby.
        public void Rematch()
        {
            if (CanRematch) SendPackets(authority.ReturnToLobby());
        }

        public void StopSession()
        {
            stopping = true;
            if (NetworkServer.active) StopHost();
            else if (NetworkClient.active) StopClient();
            Session = null;
            stopping = false;
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
            NetworkClient.Send(new DubitoPacket { Payload = ProtocolCodec.Encode(new HelloMessage(ProtocolInfo.Version, playerName)) });
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
            Status = error ?? (connected ? "L'hôte a quitté la partie." : "Connexion impossible à 127.0.0.1:7777.");
            Disconnected?.Invoke(Status);
        }

        public override void OnClientError(TransportError transportError, string reason) =>
            Fail(connected ? "La connexion à l'hôte a été perdue." : "Connexion impossible à 127.0.0.1:7777.");
        public override void OnServerError(NetworkConnectionToClient connection, TransportError transportError, string reason)
        {
            if (connection == null) Fail("Impossible d'héberger sur le port 7777.");
        }
        public override void OnStopServer() { authority = null; closing.Clear(); }
        void Fail(string message)
        {
            error = message;
            StopSession();
            Status = message;
            Disconnected?.Invoke(message);
        }
    }
}
