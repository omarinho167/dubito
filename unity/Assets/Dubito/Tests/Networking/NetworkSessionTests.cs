using System;
using System.Collections.Generic;
using System.Linq;
using Dubito.Core;
using Dubito.Networking;
using Dubito.Protocol;
using Dubito.Tests.Core;
using Dubito.Tests.Protocol;
using NUnit.Framework;

namespace Dubito.Tests.Networking
{
    public class NetworkSessionTests
    {
        // Independent Core oracle + real client/host byte path. Every host packet,
        // including the host player's packets, is decoded and inspected before delivery.
        sealed class Wire
        {
            public readonly HostAuthority Host = new HostAuthority();
            public readonly Dictionary<int, NetworkGameSession> Clients = new Dictionary<int, NetworkGameSession>();
            public readonly List<(int Connection, IProtocolMessage Message)> Trace = new List<(int, IProtocolMessage)>();
            public readonly GameEngine Oracle;
            public double Now;
            double revelationTime;
            public int Views;
            public Wire(int count = 3, int seed = 932)
            {
                for (int id = 0; id < count; id++)
                {
                    int connection = Connection(id);
                    Clients.Add(connection, new NetworkGameSession(b => SendBytes(connection, b), () => Now));
                    Deliver(Host.Receive(connection, ProtocolCodec.Encode(new HelloMessage(ProtocolInfo.Version, "Nom " + id))));
                }
                Oracle = new GameEngine(GameSetup.Create(new GameConfig(count), new SystemRandomSource(seed)));
                Deliver(Host.StartGame(new GameConfig(count), new SystemRandomSource(seed)));
            }
            public static int Connection(int player) => 101 + player * 17;
            public NetworkGameSession Client(int player) => Clients[Connection(player)];
            public NetworkGameSession Active => Client(Oracle.State.ActivePlayerId);
            public void SendBytes(int connection, byte[] bytes)
            {
                var command = (CommandMessage)ProtocolCodec.Decode(bytes).Message;
                int id = Clients[connection].ControlledPlayerId;
                Oracle.Execute(id, command.ToGameCommand());
                Deliver(Host.Receive(connection, bytes));
            }
            public void Play(int count = 1, CardRank? rank = null)
            {
                var view = Active.CurrentView;
                Active.Play(view.Hand.Take(count).ToArray(), view.RoundRank ?? rank ?? CardRank.King);
            }
            public void Tick(double seconds)
            {
                Now += seconds;
                if (Oracle.State.Phase == GamePhase.Revelation)
                {
                    revelationTime += seconds;
                    if (revelationTime >= HostAuthority.RevelationDelaySeconds)
                        Oracle.ResolveRevelation(Oracle.State.Revision);
                }
                else Oracle.AdvanceTime(seconds);
                if (Oracle.State.Phase != GamePhase.Revelation) revelationTime = 0;
                Deliver(Host.Tick(seconds));
            }
            public void Disconnect(int id)
            {
                Clients.Remove(Connection(id));
                Oracle.RemovePlayer(id);
                Deliver(Host.Disconnect(Connection(id)));
            }
            public void Deliver(IReadOnlyList<OutgoingPacket> packets)
            {
                foreach (var packet in packets)
                {
                    Assert.That(packet.DisconnectAfterSend, Is.False);
                    Assert.That(packet.Payload.Length, Is.LessThanOrEqualTo(ProtocolInfo.MaxMessageBytes));
                    var decoded = ProtocolCodec.Decode(packet.Payload);
                    Assert.That(decoded.Success, Is.True, decoded.Error);
                    var message = decoded.Message;
                    Trace.Add((packet.ConnectionId, message));
                    int id = (packet.ConnectionId - 101) / 17;
                    if (message is ViewMessage snapshot)
                    {
                        Views++;
                        Assert.That(snapshot.View.PlayerId, Is.EqualTo(id));
                        var expected = PlayerView.Create(Oracle.State, id);
                        SnapshotAssert.Equal(expected, snapshot.View);
                        var allowed = new HashSet<Card>(Oracle.State.Hands[id]);
                        if (Oracle.State.Phase == GamePhase.Revelation) allowed.UnionWith(Oracle.State.Revelation.Cards);
                        Assert.That(ProtocolTestData.ReachableCards(message), Is.EquivalentTo(allowed));
                    }
                    else Assert.That(ProtocolTestData.ReachableCards(message), Is.Empty);
                    Clients[packet.ConnectionId].Receive(packet.Payload);
                }
            }
        }

