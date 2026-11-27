using System;
using System.Collections.Generic;
using System.Linq;
using Dubito.Core;
using Dubito.Networking;
using Dubito.Protocol;
using NUnit.Framework;

namespace Dubito.Tests.Networking
{
    // Step 3.4 (UX §6.2): lobby on the real byte path between HostAuthority and NetworkGameSession.
    public class LobbyTests
    {
        const int HostConnection = 0;

        sealed class Lobby
        {
            public readonly HostAuthority Host = new HostAuthority();
            public readonly Dictionary<int, NetworkGameSession> Clients = new Dictionary<int, NetworkGameSession>();
            public readonly List<int> Closed = new List<int>();

            public Lobby(int players)
            {
                Host.SetHostConnection(HostConnection);
                for (int i = 0; i < players; i++) Join(i);
            }

            public int Join(int connection)
            {
                var session = new NetworkGameSession(bytes => Deliver(Host.Receive(connection, bytes)));
                Clients[connection] = session;
                Deliver(Host.Receive(connection, ProtocolCodec.Encode(new HelloMessage(ProtocolInfo.Version, ""))));
                return connection;
            }

            public void Deliver(IReadOnlyList<OutgoingPacket> packets)
            {
                foreach (var packet in packets)
                {
                    if (packet.Payload != null && Clients.TryGetValue(packet.ConnectionId, out var client)) client.Receive(packet.Payload);
                    if (packet.DisconnectAfterSend) Closed.Add(packet.ConnectionId);
                }
            }
        }

        [Test]
        public void EachClientSeesTheLobbyAndKnowsItsOwnEntryAndTheHost()
        {
            var lobby = new Lobby(3);
            foreach (var pair in lobby.Clients)
            {
                var state = pair.Value.Lobby;
                Assert.That(state.Players.Select(p => p.PlayerName), Is.EqualTo(new[] { "Joueur 1", "Joueur 2", "Joueur 3" }));
                Assert.That(state.YourPlayerId, Is.EqualTo(pair.Key));
                Assert.That(state.HostPlayerId, Is.EqualTo(0));
                Assert.That(state.TurnDurationSeconds, Is.EqualTo(45));
                Assert.That(state.MaxPlayers, Is.EqualTo(6));
            }
        }

        [Test]
        public void LaunchNeedsTwoPlayersAndEveryGuestReady()
        {
            var lobby = new Lobby(1);
            Assert.That(lobby.Host.CanStart, Is.False, "alone");
            lobby.Join(1); lobby.Join(2);
            Assert.That(lobby.Host.CanStart, Is.False, "guests not ready");
            lobby.Clients[1].SetReady(true);
            Assert.That(lobby.Host.CanStart, Is.False, "one guest still not ready");
            lobby.Clients[2].SetReady(true);
            Assert.That(lobby.Host.CanStart, Is.True);
            Assert.That(lobby.Clients[1].Lobby.Players.Single(p => p.PlayerId == 2).IsReady, Is.True);
            lobby.Clients[2].SetReady(false);
            Assert.That(lobby.Host.CanStart, Is.False, "ready can be withdrawn");
        }

        [Test]
        public void HostSettingsAreSharedAndBounded()
        {
            var lobby = new Lobby(3);
            lobby.Deliver(lobby.Host.SetSettings(90, 4));
            Assert.That(lobby.Clients[2].Lobby.TurnDurationSeconds, Is.EqualTo(90));
            Assert.That(lobby.Clients[2].Lobby.MaxPlayers, Is.EqualTo(4));
            Assert.Throws<ArgumentOutOfRangeException>(() => lobby.Host.SetSettings(50, 4), "not a RULES §5 duration");
            Assert.Throws<ArgumentOutOfRangeException>(() => lobby.Host.SetSettings(45, 2), "below the players present");
            Assert.Throws<ArgumentOutOfRangeException>(() => lobby.Host.SetSettings(45, 7));
        }

        [Test]
        public void GameUsesTheChosenTimer()
        {
            var lobby = new Lobby(2);
            lobby.Deliver(lobby.Host.SetSettings(30, 6));
            lobby.Deliver(lobby.Host.StartGame(new SystemRandomSource(3)));
            Assert.That(lobby.Clients[0].CurrentView.RemainingTurnSeconds, Is.EqualTo(30));
            Assert.That(lobby.Clients[1].Lobby, Is.Null, "the lobby is over once the game starts");
        }

        [Test]
        public void FullLobbyAndStartedGameAreRefusedWithAReason()
        {
            var lobby = new Lobby(2);
            lobby.Deliver(lobby.Host.SetSettings(45, 2));
            var refusals = new List<JoinRefusal>();
            var third = new NetworkGameSession(_ => { });
            third.JoinRefused += refusals.Add;
            lobby.Clients[7] = third;
            lobby.Deliver(lobby.Host.Receive(7, ProtocolCodec.Encode(new HelloMessage(ProtocolInfo.Version, ""))));
            lobby.Deliver(lobby.Host.StartGame(new SystemRandomSource(3)));
            var late = new NetworkGameSession(_ => { });
            late.JoinRefused += refusals.Add;
            lobby.Clients[8] = late;
            lobby.Deliver(lobby.Host.Receive(8, ProtocolCodec.Encode(new HelloMessage(ProtocolInfo.Version, ""))));
            Assert.That(refusals, Is.EqualTo(new[] { JoinRefusal.LobbyFull, JoinRefusal.GameStarted }));
            Assert.That(lobby.Closed, Is.EquivalentTo(new[] { 7, 8 }));
        }

        [Test]
        public void RematchBringsEveryoneBackToTheSameLobbyNotReady()
        {
            var lobby = new Lobby(3);
            lobby.Clients[1].SetReady(true); lobby.Clients[2].SetReady(true);
            lobby.Deliver(lobby.Host.StartGame(new SystemRandomSource(5)));
            // A guest leaves during the game: the next lobby has dense IDs again.
            lobby.Deliver(lobby.Host.Disconnect(1));
            lobby.Clients.Remove(1);
            lobby.Deliver(lobby.Host.ReturnToLobby());
            foreach (var client in lobby.Clients.Values)
            {
                Assert.That(client.IsReady, Is.False, "no game view any more");
                Assert.That(client.Lobby.Players.Select(p => p.PlayerId), Is.EqualTo(new[] { 0, 1 }));
                Assert.That(client.Lobby.Players.Single(p => p.PlayerId != 0).IsReady, Is.False);
            }
            Assert.That(lobby.Host.CanStart, Is.False);
            lobby.Clients[2].SetReady(true);
            Assert.That(lobby.Host.CanStart, Is.True);
            lobby.Deliver(lobby.Host.StartGame(new SystemRandomSource(6)));
            Assert.That(lobby.Clients[2].CurrentView.CardCounts.Count, Is.EqualTo(2));
        }
    }
}
