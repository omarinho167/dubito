using System;
using System.Collections.Generic;
using System.Linq;
using Dubito.Core;
using Dubito.Protocol;

namespace Dubito.Networking
{
    public readonly struct OutgoingPacket
    {
        public int ConnectionId { get; }
        public byte[] Payload { get; }
        public bool DisconnectAfterSend { get; }
        public OutgoingPacket(int connectionId, byte[] payload, bool disconnectAfterSend = false)
        { ConnectionId = connectionId; Payload = payload; DisconnectAfterSend = disconnectAfterSend; }
    }

    // Sequential authority. Transport identity is the only source of player identity.
    public sealed class HostAuthority
    {
        // UX §5 animation lasts at most ~3.6 s. Only the host continues the game,
        // after 4 s, independently of client animation callbacks or frame rates.
        public const double RevelationDelaySeconds = 4;
        readonly List<(int ConnectionId, string Name)> players = new List<(int, string)>();
        readonly Dictionary<int, int> seats = new Dictionary<int, int>();
        readonly HashSet<int> ready = new HashSet<int>(); // connection ids marked "Prêt" in the lobby
        GameEngine engine;
        double revelationElapsed;
        int hostConnectionId = -1;
        public int PlayerCount => players.Count;
        public bool IsStarted => engine != null;
        // Host settings (UX §6.2), editable until the game starts.
        public int TurnSeconds { get; private set; } = 45;
        public int MaxPlayers { get; private set; } = 6;
        public bool IsFinished => engine != null && engine.State.Phase == GamePhase.Finished;
        // UX §6.2: at least 2 players and everyone ready; the host's own "Lancer" counts as ready.
        public bool CanStart => !IsStarted && PlayerCount >= 2 &&
            players.All(p => p.ConnectionId == hostConnectionId || ready.Contains(p.ConnectionId));

        // The host's own local connection: it never needs "Prêt" and is shown as the host.
        public void SetHostConnection(int connectionId) => hostConnectionId = connectionId;

        public IReadOnlyList<OutgoingPacket> SetSettings(int turnSeconds, int maxPlayers)
        {
            if (IsStarted) throw new InvalidOperationException("Settings are locked once the game started.");
            if (turnSeconds != 30 && turnSeconds != 45 && turnSeconds != 60 && turnSeconds != 90)
                throw new ArgumentOutOfRangeException(nameof(turnSeconds));
            // UX §6.2: 2 to 6 seats, never below the players already present.
            if (maxPlayers < Math.Max(2, PlayerCount) || maxPlayers > 6) throw new ArgumentOutOfRangeException(nameof(maxPlayers));
            TurnSeconds = turnSeconds;
            MaxPlayers = maxPlayers;
            return LobbyPackets();
        }

        public IReadOnlyList<OutgoingPacket> StartGame(IRandomSource random) =>
            StartGame(new GameConfig(PlayerCount, TurnSeconds), random);

        // "Revanche" (UX §5): everyone goes back to the same lobby, readiness reset.
        public IReadOnlyList<OutgoingPacket> ReturnToLobby()
        {
            if (!IsStarted) return Array.Empty<OutgoingPacket>();
            engine = null;
            revelationElapsed = 0;
            ready.Clear();
            // Players who left during the game are gone: make IDs dense again for GameSetup.
            for (int i = 0; i < players.Count; i++) seats[players[i].ConnectionId] = i;
            return LobbyPackets();
        }

        public IReadOnlyList<OutgoingPacket> Join(int connectionId, HelloMessage hello)
        {
            if (hello == null) throw new ArgumentNullException(nameof(hello));
            if (hello.ProtocolVersion != ProtocolInfo.Version)
                return new[] { new OutgoingPacket(connectionId,
                    ProtocolCodec.Encode(new VersionMismatchMessage(ProtocolInfo.Version)), true) };
            if (seats.ContainsKey(connectionId)) return Close(connectionId);
            if (IsStarted) return Refuse(connectionId, JoinRefusal.GameStarted);
            if (players.Count >= MaxPlayers) return Refuse(connectionId, JoinRefusal.LobbyFull);
            // IDs are provisional until launch. Lobby departures compact arrival order
            // to the dense IDs required by GameSetup; no client has a Welcome yet.
            seats.Add(connectionId, players.Count);
            players.Add((connectionId, hello.PlayerName));
            return LobbyPackets();
        }

        public IReadOnlyList<OutgoingPacket> StartGame(GameConfig config, IRandomSource random)
        {
            if (IsStarted || PlayerCount < 2) throw new InvalidOperationException("At least two lobby players required.");
            if (config == null || config.PlayerCount != PlayerCount) throw new ArgumentException("Lobby/config count mismatch.", nameof(config));
            engine = new GameEngine(GameSetup.Create(config, random));
            var names = new Dictionary<int, string>();
            foreach (var player in players)
            {
                int id = seats[player.ConnectionId];
                names.Add(id, string.IsNullOrWhiteSpace(player.Name) ? $"Joueur {id + 1}" : player.Name);
            }
            var packets = new List<OutgoingPacket>();
            foreach (var player in players)
            {
                int id = seats[player.ConnectionId];
                packets.Add(Packet(player.ConnectionId, new WelcomeMessage(ProtocolInfo.Version, id, names, engine.State.TurnOrder)));
                packets.Add(ViewPacket(player.ConnectionId, id));
            }
            return packets;
        }