        [Test]
        public void ArrivalIdentityIsBoundToConnectionAndRandomizedSeatingIsFrozen()
        {
            var w = new Wire(6);
            foreach (var client in w.Clients.Values)
            {
                Assert.That(client.IsReady, Is.True);
                Assert.That(client.SeatOrder, Is.EqualTo(w.Oracle.State.TurnOrder));
                Assert.That(client.PlayerNames[client.ControlledPlayerId], Is.EqualTo("Nom " + client.ControlledPlayerId));
            }
            Assert.That(w.Clients.Select(p => p.Value.ControlledPlayerId), Is.EqualTo(Enumerable.Range(0, 6)));
            var seats = w.Client(0).SeatOrder.ToArray();
            w.Disconnect(w.Oracle.State.ActivePlayerId);
            foreach (var client in w.Clients.Values) Assert.That(client.SeatOrder, Is.EqualTo(seats));
        }

        [Test]
        public void ConnectionACannotActAsBOrPlayBsCards()
        {
            var w = new Wire();
            int b = w.Oracle.State.ActivePlayerId, a = (b + 1) % 3;
            var errors = new List<CommandError>();
            w.Client(a).CommandRejected += errors.Add;
            long revision = w.Client(a).CurrentView.Revision;
            w.Client(a).Play(w.Client(b).CurrentView.Hand.Take(1).ToArray(), CardRank.Ace);
            Assert.That(errors, Is.EqualTo(new[] { CommandError.NotYourTurn }));
            Assert.That(w.Oracle.State.Revision, Is.EqualTo(revision));
            errors.Clear();
            w.Client(b).CommandRejected += errors.Add;
            w.Client(b).Play(w.Client(a).CurrentView.Hand.Take(1).ToArray(), CardRank.Ace);
            Assert.That(errors, Is.EqualTo(new[] { CommandError.CardNotInHand }));
            Assert.That(w.Trace.Where(p => p.Message is CommandRejectedMessage).Select(p => p.Connection),
                Is.EqualTo(new[] { Wire.Connection(a), Wire.Connection(b) }));
        }

        [Test]
        public void DuplicateIgnoredAndStaleRevisionRejectedOnlyToSenderWithResync()
        {
            var w = new Wire();
            int id = w.Oracle.State.ActivePlayerId;
            var command = new CommandMessage(Guid.NewGuid(), w.Oracle.State.Revision, CommandKind.Play,
                w.Client(id).CurrentView.Hand.Take(1), CardRank.King);
            var bytes = ProtocolCodec.Encode(command);
            w.SendBytes(Wire.Connection(id), bytes);
            int before = w.Trace.Count;
            w.SendBytes(Wire.Connection(id), bytes);
            Assert.That(w.Trace.Count, Is.EqualTo(before));
            w.SendBytes(Wire.Connection(id), ProtocolCodec.Encode(new CommandMessage(Guid.NewGuid(), 0, CommandKind.Doubt)));
            var tail = w.Trace.Skip(before).ToArray();
            Assert.That(tail.Length, Is.EqualTo(2));
            Assert.That(tail.All(p => p.Connection == Wire.Connection(id)), Is.True);
            Assert.That(((CommandRejectedMessage)tail[0].Message).Error, Is.EqualTo(CommandError.StaleRevision));
            Assert.That(tail[1].Message, Is.TypeOf<ViewMessage>());
        }

        [Test]
        public void WrongVersionRespondsAndRequestsDisconnect()
        {
            var host = new HostAuthority();
            var packets = host.Receive(79, ProtocolCodec.Encode(new HelloMessage(ProtocolInfo.Version + 1, "Ancien")));
            Assert.That(packets.Count, Is.EqualTo(1));
            Assert.That(packets[0].ConnectionId, Is.EqualTo(79));
            Assert.That(packets[0].DisconnectAfterSend, Is.True);
            var client = new NetworkGameSession(_ => Assert.Fail());
            int version = 0;
            client.VersionMismatch += v => version = v;
            client.Receive(packets[0].Payload);
            Assert.That(version, Is.EqualTo(ProtocolInfo.Version));
            Assert.That(host.PlayerCount, Is.Zero);
            Assert.That(client.IsReady, Is.False);
        }

