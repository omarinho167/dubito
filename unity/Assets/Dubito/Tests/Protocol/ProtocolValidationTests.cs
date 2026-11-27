using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using Dubito.Core;
using Dubito.Protocol;
using NUnit.Framework;

namespace Dubito.Tests.Protocol
{
    public class ProtocolValidationTests
    {
        // Independent wire writer deliberately bypasses all production constructors.
        private static byte[] Packet(byte type, Action<BinaryWriter> payload, int wireVersion = 1)
        {
            using var stream = new MemoryStream();
            using var w = new BinaryWriter(stream, Encoding.UTF8, true);
            w.Write(type); w.Write(wireVersion); payload(w); w.Flush();
            return stream.ToArray();
        }
        private static void Bytes(BinaryWriter w, byte[] bytes) { w.Write((byte)bytes.Length); w.Write(bytes); }
        private static void Name(BinaryWriter w, string name)
        {
            byte[] bytes = Encoding.UTF8.GetBytes(name); w.Write((ushort)bytes.Length); w.Write(bytes);
        }
        private static byte[] Command(byte kind = 0, byte[] cards = null, byte rank = 0,
            long revision = 0, bool emptyId = false)
            => Packet(2, w =>
            {
                w.Write((emptyId ? Guid.Empty : ProtocolTestData.CommandId).ToByteArray());
                w.Write(revision); w.Write(kind); Bytes(w, cards ?? Array.Empty<byte>()); w.Write(rank);
            });

        private sealed class WireView
        {
            internal byte Player = 0;
            internal byte[] Hand = { 0 }, Counts = { 1, 2 }, Order = { 0, 1 };
            internal byte Pile = 0, Rank = 0, AnnouncementFlag = 0;
            internal byte[] Announcement = { 0, 1, 1 };
            internal byte Active = 0, Paused = 0, Phase = 0, RevealFlag = 0;
            internal double Time = 45;
            internal byte Accused = 0, Doubter = 1, Automatic = 0, Correct = 0, Recipient = 0;
            internal byte[] RevealCards = { 3 };
            internal byte Pending = 255, Winner = 255, Skipped = 0;
            internal long Revision = 0;
            internal byte[] Actions = { 0 };
            internal byte[] Encode() => Packet(5, w =>
            {
                w.Write(Player); Bytes(w, Hand); Bytes(w, Counts); Bytes(w, Order);
                w.Write(Pile); w.Write(Rank); w.Write(AnnouncementFlag);
                if (AnnouncementFlag != 0) w.Write(Announcement);
                w.Write(Active); w.Write(Time); w.Write(Paused); w.Write(Phase); w.Write(RevealFlag);
                if (RevealFlag != 0)
                {
                    w.Write(Accused); w.Write(Doubter); w.Write(Automatic); Bytes(w, RevealCards);
                    w.Write(Correct); w.Write(Recipient);
                }
                w.Write(Pending); w.Write(Winner); w.Write(Skipped); w.Write(Revision); Bytes(w, Actions);
            });
        }
        private static byte[] View(Action<WireView> change)
        { var v = new WireView(); change(v); return v.Encode(); }
        private static TestCaseData Bad(string reason, byte[] packet)
            => new TestCaseData(packet).SetName("Reject_" + reason);

