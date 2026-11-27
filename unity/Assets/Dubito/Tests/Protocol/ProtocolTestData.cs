using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using Dubito.Core;
using Dubito.Protocol;
using Dubito.Tests.Core;
using NUnit.Framework;

namespace Dubito.Tests.Protocol
{
    internal static class ProtocolTestData
    {
        internal static readonly Guid CommandId = new Guid("12345678-90ab-cdef-1234-567890abcdef");
        internal static GameEngine Game(int players = 3, int seed = 6291)
            => new GameEngine(GameSetup.Create(new GameConfig(players), new SystemRandomSource(seed)));
        internal static void Send(GameEngine engine, CommandKind kind, int count = 0, CardRank? rank = null)
        {
            int p = engine.State.ActivePlayerId;
            var result = engine.Execute(p, new GameCommand(CommandIdFor(engine), engine.State.Revision,
                kind, engine.State.Hands[p].Take(count), rank));
            Assert.That(result.Error, Is.EqualTo(CommandError.None));
        }
        private static Guid CommandIdFor(GameEngine engine)
        {
            var bytes = CommandId.ToByteArray();
            Array.Copy(BitConverter.GetBytes(engine.State.Revision + 1), bytes, 8);
            return new Guid(bytes);
        }
        internal static GameEngine BeforeDoubt(bool honest = false)
        {
            var e = Game();
            int second = e.State.TurnOrder[1];
            var lastRank = e.State.Hands[second][0].Rank;
            var rank = honest ? lastRank : (CardRank)((int)lastRank % 13 + 1);
            Send(e, CommandKind.Play, 2, rank);
            Send(e, CommandKind.Play, 1);
            return e;
        }
        internal static T RoundTrip<T>(T message) where T : class, IProtocolMessage
        {
            var result = ProtocolCodec.Decode(ProtocolCodec.Encode(message));
            Assert.That(result.Success, Is.True, result.Error);
            Assert.That(result.Error, Is.Null);
            Assert.That(result.Message, Is.TypeOf<T>());
            return (T)result.Message;
        }
        internal static HashSet<Card> ReachableCards(object root)
        {
            var cards = new HashSet<Card>();
            var seen = new HashSet<object>();
            void Walk(object item)
            {
                if (item == null) return;
                if (item is Card card) { cards.Add(card); return; }
                var type = item.GetType();
                if (type.IsPrimitive || type.IsEnum || item is string || item is Guid) return;
                if (!type.IsValueType && !seen.Add(item)) return;
                if (item is IEnumerable sequence)
                {
                    foreach (var value in sequence) Walk(value);
                    if (type.Assembly != typeof(PlayerView).Assembly &&
                        type.Assembly != typeof(ViewMessage).Assembly) return;
                }
                for (var t = type; t != null; t = t.BaseType)
                    foreach (var field in t.GetFields(BindingFlags.Instance | BindingFlags.Public |
                        BindingFlags.NonPublic | BindingFlags.DeclaredOnly)) Walk(field.GetValue(item));
            }
            Walk(root);
            return cards;
        }
        internal static ViewMessage[] CheckPrivacy(GameEngine e)
        {
            var result = new List<ViewMessage>();
            for (int p = 0; p < e.State.Hands.Count; p++)
            {
                var original = PlayerView.Create(e.State, p);
                var decoded = RoundTrip(new ViewMessage(original));
                SnapshotAssert.Equal(original, decoded.View);
                var allowed = new HashSet<Card>(e.State.Hands[p]);
                if (e.State.Phase == GamePhase.Revelation) allowed.UnionWith(e.State.Revelation.Cards);
                Assert.That(ReachableCards(decoded).IsSubsetOf(allowed), Is.True, "Private cards for player " + p);
                result.Add(decoded);
            }
            return result.ToArray();
        }
        internal static IEnumerable<IProtocolMessage> Messages()
        {
            yield return new HelloMessage(1, "Élise 🂡");
            yield return new HelloMessage(17, new string('é', 32)); // negotiation payload, wire version remains 1
            yield return new CommandMessage(CommandId, 1234567890123, CommandKind.Play,
                Deck.CreateStandard().Take(4), CardRank.King);
            yield return new CommandMessage(CommandId, 0, CommandKind.Doubt);
            yield return new CommandMessage(CommandId, 4, CommandKind.Discard);
            yield return new ReadyMessage(true); yield return new ReadyMessage(false);
            yield return new WelcomeMessage(1, 2, new Dictionary<int, string> { [2] = "B", [0] = "A" }, new[] { 2, 0 });
            foreach (CommandError error in Enum.GetValues(typeof(CommandError)))
                yield return new CommandRejectedMessage(CommandId, error);
            yield return new VersionMismatchMessage(12);
            foreach (int duration in new[] { 30, 45, 60, 90 })
                yield return new LobbyStateMessage(new[] { new LobbyPlayer(0, "Hôte", true),
                    new LobbyPlayer(5, "Client", false) }, duration, 6, 5, 0);
            yield return new JoinRefusedMessage(JoinRefusal.LobbyFull);
            yield return new JoinRefusedMessage(JoinRefusal.GameStarted);
            var e = BeforeDoubt();
            yield return new ViewMessage(PlayerView.Create(e.State, e.State.ActivePlayerId));
            Send(e, CommandKind.Doubt);
            yield return new ViewMessage(PlayerView.Create(e.State, e.State.ActivePlayerId));
        }
    }
}