        [Test]
        public void MalformedOversizedUnexpectedAndUnauthenticatedMessagesAreClosed()
        {
            var host = new HostAuthority();
            var samples = new[] { null, new byte[ProtocolInfo.MaxMessageBytes + 1], new byte[] { 1, 2 },
                ProtocolCodec.Encode(new ReadyMessage(true)),
                ProtocolCodec.Encode(new CommandMessage(Guid.NewGuid(), 0, CommandKind.Doubt)) };
            foreach (var bytes in samples)
                Assert.That(host.Receive(55, bytes).Single().DisconnectAfterSend, Is.True);
            host.Join(55, new HelloMessage(ProtocolInfo.Version, "A"));
            // Step 3.4: "Prêt" is valid in the lobby; a game command before the launch is not.
            Assert.That(host.Receive(55, ProtocolCodec.Encode(new ReadyMessage(true))).All(p => !p.DisconnectAfterSend), Is.True);
            Assert.That(host.Receive(55, ProtocolCodec.Encode(new CommandMessage(Guid.NewGuid(), 0, CommandKind.Doubt)))
                .Single().DisconnectAfterSend, Is.True);
            Assert.Throws<InvalidOperationException>(() => host.StartGame(new GameConfig(2), new SystemRandomSource(1)));
        }

        [Test]
        public void LobbyDepartureCompactsProvisionalIdsAndRejectsLateJoin()
        {
            var host = new HostAuthority();
            host.Join(9, new HelloMessage(1, "A")); host.Join(3, new HelloMessage(1, "B"));
            host.Join(88, new HelloMessage(1, "C")); host.Disconnect(3);
            var packets = host.StartGame(new GameConfig(2), new SystemRandomSource(2));
            var welcomes = packets.Select(p => (p.ConnectionId, Message: ProtocolCodec.Decode(p.Payload).Message))
                .Where(p => p.Message is WelcomeMessage).ToArray();
            Assert.That(((WelcomeMessage)welcomes[0].Message).PlayerId, Is.Zero);
            Assert.That(welcomes[1].ConnectionId, Is.EqualTo(88));
            Assert.That(((WelcomeMessage)welcomes[1].Message).PlayerId, Is.EqualTo(1));
            Assert.That(host.Join(10, new HelloMessage(1, "D")).Single().DisconnectAfterSend, Is.True);
        }

        [Test]
        public void BlankNamesDefaultToFinalPlayerIdsAfterLobbyDeparture()
        {
            var host = new HostAuthority();
            host.Receive(9, ProtocolCodec.Encode(new HelloMessage(ProtocolInfo.Version, "")));
            host.Receive(3, ProtocolCodec.Encode(new HelloMessage(ProtocolInfo.Version, "Partant")));
            host.Receive(88, ProtocolCodec.Encode(new HelloMessage(ProtocolInfo.Version, " \t ")));
            host.Receive(74, ProtocolCodec.Encode(new HelloMessage(ProtocolInfo.Version, "Élise")));
            host.Disconnect(3);
            var packets = host.StartGame(new GameConfig(3), new SystemRandomSource(2));
            foreach (var packet in packets)
            {
                if (!(ProtocolCodec.Decode(packet.Payload).Message is WelcomeMessage welcome)) continue;
                Assert.That(welcome.PlayerNames[0], Is.EqualTo("Joueur 1"));
                Assert.That(welcome.PlayerNames[1], Is.EqualTo("Joueur 2"));
                Assert.That(welcome.PlayerNames[2], Is.EqualTo("Élise"));
            }
        }

