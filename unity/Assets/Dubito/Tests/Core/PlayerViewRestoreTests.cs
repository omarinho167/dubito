using System;
using System.Collections;
using System.Linq;
using Dubito.Core;
using NUnit.Framework;

namespace Dubito.Tests.Core
{
    internal static class SnapshotAssert
    {
        // Recursively compare every public field/property, including future additions.
        internal static void Equal(object expected, object actual, string path = "snapshot")
        {
            if (expected == null) { Assert.That(actual, Is.Null, path); return; }
            Assert.That(actual, Is.Not.Null, path);
            var type = expected.GetType();
            Assert.That(actual.GetType(), Is.EqualTo(type), path);
            if (type.IsPrimitive || type.IsEnum || expected is string || expected is Guid || expected is Card)
            { Assert.That(actual, Is.EqualTo(expected), path); return; }
            if (expected is IEnumerable items)
            {
                var a = items.Cast<object>().ToArray();
                var b = ((IEnumerable)actual).Cast<object>().ToArray();
                Assert.That(b.Length, Is.EqualTo(a.Length), path);
                for (int i = 0; i < a.Length; i++) Equal(a[i], b[i], path + "[" + i + "]");
                return;
            }
            foreach (var property in type.GetProperties())
                if (property.GetIndexParameters().Length == 0)
                    Equal(property.GetValue(expected), property.GetValue(actual), path + "." + property.Name);
            foreach (var field in type.GetFields())
                Equal(field.GetValue(expected), field.GetValue(actual), path + "." + field.Name);
        }
    }

    public class PlayerViewRestoreTests
    {
        internal static PlayerView Copy(PlayerView v)
        {
            var play = v.LastAnnouncement == null ? null : PlayAnnouncement.Restore(
                v.LastAnnouncement.PlayerId, v.LastAnnouncement.CardCount, v.LastAnnouncement.AnnouncedRank);
            var r = v.Revelation;
            var reveal = r == null ? null : RevelationView.Restore(r.AccusedPlayerId, r.DoubterId,
                r.IsAutomatic, r.Cards, r.IsDoubtCorrect, r.PileRecipientId);
            return PlayerView.Restore(v.PlayerId, v.Hand, v.CardCounts, v.TurnOrder,
                v.PileCount, v.RoundRank, play, v.ActivePlayerId, v.RemainingTurnSeconds,
                v.IsTimerPaused, v.Phase, reveal, v.PendingWinnerId, v.WinnerId,
                v.OwnSkippedTurns, v.Revision, v.AllowedActions);
        }

        [TestCase(2)] [TestCase(3)] [TestCase(6)]
        public void CreateFieldsRestorePreservesEveryValue(int players)
        {
            var e = new GameEngine(GameSetup.Create(new GameConfig(players), new SystemRandomSource(621)));
            void Check()
            {
                for (int p = 0; p < players; p++)
                {
                    var v = PlayerView.Create(e.State, p);
                    var copy = Copy(v);
                    SnapshotAssert.Equal(v, copy);
                    Assert.That(copy.Hand, Is.Not.SameAs(v.Hand));
                    Assert.That(copy.CardCounts, Is.Not.SameAs(v.CardCounts));
                    Assert.That(copy.TurnOrder, Is.Not.SameAs(v.TurnOrder));
                    Assert.That(copy.AllowedActions, Is.Not.SameAs(v.AllowedActions));
                    if (v.Revelation != null)
                        Assert.That(copy.Revelation.Cards, Is.Not.SameAs(v.Revelation.Cards));
                }
            }
            Check();
            Assert.That(e.AdvanceTime(45).Error, Is.EqualTo(CommandError.None)); Check();
            int active = e.State.ActivePlayerId;
            Assert.That(e.Execute(active, new GameCommand(Guid.NewGuid(), e.State.Revision,
                CommandKind.Play, e.State.Hands[active].Take(2), CardRank.Ace)).Error, Is.EqualTo(CommandError.None)); Check();
            Assert.That(e.Execute(e.State.ActivePlayerId, new GameCommand(Guid.NewGuid(), e.State.Revision,
                CommandKind.Doubt)).Error, Is.EqualTo(CommandError.None)); Check();
            Assert.That(e.ResolveRevelation(e.State.Revision).Error, Is.EqualTo(CommandError.None)); Check();
            Assert.That(e.StopGame().Error, Is.EqualTo(CommandError.None)); Check();
        }

        [Test]
        public void RestoreCopiesInputsAndDoesNotRecomputeActionsOrVerdict()
        {
            var hand = new[] { new Card(CardRank.King, CardSuit.Hearts) };
            var counts = new[] { 1, 0 };
            var order = new[] { 1, 0 };
            var actions = new[] { CommandKind.Discard };
            var reveal = RevelationView.Restore(0, 1, true, hand, true, 1);
            var v = PlayerView.Restore(0, hand, counts, order, 12, CardRank.Two,
                PlayAnnouncement.Restore(1, 3, CardRank.Queen), 1, 12.5, false,
                GamePhase.Finished, reveal, 0, 1, 2, 456, actions);
            var before = Copy(v);
            hand[0] = new Card(CardRank.Ace, CardSuit.Clubs);
            counts[0] = 50; order[0] = 0; actions[0] = CommandKind.Play;
            SnapshotAssert.Equal(before, v);
            Assert.That(v.AllowedActions, Is.EqualTo(new[] { CommandKind.Discard }));
            Assert.That(v.Revelation.IsDoubtCorrect, Is.True);
            Assert.Throws<NotSupportedException>(() => ((System.Collections.Generic.IList<Card>)v.Hand).Clear());
            Assert.Throws<NotSupportedException>(() => ((System.Collections.Generic.IList<Card>)reveal.Cards).Clear());
        }
    }
}
