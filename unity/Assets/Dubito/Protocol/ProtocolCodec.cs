using System;
using System.Collections.Generic;
using System.IO;
using Dubito.Core;

namespace Dubito.Protocol
{
    public readonly struct DecodeResult
    {
        public bool Success => Message != null;
        public IProtocolMessage Message { get; }
        public string Error { get; }
        internal DecodeResult(IProtocolMessage message, string error) { Message = message; Error = error; }
    }

    // V1: type:u8, wireVersion:i32, payload. BinaryWriter uses little endian.
    // Counts/IDs/enums/bools/cards use u8; null rank=0, null ID/active -1=255.
    // Strings: byteLength:u16 + strict UTF-8; GUID: .NET Guid.ToByteArray order.
    // Revisions:i64, remaining time:f64. No padding, extension fields or trailing bytes.
    public static class ProtocolCodec
    {
        private enum MessageType : byte
        {
            Hello = 1, Command = 2, Ready = 3, Welcome = 4,
            View = 5, CommandRejected = 6, VersionMismatch = 7, LobbyState = 8, JoinRefused = 9
        }

        public static byte[] Encode(IProtocolMessage message)
        {
            if (message == null) throw new ArgumentNullException(nameof(message));
            using var stream = new MemoryStream();
            using var writer = new BinaryWriter(stream, Check.Utf8, true);
            switch (message)
            {
                case HelloMessage hello:
                    Header(writer, MessageType.Hello);
                    writer.Write(hello.ProtocolVersion); Name(writer, hello.PlayerName);
                    break;
                case CommandMessage command:
                    Header(writer, MessageType.Command);
                    writer.Write(command.Id.ToByteArray()); writer.Write(command.ExpectedRevision);
                    writer.Write((byte)command.Kind); Cards(writer, command.Cards);
                    Rank(writer, command.AnnouncedRank);
                    break;
                case ReadyMessage ready:
                    Header(writer, MessageType.Ready); writer.Write(ready.IsReady);
                    break;
                case WelcomeMessage welcome:
                    Header(writer, MessageType.Welcome);
                    writer.Write(welcome.ProtocolVersion); writer.Write((byte)welcome.PlayerId);
                    writer.Write((byte)welcome.PlayerNames.Count);
                    // Stable canonical map ordering, independent of dictionary implementation.
                    for (int id = 0; id < 6; id++)
                        if (welcome.PlayerNames.TryGetValue(id, out string name))
                        { writer.Write((byte)id); Name(writer, name); }
                    Ints(writer, welcome.SeatOrder);
                    break;
                case ViewMessage view:
                    Header(writer, MessageType.View); WriteView(writer, view.View);
                    break;
                case CommandRejectedMessage rejected:
                    Header(writer, MessageType.CommandRejected);
                    writer.Write(rejected.CommandId.ToByteArray()); writer.Write((byte)rejected.Error);
                    break;
                case VersionMismatchMessage mismatch:
                    Header(writer, MessageType.VersionMismatch); writer.Write(mismatch.ExpectedVersion);
                    break;
                case LobbyStateMessage lobby:
                    Header(writer, MessageType.LobbyState);
                    writer.Write((byte)lobby.MaxPlayers); writer.Write((byte)lobby.TurnDurationSeconds);
                    writer.Write((byte)lobby.Players.Count);
                    foreach (var player in lobby.Players)
                    { writer.Write((byte)player.PlayerId); Name(writer, player.PlayerName); writer.Write(player.IsReady); }
                    writer.Write((byte)lobby.YourPlayerId); writer.Write((byte)lobby.HostPlayerId);
                    break;
                case JoinRefusedMessage refused:
                    Header(writer, MessageType.JoinRefused); writer.Write((byte)refused.Reason);
                    break;
                default: throw new ArgumentException("Unsupported protocol message.", nameof(message));
            }
            writer.Flush();
            Check.Require(stream.Length <= ProtocolInfo.MaxMessageBytes, "Message too long.");
            return stream.ToArray();
        }

