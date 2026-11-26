using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using Dubito.Core;
using NUnit.Framework;

namespace Dubito.Tests.Core
{
    public partial class ActionTests
    {
        // Stage 1.6 tests the actual view data; real serialized network messages
        // must additionally be checked at ROADMAP 3.3 (C-PRV-01 through 06).
        private static HashSet<Card> ReachableCards(object root, HashSet<object> references = null)
        {
            var cards = new HashSet<Card>();
            var visited = new HashSet<object>(ReferenceComparer.Instance);
            void Walk(object value, int depth = 0)
            {
                if (value == null) return;
                Assert.That(depth, Is.LessThan(100), value.GetType().FullName);
                if (value is Card card) { cards.Add(card); return; }
                var type = value.GetType();
                if (type.IsPrimitive || type.IsEnum || value is string || value is decimal) return;
                if (!type.IsValueType && !visited.Add(value)) return;
                if (!type.IsValueType) references?.Add(value);
                if (value is IEnumerable items)
                {
                    foreach (var item in items) Walk(item, depth + 1);
                    // Inspect framework collections through their logical contents;
                    // array runtime internals contain native pointers, not view data.
                    if (type.Assembly != typeof(PlayerView).Assembly) return;
                }
                // Include private backing fields: a hidden host reference is a leak too.
                for (var t = type; t != null; t = t.BaseType)
                {
                    const BindingFlags flags = BindingFlags.Instance | BindingFlags.Public |
                        BindingFlags.NonPublic | BindingFlags.DeclaredOnly;
                    foreach (var field in t.GetFields(flags)) Walk(field.GetValue(value), depth + 1);
                    foreach (var property in t.GetProperties(flags))
                        if (property.GetIndexParameters().Length == 0 && property.GetGetMethod(true) != null)
                            Walk(property.GetValue(value, null), depth + 1);
                }
            }
            Walk(root);
            return cards;
        }

        private sealed class ReferenceComparer : IEqualityComparer<object>
        {
            internal static readonly ReferenceComparer Instance = new ReferenceComparer();
            public new bool Equals(object a, object b) => ReferenceEquals(a, b);
            public int GetHashCode(object value) => System.Runtime.CompilerServices.RuntimeHelpers.GetHashCode(value);
        }

        private static void CheckViews(GameState s)
        {
            Invariants(s);
            for (int p = 0; p < s.Hands.Count; p++)
            {
                var view = PlayerView.Create(s, p);
                var allowed = new HashSet<Card>(s.Hands[p]);
                if (s.Phase == GamePhase.Revelation) allowed.UnionWith(s.Revelation.Cards);
                Assert.That(ReachableCards(view).IsSubsetOf(allowed), Is.True, "Private card leak");
                Assert.That(view.Hand, Is.EqualTo(s.Hands[p]));
                Assert.That(view.CardCounts, Is.EqualTo(s.Hands.Select(h => h.Count)));
                Assert.That(view.TurnOrder, Is.EqualTo(s.TurnOrder));
                Assert.That(view.PileCount, Is.EqualTo(s.Pile.Count));
                Assert.That(view.RoundRank, Is.EqualTo(s.RoundRank));
                Assert.That(view.ActivePlayerId, Is.EqualTo(s.ActivePlayerId));
                Assert.That(view.RemainingTurnSeconds, Is.EqualTo(s.RemainingTurnSeconds));
                Assert.That(view.IsTimerPaused, Is.EqualTo(s.IsTimerPaused));
                Assert.That(view.Phase, Is.EqualTo(s.Phase));
                Assert.That(view.PendingWinnerId, Is.EqualTo(s.PendingWinnerId));
                Assert.That(view.WinnerId, Is.EqualTo(s.WinnerId));
                Assert.That(view.Revision, Is.EqualTo(s.Revision));
                foreach (CommandKind kind in Enum.GetValues(typeof(CommandKind)))
                {
                    // Probe independently from the identical host snapshot, with a valid
                    // payload and fresh ID/revision; no action may affect the next probe.
                    var probe = new GameEngine(s);
                    var result = probe.Execute(p, new GameCommand(Guid.NewGuid(), s.Revision,
                        kind, s.Hands[p].Take(1), s.RoundRank ?? CardRank.Ace));
                    Assert.That(view.AllowedActions.Contains(kind),
                        Is.EqualTo(result.Status == CommandStatus.Applied), $"Player {p}, {kind}, {s.Phase}");
                }
            }
        }

        [TestCase(TestName = "C-PRV-01")]
        public void ViewsContainOnlyOwnCards()
        {
            var e = new GameEngine(GameSetup.Create(new GameConfig(6), new SystemRandomSource(91)));
            CheckViews(e.State);
            for (int p = 0; p < 6; p++)
                Assert.That(ReachableCards(PlayerView.Create(e.State, p)), Is.EquivalentTo(e.State.Hands[p]));
        }