        public IReadOnlyList<OutgoingPacket> Receive(int connectionId, byte[] bytes)
        {
            if (bytes == null || bytes.Length > ProtocolInfo.MaxMessageBytes) return Close(connectionId);
            var decoded = ProtocolCodec.Decode(bytes);
            if (!decoded.Success) return Close(connectionId);
            if (!seats.TryGetValue(connectionId, out int id))
                return decoded.Message is HelloMessage hello ? Join(connectionId, hello) : Close(connectionId);
            if (engine == null)
            {
                if (!(decoded.Message is ReadyMessage readiness)) return Close(connectionId);
                if (readiness.IsReady) ready.Add(connectionId); else ready.Remove(connectionId);
                return LobbyPackets();
            }
            if (decoded.Message is ReadyMessage) return Array.Empty<OutgoingPacket>(); // late click, harmless
            if (!(decoded.Message is CommandMessage command)) return Close(connectionId);
            long revision = engine.State.Revision;
            var result = engine.Execute(id, command.ToGameCommand());
            if (result.Status == CommandStatus.Rejected)
                return new[] { Packet(connectionId, new CommandRejectedMessage(command.Id, result.Error)), ViewPacket(connectionId, id) };
            return Changed(revision);
        }

        public IReadOnlyList<OutgoingPacket> Tick(double seconds)
        {
            if (double.IsNaN(seconds) || double.IsInfinity(seconds) || seconds < 0)
                throw new ArgumentOutOfRangeException(nameof(seconds));
            if (engine == null) return Array.Empty<OutgoingPacket>();
            long revision = engine.State.Revision;
            if (engine.State.Phase == GamePhase.Revelation)
            {
                revelationElapsed += seconds;
                if (revelationElapsed >= RevelationDelaySeconds)
                    engine.ResolveRevelation(revision);
                // Time spent presenting a revelation is never banked into the next turn.
                return Changed(revision);
            }
            if (engine.State.Phase != GamePhase.Playing) return Array.Empty<OutgoingPacket>();
            // Only time passes here: a new revision is always a timeout of the active player (RULES §5).
            int active = engine.State.ActivePlayerId;
            engine.AdvanceTime(seconds);
            if (engine.State.Revision == revision) return Array.Empty<OutgoingPacket>();
            var kind = engine.State.TurnOrder.Contains(active) ? TableEventKind.TurnSkipped : TableEventKind.Excluded;
            return Announce(new TableEventMessage(kind, active), Changed(revision));
        }

        public IReadOnlyList<OutgoingPacket> Disconnect(int connectionId)
        {
            if (!seats.TryGetValue(connectionId, out int id)) return Array.Empty<OutgoingPacket>();
            seats.Remove(connectionId);
            ready.Remove(connectionId);
            players.RemoveAll(p => p.ConnectionId == connectionId);
            if (engine == null)
            {
                for (int i = 0; i < players.Count; i++) seats[players[i].ConnectionId] = i;
                return LobbyPackets();
            }
            long revision = engine.State.Revision;
            engine.RemovePlayer(id);
            // RULES §7: everyone still at the table is told who left, then gets the new views.
            return Announce(new TableEventMessage(TableEventKind.Left, id), Changed(revision));
        }

        // The event goes to every connected player first, so it is shown with the view that follows.
        IReadOnlyList<OutgoingPacket> Announce(TableEventMessage tableEvent, IReadOnlyList<OutgoingPacket> views)
        {
            var packets = new List<OutgoingPacket>();
            byte[] bytes = ProtocolCodec.Encode(tableEvent);
            foreach (var player in players) packets.Add(new OutgoingPacket(player.ConnectionId, bytes));
            packets.AddRange(views);
            return packets;
        }

        IReadOnlyList<OutgoingPacket> Changed(long previousRevision)
        {
            if (engine.State.Revision == previousRevision) return Array.Empty<OutgoingPacket>();
            // A departure during an existing revelation does not restart its deadline.
            if (engine.State.Phase != GamePhase.Revelation) revelationElapsed = 0;
            var packets = new List<OutgoingPacket>();
            foreach (var player in players) packets.Add(ViewPacket(player.ConnectionId, seats[player.ConnectionId]));
            return packets;
        }

        OutgoingPacket ViewPacket(int connectionId, int playerId) =>
            Packet(connectionId, new ViewMessage(PlayerView.Create(engine.State, playerId)));
        static OutgoingPacket Packet(int connectionId, IProtocolMessage message) =>
            new OutgoingPacket(connectionId, ProtocolCodec.Encode(message));
        static IReadOnlyList<OutgoingPacket> Close(int connectionId) =>
            new[] { new OutgoingPacket(connectionId, null, true) };
        static IReadOnlyList<OutgoingPacket> Refuse(int connectionId, JoinRefusal reason) =>
            new[] { new OutgoingPacket(connectionId, ProtocolCodec.Encode(new JoinRefusedMessage(reason)), true) };
        IReadOnlyList<OutgoingPacket> LobbyPackets()
        {
            var packets = new List<OutgoingPacket>();
            if (players.Count == 0) return packets;
            var entries = players.Select(p => new LobbyPlayer(seats[p.ConnectionId], DisplayName(p.Name, seats[p.ConnectionId]),
                p.ConnectionId == hostConnectionId || ready.Contains(p.ConnectionId))).ToList();
            int host = seats.TryGetValue(hostConnectionId, out int hostId) ? hostId : entries[0].PlayerId;
            // One message per recipient: it says which entry is theirs.
            foreach (var player in players)
                packets.Add(Packet(player.ConnectionId, new LobbyStateMessage(entries, TurnSeconds, MaxPlayers,
                    seats[player.ConnectionId], host)));
            return packets;
        }
        static string DisplayName(string name, int playerId) =>
            string.IsNullOrWhiteSpace(name) ? $"Joueur {playerId + 1}" : name;
    }
}