        public static DecodeResult Decode(byte[] bytes)
        {
            if (bytes == null || bytes.Length > ProtocolInfo.MaxMessageBytes)
                return new DecodeResult(null, "Null or oversized message.");
            try
            {
                using var stream = new MemoryStream(bytes, false);
                using var reader = new BinaryReader(stream, Check.Utf8, true);
                var type = (MessageType)reader.ReadByte();
                Check.EnumValue(type);
                Check.Require(reader.ReadInt32() == ProtocolInfo.Version, "Unsupported wire version.");
                IProtocolMessage message;
                switch (type)
                {
                    case MessageType.Hello:
                        message = new HelloMessage(reader.ReadInt32(), ReadName(reader)); break;
                    case MessageType.Command:
                        message = new CommandMessage(new Guid(Exact(reader, 16)), reader.ReadInt64(),
                            (CommandKind)reader.ReadByte(), ReadCards(reader, 4), ReadRank(reader)); break;
                    case MessageType.Ready:
                        message = new ReadyMessage(Bool(reader)); break;
                    case MessageType.Welcome:
                        int version = reader.ReadInt32(), playerId = reader.ReadByte();
                        int count = Count(reader, 2, 6);
                        var names = new Dictionary<int, string>();
                        for (int i = 0; i < count; i++) names.Add(reader.ReadByte(), ReadName(reader));
                        message = new WelcomeMessage(version, playerId, names, ReadInts(reader, 2, 6)); break;
                    case MessageType.View:
                        message = new ViewMessage(ReadView(reader)); break;
                    case MessageType.CommandRejected:
                        message = new CommandRejectedMessage(new Guid(Exact(reader, 16)),
                            (CommandError)reader.ReadByte()); break;
                    case MessageType.VersionMismatch:
                        message = new VersionMismatchMessage(reader.ReadInt32()); break;
                    case MessageType.LobbyState:
                        int max = reader.ReadByte(), duration = reader.ReadByte();
                        int connected = Count(reader, 1, 6);
                        var players = new List<LobbyPlayer>(connected);
                        for (int i = 0; i < connected; i++)
                            players.Add(new LobbyPlayer(reader.ReadByte(), ReadName(reader), Bool(reader)));
                        message = new LobbyStateMessage(players, duration, max, reader.ReadByte(), reader.ReadByte()); break;
                    case MessageType.JoinRefused:
                        message = new JoinRefusedMessage((JoinRefusal)reader.ReadByte()); break;
                    default: throw new ArgumentException("Unknown message type.");
                }
                Check.Require(stream.Position == stream.Length, "Trailing message bytes.");
                return new DecodeResult(message, null);
            }
            // Every length is bounded before allocation. These are the only parse/validation
            // failures; programming bugs are not hidden by a catch-all.
            catch (IOException) { return new DecodeResult(null, "Truncated or invalid message."); }
            catch (ArgumentException) { return new DecodeResult(null, "Invalid message fields."); }
            catch (OverflowException) { return new DecodeResult(null, "Numeric overflow."); }
        }

        private static void Header(BinaryWriter writer, MessageType type)
        { writer.Write((byte)type); writer.Write(ProtocolInfo.Version); }
        private static void Name(BinaryWriter writer, string value)
        {
            byte[] bytes = Check.Utf8.GetBytes(value);
            writer.Write((ushort)bytes.Length); writer.Write(bytes);
        }
        private static string ReadName(BinaryReader reader)
        {
            int length = reader.ReadUInt16();
            Check.Range(length, 0, ProtocolInfo.MaxPlayerNameBytes, "Name bytes");
            string value = Check.Utf8.GetString(Exact(reader, length));
            Check.Name(value);
            return value;
        }
        private static byte[] Exact(BinaryReader reader, int length)
        {
            byte[] result = reader.ReadBytes(length);
            if (result.Length != length) throw new EndOfStreamException();
            return result;
        }
        private static bool Bool(BinaryReader reader)
        {
            byte value = reader.ReadByte();
            Check.Range(value, 0, 1, "Boolean");
            return value == 1;
        }
        private static int Count(BinaryReader reader, int min, int max)
        {
            int count = reader.ReadByte();
            Check.Range(count, min, max, "Collection count");
            return count;
        }
        private static void Cards(BinaryWriter writer, IReadOnlyList<Card> cards)
        {
            writer.Write((byte)cards.Count);
            foreach (var card in cards) writer.Write((byte)(((int)card.Rank - 1) * 4 + (int)card.Suit));
        }
        private static IReadOnlyList<Card> ReadCards(BinaryReader reader, int max)
        {
            int count = Count(reader, 0, max);
            var cards = new List<Card>(count);
            var seen = new HashSet<byte>();
            for (int i = 0; i < count; i++)
            {
                byte id = reader.ReadByte();
                Check.Require(id < 52 && seen.Add(id), "Invalid or duplicate card.");
                cards.Add(new Card((CardRank)(id / 4 + 1), (CardSuit)(id % 4)));
            }
            return cards;
        }
        private static void Rank(BinaryWriter writer, CardRank? rank) => writer.Write((byte)(rank ?? 0));
        private static CardRank? ReadRank(BinaryReader reader)
        {
            byte rank = reader.ReadByte();
            if (rank == 0) return null;
            var result = (CardRank)rank;
            Check.EnumValue(result);
            return result;
        }
        private static void Id(BinaryWriter writer, int? id) => writer.Write((byte)(id ?? 255));
        private static int? ReadId(BinaryReader reader)
        {
            byte id = reader.ReadByte();
            return id == 255 ? (int?)null : id;
        }
        private static void Ints(BinaryWriter writer, IReadOnlyList<int> values)
        {
            writer.Write((byte)values.Count);
            foreach (int value in values) writer.Write((byte)value);
        }
        private static IReadOnlyList<int> ReadInts(BinaryReader reader, int min, int max)
        {
            int count = Count(reader, min, max);
            var values = new List<int>(count);
            for (int i = 0; i < count; i++) values.Add(reader.ReadByte());
            return values;
        }