        [TestCase(TestName = "C-PRV-02")]
        public void AnnouncementContainsNoPlayedCards()
        {
            var e = Open();
            for (int p = 0; p < 3; p++)
            {
                var v = PlayerView.Create(e.State, p);
                Assert.That(v.LastAnnouncement.PlayerId, Is.EqualTo(0));
                Assert.That(v.LastAnnouncement.CardCount, Is.EqualTo(2));
                Assert.That(v.LastAnnouncement.AnnouncedRank, Is.EqualTo(CardRank.Seven));
                Assert.That(ReachableCards(v.LastAnnouncement), Is.Empty);
            }
            CheckViews(e.State);
        }

        [TestCase(TestName = "C-PRV-03")]
        public void RevelationSendsOnlyLastPlayToEveryone()
        {
            var e = Open(); Play(e, 1, null, C(CardRank.Four)); Doubt(e, 2);
            for (int p = 0; p < 3; p++)
            {
                var v = PlayerView.Create(e.State, p);
                Assert.That(v.Revelation.Cards, Is.EqualTo(new[] { C(CardRank.Four) }));
                Assert.That(v.Revelation.IsDoubtCorrect, Is.True);
                Assert.That(v.Revelation.AccusedPlayerId, Is.EqualTo(1));
                Assert.That(v.Revelation.DoubterId, Is.EqualTo(2));
                Assert.That(v.Revelation.IsAutomatic, Is.False);
                Assert.That(v.AllowedActions, Is.Empty);
            }
            CheckViews(e.State);
        }

        [TestCase(TestName = "C-PRV-04")]
        public void PickupValuesGoOnlyToRecipient()
        {
            foreach (bool honest in new[] { false, true })
            {
                var e = Open(honest); var pile = e.State.Pile.ToArray();
                Doubt(e, 1); Resolve(e);
                int recipient = honest ? 1 : 0;
                for (int p = 0; p < 3; p++)
                {
                    var v = PlayerView.Create(e.State, p);
                    Assert.That(v.Revelation, Is.Null);
                    Assert.That(pile.All(c => ReachableCards(v).Contains(c)), Is.EqualTo(p == recipient));
                }
                CheckViews(e.State);
            }
        }

        [TestCase(TestName = "C-PRV-05")]
        public void RemovalRevealsNoCards()
        {
            var e = Open(); Discard(e, 1); CheckViews(e.State);
            Leave(e, 0); CheckViews(e.State);
            for (int p = 0; p < 3; p++)
                Assert.That(ReachableCards(PlayerView.Create(e.State, p)).Overlaps(e.State.RemovedCards), Is.False);
        }

        [TestCase(TestName = "C-PRV-06")]
        public void DepartedPendingWinnerRevealsOnlyTwoOfNineCards()
        {
            var e = Game(4,
                new[] { C(CardRank.Ace), C(CardRank.Two), C(CardRank.Three), C(CardRank.Four), C(CardRank.Five), C(CardRank.Queen) },
                new[] { C(CardRank.Six), C(CardRank.Seven) },
                new[] { C(CardRank.Eight), C(CardRank.Nine), C(CardRank.Ten), C(CardRank.Jack) });
            Play(e, 0, CardRank.King, e.State.Hands[0].Take(4).ToArray());
            Tick(e, 45); // J2 will empty its hand on the next cycle.
            Play(e, 2, null, e.State.Hands[2].Take(2).ToArray());
            Tick(e, 45); Play(e, 0, null, e.State.Hands[0].Take(1).ToArray());
            var last = e.State.Hands[1].ToArray(); Play(e, 1, null, last);
            Assert.That(e.State.Pile.Count, Is.EqualTo(9)); Leave(e, 1);
            for (int p = 0; p < 4; p++)
            {
                var v = PlayerView.Create(e.State, p);
                Assert.That(v.Revelation.Cards, Is.EqualTo(last));
                Assert.That(v.Revelation.IsAutomatic, Is.True);
                Assert.That(v.PendingWinnerId, Is.EqualTo(1));
            }
            CheckViews(e.State);
        }