        private static IEnumerable<TestCaseData> InvalidPackets()
        {
            yield return Bad("Null", null);
            yield return Bad("Empty", Array.Empty<byte>());
            yield return Bad("Over16KiB", new byte[ProtocolInfo.MaxMessageBytes + 1]);
            yield return Bad("UnknownType", Packet(255, _ => { }));
            yield return Bad("UnknownWireVersion", Packet(3, w => w.Write((byte)1), 2));
            yield return Bad("ZeroWireVersion", Packet(3, w => w.Write((byte)1), 0));
            yield return Bad("NegativeWireVersion", Packet(3, w => w.Write((byte)1), -1));
            yield return Bad("TrailingBytes", Packet(3, w => { w.Write((byte)1); w.Write((byte)0); }));
            yield return Bad("NonCanonicalBoolean", Packet(3, w => w.Write((byte)2)));
            yield return Bad("HelloNegativeVersion", Packet(1, w => { w.Write(-1); Name(w, "A"); }));
            yield return Bad("HelloOver32Characters", Packet(1, w => { w.Write(1); Name(w, new string('A', 33)); }));
            yield return Bad("NameOver128Bytes", Packet(1, w => { w.Write(1); w.Write((ushort)129); w.Write(new byte[129]); }));
            yield return Bad("NameHostileLength", Packet(1, w => { w.Write(1); w.Write(ushort.MaxValue); }));
            yield return Bad("MalformedUtf8", Packet(1, w => { w.Write(1); w.Write((ushort)2); w.Write(new byte[] { 0xc0, 0xaf }); }));
            yield return Bad("TruncatedUtf8", Packet(1, w => { w.Write(1); w.Write((ushort)1); w.Write((byte)0xc3); }));
            yield return Bad("StringLengthPastEnd", Packet(1, w => { w.Write(1); w.Write((ushort)32); w.Write((byte)65); }));
            yield return Bad("CommandKind", Command(kind: 3));
            yield return Bad("CommandRank", Command(rank: 14));
            yield return Bad("CommandOver4Cards", Command(cards: new byte[] { 0, 1, 2, 3, 4 }));
            yield return Bad("CommandCard52", Command(cards: new byte[] { 52 }));
            yield return Bad("CommandCard255", Command(cards: new byte[] { 255 }));
            yield return Bad("CommandDuplicateCards", Command(cards: new byte[] { 0, 0 }));
            yield return Bad("CommandNegativeRevision", Command(revision: -1));
            yield return Bad("CommandEmptyId", Command(emptyId: true));
            yield return Bad("UnknownErrorEnum", Packet(6, w => { w.Write(ProtocolTestData.CommandId.ToByteArray()); w.Write((byte)18); }));
            yield return Bad("RejectedEmptyId", Packet(6, w => { w.Write(new byte[16]); w.Write((byte)1); }));
            yield return Bad("MismatchZeroVersion", Packet(7, w => w.Write(0)));

            yield return Bad("ViewRecipient", View(v => v.Player = 2));
            yield return Bad("ViewHandOver52", View(v => v.Hand = new byte[53]));
            yield return Bad("ViewInvalidCard", View(v => v.Hand = new byte[] { 52 }));
            yield return Bad("ViewDuplicateCard", View(v => { v.Hand = new byte[] { 0, 0 }; v.Counts[0] = 2; }));
            yield return Bad("ViewTooFewPlayers", View(v => v.Counts = new byte[] { 1 }));
            yield return Bad("ViewTooManyPlayers", View(v => v.Counts = new byte[7]));
            yield return Bad("ViewCountOver52", View(v => v.Counts[1] = 53));
            yield return Bad("ViewOwnCountMismatch", View(v => v.Counts[0] = 0));
            yield return Bad("ViewTooManySeats", View(v => v.Order = new byte[7]));
            yield return Bad("ViewSeatCountExceedsRoster", View(v => v.Order = new byte[] { 0, 1, 0 }));
            yield return Bad("ViewSeatOutsideRoster", View(v => v.Order[1] = 2));
            yield return Bad("ViewDuplicateSeats", View(v => v.Order[1] = 0));
            yield return Bad("ViewPileOver52", View(v => v.Pile = 53));
            yield return Bad("ViewRank", View(v => v.Rank = 14));
            yield return Bad("ViewAnnouncementFlag", View(v => v.AnnouncementFlag = 2));
            yield return Bad("ViewAnnouncementPlayer", View(v => { v.AnnouncementFlag = 1; v.Announcement[0] = 2; }));
            yield return Bad("ViewAnnouncementCountZero", View(v => { v.AnnouncementFlag = 1; v.Announcement[1] = 0; }));
            yield return Bad("ViewAnnouncementCountOver4", View(v => { v.AnnouncementFlag = 1; v.Announcement[1] = 5; }));
            yield return Bad("ViewAnnouncementRank", View(v => { v.AnnouncementFlag = 1; v.Announcement[2] = 0; }));
            yield return Bad("ViewActivePlayer", View(v => v.Active = 2));
            yield return Bad("ViewNaNTime", View(v => v.Time = double.NaN));
            yield return Bad("ViewInfiniteTime", View(v => v.Time = double.PositiveInfinity));
            yield return Bad("ViewNegativeTime", View(v => v.Time = -1));
            yield return Bad("ViewTimeOver90", View(v => v.Time = 90.1));
            yield return Bad("ViewPausedFlag", View(v => v.Paused = 2));
            yield return Bad("ViewGamePhase", View(v => v.Phase = 3));
            yield return Bad("ViewRevealFlag", View(v => v.RevealFlag = 2));
            yield return Bad("ViewRevealEmpty", View(v => { v.RevealFlag = 1; v.RevealCards = Array.Empty<byte>(); }));
            yield return Bad("ViewRevealOver4", View(v => { v.RevealFlag = 1; v.RevealCards = new byte[5]; }));
            yield return Bad("ViewRevealInvalidCard", View(v => { v.RevealFlag = 1; v.RevealCards = new byte[] { 52 }; }));
            yield return Bad("ViewRevealDuplicateCard", View(v => { v.RevealFlag = 1; v.RevealCards = new byte[] { 3, 3 }; }));
            yield return Bad("ViewRevealAccused", View(v => { v.RevealFlag = 1; v.Accused = 2; }));
            yield return Bad("ViewRevealDoubter", View(v => { v.RevealFlag = 1; v.Doubter = 2; }));
            yield return Bad("ViewRevealRecipient", View(v => { v.RevealFlag = 1; v.Recipient = 2; }));
            yield return Bad("ViewRevealAutomaticFlag", View(v => { v.RevealFlag = 1; v.Automatic = 2; }));
            yield return Bad("ViewRevealVerdictFlag", View(v => { v.RevealFlag = 1; v.Correct = 2; }));
            yield return Bad("ViewPendingWinner", View(v => v.Pending = 2));
            yield return Bad("ViewWinner", View(v => v.Winner = 2));
            yield return Bad("ViewSkippedTurns", View(v => v.Skipped = 3));
            yield return Bad("ViewRevision", View(v => v.Revision = -1));
            yield return Bad("ViewActionEnum", View(v => v.Actions = new byte[] { 3 }));
            yield return Bad("ViewDuplicateAction", View(v => v.Actions = new byte[] { 0, 0 }));
            yield return Bad("ViewActionCount", View(v => v.Actions = new byte[4]));
            yield return Bad("LobbyMaxBelow2", Lobby(max: 1));
            yield return Bad("LobbyMaxOver6", Lobby(max: 7));
            yield return Bad("LobbyInvalidTimer", Lobby(duration: 46));
            yield return Bad("LobbyEmpty", Lobby(ids: Array.Empty<byte>()));
            yield return Bad("LobbyTooManyConnected", Lobby(max: 2, ids: new byte[] { 0, 1, 2 }));
            yield return Bad("LobbyOver6Connected", Lobby(ids: new byte[7]));
            yield return Bad("LobbyDuplicateId", Lobby(ids: new byte[] { 0, 0 }));
            yield return Bad("LobbyIdOutOfRange", Lobby(ids: new byte[] { 6 }));
            yield return Bad("LobbyReadyFlag", Lobby(ready: 2));
            yield return Bad("LobbyRecipientMissing", Lobby(you: 3));
            yield return Bad("LobbyHostMissing", Lobby(host: 4));
            yield return Bad("JoinRefusedZeroReason", Packet(9, w => w.Write((byte)0)));
            yield return Bad("JoinRefusedUnknownReason", Packet(9, w => w.Write((byte)3)));
            yield return Bad("TableEventUnknownKind", Packet(10, w => { w.Write((byte)4); w.Write((byte)0); }));
            yield return Bad("TableEventZeroKind", Packet(10, w => { w.Write((byte)0); w.Write((byte)0); }));
            yield return Bad("TableEventPlayerOutOfRange", Packet(10, w => { w.Write((byte)1); w.Write((byte)6); }));
            yield return Bad("WelcomeInvalidVersion", Welcome(version: 0));
            yield return Bad("WelcomeUnknownRecipient", Welcome(recipient: 2));
            yield return Bad("WelcomeTooFewPlayers", Welcome(ids: new byte[] { 0 }));
            yield return Bad("WelcomeTooManyPlayers", Welcome(ids: new byte[7]));
            yield return Bad("WelcomeDuplicateId", Welcome(ids: new byte[] { 0, 0 }));
            yield return Bad("WelcomeIdOutOfRange", Welcome(ids: new byte[] { 0, 6 }));
            yield return Bad("WelcomeMissingSeat", Welcome(order: new byte[] { 0 }));
            yield return Bad("WelcomeDuplicateSeat", Welcome(order: new byte[] { 0, 0 }));
            yield return Bad("WelcomeUnknownSeat", Welcome(order: new byte[] { 0, 2 }));
        }
        private static byte[] Lobby(byte max = 2, byte duration = 45, byte[] ids = null, byte ready = 1,
            byte you = 0, byte host = 0)
            => Packet(8, w =>
            {
                w.Write(max); w.Write(duration); var players = ids ?? new byte[] { 0 };
                w.Write((byte)players.Length);
                foreach (byte id in players) { w.Write(id); Name(w, "A"); w.Write(ready); }
                w.Write(you); w.Write(host);
            });
        private static byte[] Welcome(int version = 1, byte recipient = 0, byte[] ids = null, byte[] order = null)
            => Packet(4, w =>
            {
                w.Write(version); w.Write(recipient); var players = ids ?? new byte[] { 0, 1 };
                w.Write((byte)players.Length);
                foreach (byte id in players) { w.Write(id); Name(w, "A"); }
                Bytes(w, order ?? new byte[] { 1, 0 });
            });