        [Test]
        public void ClientCommandsUseFreshIdsAndCurrentRevisionAndNoSeat()
        {
            var w = new Wire();
            var sent = new List<byte[]>();
            var client = new NetworkGameSession(sent.Add);
            int id = w.Active.ControlledPlayerId;
            foreach (var message in w.Trace.Where(p => p.Connection == Wire.Connection(id) &&
                (p.Message is WelcomeMessage || p.Message is ViewMessage))) client.Receive(ProtocolCodec.Encode(message.Message));
            client.Play(client.CurrentView.Hand.Take(1).ToArray(), CardRank.Two); client.Doubt(); client.Discard();
            var commands = sent.Select(b => (CommandMessage)ProtocolCodec.Decode(b).Message).ToArray();
            Assert.That(commands.Select(c => c.Id).Distinct().Count(), Is.EqualTo(3));
            Assert.That(commands.All(c => c.Id != Guid.Empty && c.ExpectedRevision == client.CurrentView.Revision), Is.True);
            Assert.That(commands.Select(c => c.Kind), Is.EqualTo(new[] { CommandKind.Play, CommandKind.Doubt, CommandKind.Discard }));
        }

        [Test]
        public void RevelationIsHostTimedAndClientCountdownFreezesAndResumes()
        {
            var w = new Wire();
            var client = w.Active;
            Assert.That(client.RemainingTurnSeconds, Is.EqualTo(45));
            w.Tick(8);
            Assert.That(client.RemainingTurnSeconds, Is.EqualTo(37));
            w.Play();
            w.Active.Doubt();
            double remaining = client.RemainingTurnSeconds;
            long revision = client.CurrentView.Revision;
            w.Tick(3.9);
            Assert.That(client.CurrentView.Phase, Is.EqualTo(GamePhase.Revelation));
            Assert.That(client.RemainingTurnSeconds, Is.EqualTo(remaining));
            Assert.That(client.CurrentView.Revision, Is.EqualTo(revision));
            w.Tick(.11);
            Assert.That(client.CurrentView.Phase, Is.EqualTo(GamePhase.Playing));
            Assert.That(client.RemainingTurnSeconds, Is.EqualTo(45));
            w.Tick(2);
            Assert.That(client.RemainingTurnSeconds, Is.EqualTo(43));
        }

        [Test]
        public void RevelationDepartureDoesNotRestartDeadlineAndLargeTickDoesNotChargeNextTurn()
        {
            var w = new Wire(4);
            w.Play(); w.Active.Doubt(); w.Tick(3);
            int other = w.Oracle.State.TurnOrder.First(id => id != w.Oracle.State.Revelation.AccusedPlayerId && id != w.Oracle.State.Revelation.DoubterId);
            w.Disconnect(other); w.Tick(30);
            Assert.That(w.Active.CurrentView.Phase, Is.EqualTo(GamePhase.Playing));
            Assert.That(w.Active.RemainingTurnSeconds, Is.EqualTo(45));
        }

        [Test]
        public void DisconnectRemovesSeatAndPrivateHandForEveryRemainingClient()
        {
            var w = new Wire();
            int departed = w.Oracle.State.ActivePlayerId;
            w.Disconnect(departed);
            Assert.That(w.Host.PlayerCount, Is.EqualTo(2));
            foreach (var client in w.Clients.Values)
            {
                Assert.That(client.CurrentView.TurnOrder, Has.No.Member(departed));
                Assert.That(client.CurrentView.CardCounts[departed], Is.Zero);
                Assert.That(client.SeatOrder, Has.Count.EqualTo(3));
            }
        }

        [TestCase(2, TestName = "C-PRV-01 (flux hôte 2 joueurs)")]
        [TestCase(3, TestName = "C-PRV-01 (flux hôte 3 joueurs)")]
        [TestCase(4, TestName = "C-PRV-01 (flux hôte 4 joueurs)")]
        [TestCase(5, TestName = "C-PRV-01 (flux hôte 5 joueurs)")]
        [TestCase(6, TestName = "C-PRV-01 (flux hôte 6 joueurs)")]
        public void EveryPacketOfCompleteScriptedGameContainsOnlyOwnAndRevealedCards(int players)
        {
            var w = new Wire(players);
            int steps = 0;
            while (w.Oracle.State.Phase != GamePhase.Finished)
            {
                Assert.That(++steps, Is.LessThan(150));
                w.Play(Math.Min(4, w.Active.CurrentView.Hand.Count));
                if (w.Active.CurrentView.PendingWinnerId.HasValue) w.Play();
                else w.Active.Discard();
            }
            Assert.That(w.Views, Is.GreaterThan(players * 10));
            foreach (var client in w.Clients.Values) Assert.That(client.CurrentView.Phase, Is.EqualTo(GamePhase.Finished));
        }

