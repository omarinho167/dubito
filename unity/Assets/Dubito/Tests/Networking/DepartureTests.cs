using System.Collections.Generic;
using System.Linq;
using Dubito.Core;
using Dubito.Networking;
using Dubito.Protocol;
using NUnit.Framework;

namespace Dubito.Tests.Networking
{
    // Step 3.5 (RULES §5 and §7): departures, skipped turns and exclusions over the real byte path.
    // The rules themselves are covered by the Core C-DEC/C-CHR tests; here we check what every client receives.
    public class DepartureTests
    {
        sealed class Table
        {
            public readonly HostAuthority Host = new HostAuthority();
            public readonly Dictionary<int, NetworkGameSession> Clients = new Dictionary<int, NetworkGameSession>();
            public readonly Dictionary<int, List<TableEventMessage>> Events = new Dictionary<int, List<TableEventMessage>>();

            public Table(int players, int seed = 11)
            {
                Host.SetHostConnection(0);
                for (int c = 0; c < players; c++)
                {
                    int connection = c;
                    var session = new NetworkGameSession(bytes => Deliver(Host.Receive(connection, bytes)));
                    Clients[connection] = session;
                    Events[connection] = new List<TableEventMessage>();
                    session.TableEvent += e => Events[connection].Add(e);
                    Deliver(Host.Receive(connection, ProtocolCodec.Encode(new HelloMessage(ProtocolInfo.Version, ""))));
                }
                Deliver(Host.StartGame(new SystemRandomSource(seed)));
            }

            public void Deliver(IReadOnlyList<OutgoingPacket> packets)
            {
                foreach (var p in packets)
                    if (p.Payload != null && Clients.TryGetValue(p.ConnectionId, out var client)) client.Receive(p.Payload);
            }

            // Connection ids equal player ids here (joined in order 0, 1, 2…).
            public PlayerView View(int player) => Clients[player].CurrentView;
            public int Active => View(0).ActivePlayerId;

            public void Leave(int player)
            {
                Clients.Remove(player);
                Deliver(Host.Disconnect(player));
            }
        }

        [TestCase(TestName = "C-DEC-01 (réseau)")]
        public void ActivePlayerLeavesEveryoneIsToldAndTheTurnMovesOn()
        {
            var t = new Table(3);
            int active = t.Active;
            int pile = t.View(0).PileCount;
            t.Leave(active);
            foreach (var pair in t.Clients)
            {
                Assert.That(t.Events[pair.Key].Single().Kind, Is.EqualTo(TableEventKind.Left));
                Assert.That(t.Events[pair.Key].Single().PlayerId, Is.EqualTo(active));
                var view = pair.Value.CurrentView;
                Assert.That(view.TurnOrder, Has.No.Member(active));
                Assert.That(view.CardCounts[active], Is.Zero, "the hand is removed from the game");
                Assert.That(view.ActivePlayerId, Is.Not.EqualTo(active));
                Assert.That(view.PileCount, Is.EqualTo(pile));
            }
        }

        [TestCase(TestName = "C-DEC-03 (réseau)")]
        public void BystanderLeavesNothingElseChanges()
        {
            var t = new Table(3);
            int active = t.Active;
            int bystander = t.View(0).TurnOrder.First(p => p != active && p != t.View(0).TurnOrder[(t.View(0).TurnOrder.ToList().IndexOf(active) + 1) % 3]);
            var before = t.View(active);
            t.Leave(bystander);
            var after = t.View(active);
            Assert.That(after.ActivePlayerId, Is.EqualTo(active));
            Assert.That(after.Hand, Is.EqualTo(before.Hand));
            Assert.That(after.TurnOrder, Has.No.Member(bystander));
        }

        [Test]
        public void LastPlayerStandingWinsAndEveryoneLeftSeesIt()
        {
            var t = new Table(3);
            var order = t.View(0).TurnOrder.ToArray();
            t.Leave(order[1]);
            t.Leave(order[2]);
            var view = t.View(order[0]);
            Assert.That(view.Phase, Is.EqualTo(GamePhase.Finished));
            Assert.That(view.WinnerId, Is.EqualTo(order[0]));
        }

        [Test]
        public void TimeoutIsAnnouncedThenSecondTimeoutExcludes()
        {
            var t = new Table(3);
            int first = t.Active;
            t.Deliver(t.Host.Tick(45));
            Assert.That(t.Events.Values.All(list => list.Single().Kind == TableEventKind.TurnSkipped && list.Single().PlayerId == first), Is.True);
            // The other two time out too, then the first one again: second skip in a row → excluded.
            t.Deliver(t.Host.Tick(45));
            t.Deliver(t.Host.Tick(45));
            t.Deliver(t.Host.Tick(45));
            var last = t.Events[first].Last();
            Assert.That(last.Kind, Is.EqualTo(TableEventKind.Excluded));
            Assert.That(last.PlayerId, Is.EqualTo(first));
            Assert.That(t.View(first).TurnOrder, Has.No.Member(first));
        }

        [Test]
        public void DepartureDuringARevelationStillEndsItOnTime()
        {
            var t = new Table(3);
            int accused = t.Active;
            var hand = t.View(accused).Hand;
            var card = hand[0];
            t.Clients[accused].Play(new[] { card }, card.Rank == CardRank.King ? CardRank.Queen : CardRank.King); // a lie
            int doubter = t.View(0).ActivePlayerId;
            t.Clients[doubter].Doubt();
            Assert.That(t.View(doubter).Phase, Is.EqualTo(GamePhase.Revelation));
            int bystander = t.View(0).TurnOrder.First(p => p != accused && p != doubter);
            t.Leave(bystander);
            Assert.That(t.View(doubter).Phase, Is.EqualTo(GamePhase.Revelation), "the verdict is applied first");
            t.Deliver(t.Host.Tick(HostAuthority.RevelationDelaySeconds));
            Assert.That(t.View(doubter).Phase, Is.EqualTo(GamePhase.Playing));
            Assert.That(t.View(accused).CardCounts[accused], Is.EqualTo(hand.Count), "the liar picked the pile back up");
        }

        [TestCase(TestName = "C-DEC-07 (réseau)")]
        public void JoiningAGameInProgressIsRefusedWithAReason()
        {
            var t = new Table(2);
            var refusals = new List<JoinRefusal>();
            var late = new NetworkGameSession(_ => { });
            late.JoinRefused += refusals.Add;
            t.Clients[9] = late;
            var packets = t.Host.Receive(9, ProtocolCodec.Encode(new HelloMessage(ProtocolInfo.Version, "")));
            t.Deliver(packets);
            Assert.That(refusals, Is.EqualTo(new[] { JoinRefusal.GameStarted }));
            Assert.That(packets.Single().DisconnectAfterSend, Is.True);
        }
    }
}