        [TestCaseSource(nameof(InvalidPackets))]
        public void HostileFieldsAreRejected(byte[] bytes)
        {
            DecodeResult result = default;
            Assert.DoesNotThrow(() => result = ProtocolCodec.Decode(bytes));
            Assert.That(result.Success, Is.False);
            Assert.That(result.Message, Is.Null);
            Assert.That(result.Error, Is.Not.Null.And.Not.Empty);
        }

        [Test]
        public void EveryTruncatedMessageFailsCleanly()
        {
            foreach (var message in ProtocolTestData.Messages())
            {
                byte[] bytes = ProtocolCodec.Encode(message);
                for (int length = 0; length < bytes.Length; length++)
                {
                    var truncated = bytes.Take(length).ToArray();
                    Assert.That(ProtocolCodec.Decode(truncated).Success, Is.False,
                        message.GetType().Name + " truncated at " + length);
                }
                Assert.That(ProtocolCodec.Decode(bytes.Concat(new byte[] { 0 }).ToArray()).Success,
                    Is.False, message.GetType().Name + " trailing byte");
            }
        }

        [Test]
        public void FuzzRandomBytesAndMutatedValidMessagesNeverThrow()
        {
            var random = new Random(62031);
            for (int i = 0; i < 10000; i++)
            {
                var bytes = new byte[random.Next(0, i % 100 == 0 ? ProtocolInfo.MaxMessageBytes + 2 : 256)];
                random.NextBytes(bytes);
                var result = ProtocolCodec.Decode(bytes); // any unhandled exception fails the test
                Assert.That(result.Success, Is.False, "Random packet " + i);
                Assert.That(result.Error, Is.Not.Empty);
            }
            foreach (var message in ProtocolTestData.Messages())
            {
                byte[] original = ProtocolCodec.Encode(message);
                for (int index = 0; index < original.Length; index++)
                {
                    var bytes = (byte[])original.Clone(); bytes[index] ^= 0xff;
                    var result = ProtocolCodec.Decode(bytes);
                    if (result.Success)
                        Assert.That(ProtocolCodec.Decode(ProtocolCodec.Encode(result.Message)).Success, Is.True);
                    else Assert.That(result.Error, Is.Not.Empty);
                }
            }
        }