        [TestCase(false, TestName = "C-PRV-02/03/04 (flux hôte mensonge)")]
        [TestCase(true, TestName = "C-PRV-02/03/04 (flux hôte vérité)")]
        public void RevelationAndPickupExposeOnlyLastPlayAndRecipientsHand(bool honest)
        {
            var w = new Wire();
            int second = w.Oracle.State.TurnOrder[1];
            var card = w.Client(second).CurrentView.Hand[0];
            var rank = honest ? card.Rank : (CardRank)((int)card.Rank % 13 + 1);
            w.Play(4, rank);
            int accused = w.Active.ControlledPlayerId;
            var hidden = w.Oracle.State.Pile.ToArray();
            w.Active.Play(new[] { card }, rank);
            var announcement = w.Trace.TakeLast(3).Select(p => ((ViewMessage)p.Message).View.LastAnnouncement);
            Assert.That(announcement.All(p => p.PlayerId == accused && p.CardCount == 1 && p.AnnouncedRank == rank), Is.True);
            w.Active.Doubt();
            var pile = w.Oracle.State.Pile.ToArray();
            int recipient = honest ? w.Active.ControlledPlayerId : accused;
            foreach (var client in w.Clients.Values)
            {
                Assert.That(client.CurrentView.Revelation.Cards, Is.EqualTo(new[] { card }));
                Assert.That(ProtocolTestData.ReachableCards(client.CurrentView).Overlaps(hidden), Is.False);
            }
            w.Tick(HostAuthority.RevelationDelaySeconds);
            foreach (var client in w.Clients.Values)
                Assert.That(pile.All(c => client.CurrentView.Hand.Contains(c)), Is.EqualTo(client.ControlledPlayerId == recipient));
        }

        [TestCase(TestName = "C-PRV-05 (flux hôte)")]
        public void DiscardAndDepartureNeverSendRemovedCards()
        {
            var w = new Wire(4);
            w.Play(4); w.Active.Discard(); w.Disconnect(w.Active.ControlledPlayerId);
            foreach (var client in w.Clients.Values)
                Assert.That(ProtocolTestData.ReachableCards(client.CurrentView).Overlaps(w.Oracle.State.RemovedCards), Is.False);
        }

        [TestCase(TestName = "C-PRV-06 (flux hôte)")]
        public void AutomaticRevelationOnPendingWinnerDepartureExposesOnlyFinalCards()
        {
            var w = new Wire();
            int target = w.Oracle.State.TurnOrder[1];
            int safety = 0;
            while (w.Client(target).CurrentView.Hand.Count > 2)
            {
                Assert.That(++safety, Is.LessThan(50));
                int count = w.Active.ControlledPlayerId == target ? Math.Min(4, w.Client(target).CurrentView.Hand.Count - 2) : 1;
                w.Play(count); w.Active.Discard();
            }
            int next = w.Oracle.State.TurnOrder[(w.Oracle.State.TurnOrder.ToList().IndexOf(target) + 1) % 3];
            if (w.Active.ControlledPlayerId == target) w.Tick(45);
            else if (w.Active.ControlledPlayerId != next) { w.Play(); w.Active.Discard(); }
            w.Play(4); w.Play(3);
            Assert.That(w.Active.ControlledPlayerId, Is.EqualTo(target));
            var final = w.Active.CurrentView.Hand.ToArray();
            w.Play(2);
            var earlier = w.Oracle.State.Pile.Except(final).ToArray();
            Assert.That(w.Oracle.State.Pile.Count, Is.EqualTo(9));
            w.Disconnect(target);
            foreach (var client in w.Clients.Values)
            {
                Assert.That(client.CurrentView.Revelation.Cards, Is.EqualTo(final));
                Assert.That(client.CurrentView.Revelation.IsAutomatic, Is.True);
                Assert.That(ProtocolTestData.ReachableCards(client.CurrentView).Overlaps(earlier), Is.False);
            }
            w.Tick(4);
            Assert.That(w.Oracle.State.Phase, Is.Not.EqualTo(GamePhase.Revelation));
        }
    }
}
