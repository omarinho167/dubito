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
        GameEngine engine;
        double revelationElapsed;
        public int PlayerCount => players.Count;
        public bool IsStarted => engine != null;

        public IReadOnlyList<OutgoingPacket> Join(int connectionId, HelloMessage hello)
        {
            if (hello == null) throw new ArgumentNullException(nameof(hello));
            if (hello.ProtocolVersion != ProtocolInfo.Version)
                return new[] { new OutgoingPacket(connectionId,
                    ProtocolCodec.Encode(new VersionMismatchMessage(ProtocolInfo.Version)), true) };
            if (IsStarted || players.Count == 6 || seats.ContainsKey(connectionId)) return Close(connectionId);
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
            if (engine == null || !(decoded.Message is CommandMessage command)) return Close(connectionId);
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
            }
            else engine.AdvanceTime(seconds);
            return Changed(revision);
        }

        public IReadOnlyList<OutgoingPacket> Disconnect(int connectionId)
        {
            if (!seats.TryGetValue(connectionId, out int id)) return Array.Empty<OutgoingPacket>();
            seats.Remove(connectionId);
            players.RemoveAll(p => p.ConnectionId == connectionId);
            if (engine == null)
            {
                for (int i = 0; i < players.Count; i++) seats[players[i].ConnectionId] = i;
                return LobbyPackets();
            }
            long revision = engine.State.Revision;
            engine.RemovePlayer(id);
            return Changed(revision);
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
        IReadOnlyList<OutgoingPacket> LobbyPackets()
        {
            var packets = new List<OutgoingPacket>();
            if (players.Count == 0) return packets;
            var lobby = new LobbyStateMessage(players.Select(p => new LobbyPlayer(seats[p.ConnectionId], p.Name, false)), 45, 6);
            foreach (var player in players) packets.Add(Packet(player.ConnectionId, lobby));
            return packets;
        }
    }
}