        [Test]
        public void BoundaryValuesRemainAccepted()
        {
            var v = new WireView
            {
                Hand = Enumerable.Range(0, 52).Select(i => (byte)i).ToArray(),
                Counts = new byte[] { 52, 0 }, Order = Array.Empty<byte>(), Pile = 52,
                Rank = 13, Active = 255, Time = 90, Phase = 2, Paused = 1,
                Pending = 0, Winner = 1, Skipped = 2, Revision = long.MaxValue,
                Actions = new byte[] { 0, 1, 2 }, AnnouncementFlag = 1,
                Announcement = new byte[] { 1, 4, 13 }, RevealFlag = 1,
                RevealCards = new byte[] { 4, 5, 6, 7 }, Recipient = 255
            };
            var result = ProtocolCodec.Decode(v.Encode());
            Assert.That(result.Success, Is.True, result.Error);
            var view = ((ViewMessage)result.Message).View;
            Assert.That(view.Hand.Count, Is.EqualTo(52));
            Assert.That(view.ActivePlayerId, Is.EqualTo(-1));
            Assert.That(view.Revision, Is.EqualTo(long.MaxValue));
            Assert.That(view.Revelation.PileRecipientId, Is.Null);
            Assert.That(ProtocolCodec.Decode(Command(cards: new byte[] { 0, 1, 50, 51 }, rank: 13,
                revision: long.MaxValue)).Success, Is.True);
            Assert.That(ProtocolCodec.Decode(Lobby(max: 6, duration: 90, ids: new byte[] { 0, 1, 2, 3, 4, 5 })).Success, Is.True);
            Assert.That(ProtocolCodec.Decode(Welcome(ids: new byte[] { 0, 1, 2, 3, 4, 5 },
                order: new byte[] { 5, 4, 3, 2, 1, 0 })).Success, Is.True);
            ProtocolTestData.RoundTrip(new HelloMessage(1, new string('é', 32)));
            ProtocolTestData.RoundTrip(new HelloMessage(1, string.Concat(Enumerable.Repeat("😀", 16))));
        }