        // PlayerView field order is fixed for this wire version.
        private static void WriteView(BinaryWriter writer, PlayerView view)
        {
            writer.Write((byte)view.PlayerId); Cards(writer, view.Hand);
            Ints(writer, view.CardCounts); Ints(writer, view.TurnOrder);
            writer.Write((byte)view.PileCount); Rank(writer, view.RoundRank);
            writer.Write(view.LastAnnouncement != null);
            if (view.LastAnnouncement != null)
            {
                var play = view.LastAnnouncement;
                writer.Write((byte)play.PlayerId); writer.Write((byte)play.CardCount);
                Rank(writer, play.AnnouncedRank);
            }
            Id(writer, view.ActivePlayerId == -1 ? (int?)null : view.ActivePlayerId);
            writer.Write(view.RemainingTurnSeconds); writer.Write(view.IsTimerPaused);
            writer.Write((byte)view.Phase); writer.Write(view.Revelation != null);
            if (view.Revelation != null)
            {
                var reveal = view.Revelation;
                writer.Write((byte)reveal.AccusedPlayerId); writer.Write((byte)reveal.DoubterId);
                writer.Write(reveal.IsAutomatic); Cards(writer, reveal.Cards);
                writer.Write(reveal.IsDoubtCorrect); Id(writer, reveal.PileRecipientId);
            }
            Id(writer, view.PendingWinnerId); Id(writer, view.WinnerId);
            writer.Write((byte)view.OwnSkippedTurns); writer.Write(view.Revision);
            writer.Write((byte)view.AllowedActions.Count);
            foreach (var action in view.AllowedActions) writer.Write((byte)action);
        }
        private static PlayerView ReadView(BinaryReader reader)
        {
            int player = reader.ReadByte();
            var hand = ReadCards(reader, 52);
            var counts = ReadInts(reader, 2, 6);
            var order = ReadInts(reader, 0, 6);
            int pile = reader.ReadByte(); var rank = ReadRank(reader);
            PlayAnnouncement play = null;
            if (Bool(reader))
                play = PlayAnnouncement.Restore(reader.ReadByte(), reader.ReadByte(), (CardRank)reader.ReadByte());
            int active = ReadId(reader) ?? -1;
            double remaining = reader.ReadDouble(); bool paused = Bool(reader);
            var phase = (GamePhase)reader.ReadByte();
            RevelationView reveal = null;
            if (Bool(reader))
            {
                int accused = reader.ReadByte(), doubter = reader.ReadByte();
                bool automatic = Bool(reader); var cards = ReadCards(reader, 4);
                reveal = RevelationView.Restore(accused, doubter, automatic, cards, Bool(reader), ReadId(reader));
            }
            int? pending = ReadId(reader), winner = ReadId(reader);
            int skipped = reader.ReadByte(); long revision = reader.ReadInt64();
            int actionCount = Count(reader, 0, 3);
            var actions = new List<CommandKind>(actionCount);
            for (int i = 0; i < actionCount; i++) actions.Add((CommandKind)reader.ReadByte());
            return PlayerView.Restore(player, hand, counts, order, pile, rank, play, active,
                remaining, paused, phase, reveal, pending, winner, skipped, revision, actions);
        }
    }
}