        [Test]
        public void RandomGamesPreserveViewPrivacyAndActionAgreement()
        {
            var random = new Random(160923);
            var seen = new HashSet<CommandKind>();
            int revelations = 0, departures = 0, timeouts = 0, steps = 0;
            for (int game = 0; game < 100; game++)
            {
                var e = new GameEngine(GameSetup.Create(new GameConfig(2 + game % 5),
                    new SystemRandomSource(random.Next())));
                for (int step = 0; step < 150 && e.State.Phase != GamePhase.Finished; step++)
                {
                    CheckViews(e.State); steps++;
                    var s = e.State;
                    int roll = random.Next(100);
                    if (roll < 3) { Assert.That(e.StopGame().Error, Is.EqualTo(CommandError.None)); }
                    else if (roll < 10 && s.TurnOrder.Any(p => !s.PendingDepartures.Contains(p)))
                    {
                        var seats = s.TurnOrder.Where(p => !s.PendingDepartures.Contains(p)).ToArray();
                        Leave(e, seats[random.Next(seats.Length)]); departures++;
                    }
                    else if (s.Phase == GamePhase.Revelation) { Resolve(e); revelations++; }
                    else if (roll < 30) { Tick(e, random.Next(1, 100)); timeouts++; }
                    else
                    {
                        int p = s.ActivePlayerId;
                        var actions = PlayerView.Create(s, p).AllowedActions;
                        var kind = actions[random.Next(actions.Count)]; seen.Add(kind);
                        var cards = s.Hands[p].OrderBy(_ => random.Next()).Take(random.Next(1, Math.Min(4, s.Hands[p].Count) + 1));
                        Send(e, p, new GameCommand(Guid.NewGuid(), s.Revision, kind, cards,
                            s.RoundRank ?? (CardRank)random.Next(1, 14)));
                    }
                }
                CheckViews(e.State);
            }
            Assert.That(seen.Count, Is.EqualTo(3));
            Assert.That(revelations, Is.GreaterThan(20));
            Assert.That(departures, Is.GreaterThan(20));
            Assert.That(timeouts, Is.GreaterThan(20));
            Assert.That(steps, Is.GreaterThan(1000));
        }

        [Test]
        public void ViewsCoverActionMatrixAndTerminalOutcomes()
        {
            var e = Open(); CheckViews(e.State); // all three actions
            Tick(e, 45); CheckViews(e.State); // closed window, occupied pile
            Discard(e, 2); CheckViews(e.State); // opening
            foreach (bool honest in new[] { false, true })
            {
                e = Pending(honest); CheckViews(e.State); // no discard
                Doubt(e, 2); CheckViews(e.State); // no actions, public verdict
                for (int p = 0; p < 3; p++)
                    Assert.That(PlayerView.Create(e.State, p).Revelation.IsDoubtCorrect, Is.EqualTo(!honest));
                Resolve(e); CheckViews(e.State); // continuation or winner
                e = Pending(honest); Leave(e, 1); CheckViews(e.State);
                Resolve(e); CheckViews(e.State); // automatic revelation outcome
            }
            e = Open(); Doubt(e, 1); e.StopGame(); CheckViews(e.State);
            Assert.That(PlayerView.Create(e.State, 0).Revelation, Is.Null);
            Assert.That(PlayerView.Create(e.State, 0).WinnerId, Is.Null);
        }

        [Test]
        public void ViewCopiesAreImmutableAndDetached()
        {
            var e = Open(); Doubt(e, 1); var s = e.State;
            var v = PlayerView.Create(s, 1);
            var hostReferences = new HashSet<object>(ReferenceComparer.Instance);
            var viewReferences = new HashSet<object>(ReferenceComparer.Instance);
            ReachableCards(s, hostReferences); ReachableCards(v, viewReferences);
            Assert.That(viewReferences.Overlaps(hostReferences), Is.False, "Shared host reference");
            var hand = v.Hand.ToArray(); var reveal = v.Revelation.Cards.ToArray();
            Assert.That(v.Hand, Is.Not.SameAs(s.Hands[1]));
            Assert.That(v.TurnOrder, Is.Not.SameAs(s.TurnOrder));
            Assert.That(v.Revelation, Is.Not.SameAs(s.Revelation));
            Assert.That(v.Revelation.Cards, Is.Not.SameAs(s.Revelation.Cards));
            Assert.Throws<NotSupportedException>(() => ((IList<Card>)v.Hand).Clear());
            Assert.Throws<NotSupportedException>(() => ((IList<Card>)v.Revelation.Cards).Clear());
            Assert.Throws<NotSupportedException>(() => ((IList<int>)v.TurnOrder).Clear());
            Assert.Throws<NotSupportedException>(() => ((IList<int>)v.CardCounts).Clear());
            Assert.Throws<NotSupportedException>(() => ((IList<CommandKind>)v.AllowedActions).Add(CommandKind.Play));
            s.MutableHands[1].Clear(); s.MutableTurnOrder.Clear(); s.RemainingTurnSeconds = 1;
            Assert.That(v.Hand, Is.EqualTo(hand)); Assert.That(v.TurnOrder.Count, Is.EqualTo(3));
            Assert.That(v.RemainingTurnSeconds, Is.EqualTo(45));
            Assert.That(v.Revelation.Cards, Is.EqualTo(reveal));
            // Even internal host-side changes cannot alter the announcement snapshot.
            s.LastPlay = null; s.Revelation = null;
            Assert.That(v.LastAnnouncement.CardCount, Is.EqualTo(2));
            Assert.That(v.Revelation.Cards, Is.EqualTo(reveal));
        }

        [Test]
        public void ViewRejectsUnknownIdentityAndNullState()
        {
            var s = Open().State;
            Assert.Throws<ArgumentNullException>(() => PlayerView.Create(null, 0));
            Assert.Throws<ArgumentOutOfRangeException>(() => PlayerView.Create(s, -1));
            Assert.Throws<ArgumentOutOfRangeException>(() => PlayerView.Create(s, 3));
        }
    }
}