        private sealed class UnsupportedMessage : IProtocolMessage { }

        [Test]
        public void LocalConstructionRejectsInvalidCardsEnumsAndNames()
        {
            Assert.Throws<ArgumentNullException>(() => ProtocolCodec.Encode(null));
            Assert.Throws<ArgumentException>(() => ProtocolCodec.Encode(new UnsupportedMessage()));
            Assert.Throws<EncoderFallbackException>(() => new HelloMessage(1, "\ud800"));
            Assert.Throws<ArgumentException>(() => new HelloMessage(1, new string('a', 33)));
            Assert.Throws<ArgumentNullException>(() => new HelloMessage(1, null));
            Assert.Throws<ArgumentException>(() => new CommandMessage(ProtocolTestData.CommandId, 0,
                CommandKind.Play, new[] { default(Card) }));
            Assert.Throws<ArgumentException>(() => new CommandMessage(ProtocolTestData.CommandId, 0, (CommandKind)256));
            Assert.Throws<ArgumentException>(() => new CommandMessage(ProtocolTestData.CommandId, 0,
                CommandKind.Play, announcedRank: (CardRank)256));
            Assert.Throws<ArgumentNullException>(() => CommandMessage.FromGameCommand(null));
            Assert.Throws<ArgumentNullException>(() => new ViewMessage(null));
        }
    }
}
