using System;
using System.Collections.Generic;
using System.Linq;
using Dubito.Core;
using Dubito.Protocol;
using Dubito.Tests.Core;
using NUnit.Framework;

namespace Dubito.Tests.Protocol
{
    public class ProtocolRoundTripTests
    {
        private static IEnumerable<TestCaseData> Messages() => ProtocolTestData.Messages()
            .Select((m, i) => new TestCaseData(m).SetName("ProtocolRoundTrip_" + m.GetType().Name + "_" + i));

        [TestCaseSource(nameof(Messages))]
        public void EveryMessagePreservesEveryField(IProtocolMessage message)
        {
            var result = ProtocolCodec.Decode(ProtocolCodec.Encode(message));
            Assert.That(result.Success, Is.True, result.Error);
            Assert.That(result.Error, Is.Null);
            // Dictionary serialization uses canonical ID order; compare by keys.
            if (message is WelcomeMessage welcome)
            {
                var copy = (WelcomeMessage)result.Message;
                Assert.That(copy.ProtocolVersion, Is.EqualTo(welcome.ProtocolVersion));
                Assert.That(copy.PlayerId, Is.EqualTo(welcome.PlayerId));
                Assert.That(copy.SeatOrder, Is.EqualTo(welcome.SeatOrder));
                Assert.That(copy.PlayerNames, Is.EquivalentTo(welcome.PlayerNames));
            }
            else SnapshotAssert.Equal(message, result.Message);
            Assert.That(ProtocolCodec.Encode(result.Message), Is.EqualTo(ProtocolCodec.Encode(message)));
        }

        [TestCase(CommandKind.Play)] [TestCase(CommandKind.Doubt)] [TestCase(CommandKind.Discard)]
        public void CommandsConvertBothWaysWithoutIdentity(CommandKind kind)
        {
            var core = new GameCommand(ProtocolTestData.CommandId, 123, kind,
                Deck.CreateStandard().Take(4), CardRank.Seven);
            var restored = ProtocolTestData.RoundTrip(CommandMessage.FromGameCommand(core)).ToGameCommand();
            SnapshotAssert.Equal(core, restored);
            Assert.That(typeof(CommandMessage).GetProperties().Any(p => p.Name == "PlayerId"), Is.False);
        }

        [Test]
        public void WireHeaderAndGuidLayoutAreStableLittleEndian()
        {
            Assert.That(ProtocolInfo.Version, Is.EqualTo(1));
            Assert.That(ProtocolCodec.Encode(new ReadyMessage(true)), Is.EqualTo(new byte[] { 3, 1, 0, 0, 0, 1 }));
            byte[] bytes = ProtocolCodec.Encode(new CommandMessage(ProtocolTestData.CommandId,
                0x0102030405060708, CommandKind.Play, new[] { new Card(CardRank.King, CardSuit.Spades) }, CardRank.King));
            Assert.That(bytes.Take(5), Is.EqualTo(new byte[] { 2, 1, 0, 0, 0 }));
            Assert.That(bytes.Skip(5).Take(16), Is.EqualTo(ProtocolTestData.CommandId.ToByteArray()));
            Assert.That(bytes.Skip(21).Take(8), Is.EqualTo(new byte[] { 8, 7, 6, 5, 4, 3, 2, 1 }));
            Assert.That(bytes.Skip(29), Is.EqualTo(new byte[] { 0, 1, 51, 13 }));
        }

        [Test]
        public void CollectionsAreCopiedAndImmutable()
        {
            var cards = Deck.CreateStandard().Take(2).ToArray();
            var command = new CommandMessage(ProtocolTestData.CommandId, 0, CommandKind.Play, cards);
            var saved = ProtocolCodec.Encode(command); cards[0] = cards[1];
            Assert.That(ProtocolCodec.Encode(command), Is.EqualTo(saved));
            Assert.Throws<NotSupportedException>(() => ((IList<Card>)command.Cards).Clear());
            var names = new Dictionary<int, string> { [0] = "A", [1] = "B" };
            var seats = new[] { 1, 0 };
            var welcome = new WelcomeMessage(1, 0, names, seats);
            names[0] = "Changed"; seats[0] = 0;
            Assert.That(welcome.PlayerNames[0], Is.EqualTo("A"));
            Assert.That(welcome.SeatOrder, Is.EqualTo(new[] { 1, 0 }));
            Assert.Throws<NotSupportedException>(() => ((IDictionary<int, string>)welcome.PlayerNames).Clear());
            Assert.Throws<NotSupportedException>(() => ((IList<int>)welcome.SeatOrder).Clear());
            var players = new[] { new LobbyPlayer(0, "A", true) };
            var lobby = new LobbyStateMessage(players, 45, 2, 0, 0); players[0] = null;
            Assert.That(lobby.Players[0].PlayerName, Is.EqualTo("A"));
            Assert.Throws<NotSupportedException>(() => ((IList<LobbyPlayer>)lobby.Players).Clear());
        }
    }
}
