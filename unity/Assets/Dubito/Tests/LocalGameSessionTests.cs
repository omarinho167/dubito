using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using Dubito.Core;
using Dubito.Networking;
using NUnit.Framework;

namespace Dubito.Tests
{
    public class LocalGameSessionTests
    {
        private const int Seed = 2202;
        private static LocalGameSession Create() =>
            new LocalGameSession(new GameConfig(3), new SystemRandomSource(Seed));

        [Test]
        public void InitialViewAndSeatsMatchSeededSetup()
        {
            var expected = GameSetup.Create(new GameConfig(3), new SystemRandomSource(Seed));
            IGameSession session = Create();
            Assert.That(session.ControlledPlayerId, Is.EqualTo(expected.TurnOrder[0]));
            Assert.That(session.SeatOrder, Is.EqualTo(expected.TurnOrder));
            Assert.That(session.CurrentView.PlayerId, Is.EqualTo(session.ControlledPlayerId));
            Assert.That(session.CurrentView.Hand, Is.EqualTo(expected.Hands[session.ControlledPlayerId]));
            Assert.Throws<NotSupportedException>(() => ((IList<int>)session.SeatOrder)[0] = 99);
        }

        [Test]
        public void ValidPlayPublishesControlledViewAfterUpdatingCurrentView()
        {
            var session = Create();
            var before = session.CurrentView;
            var views = new List<PlayerView>();
            session.ViewChanged += view =>
            {
                Assert.That(session.CurrentView, Is.SameAs(view));
                views.Add(view);
            };
            session.Play(new[] { before.Hand[0] }, CardRank.Ace);
            Assert.That(views.Count, Is.EqualTo(1));
            Assert.That(views[0].Revision, Is.EqualTo(before.Revision + 1));
            Assert.That(views[0].PlayerId, Is.EqualTo(before.PlayerId));
            Assert.That(views[0].Hand, Is.EqualTo(before.Hand.Skip(1)));
            Assert.That(views[0].PileCount, Is.EqualTo(1));
            Assert.That(before.PileCount, Is.Zero, "Previously published snapshots stay detached.");
        }

        [Test]
        public void InvalidPlayRejectsWithoutPublishingOrChangingView()
        {
            var session = Create();
            var before = session.CurrentView;
            int published = 0;
            var errors = new List<CommandError>();
            session.ViewChanged += _ => published++;
            session.CommandRejected += errors.Add;
            session.Play(Array.Empty<Card>(), CardRank.Ace);
            session.Play(Array.Empty<Card>(), CardRank.Ace);
            Assert.That(errors, Is.EqualTo(new[] { CommandError.InvalidCardCount, CommandError.InvalidCardCount }),
                "Every intent has a fresh command ID, including repeated rejected intents.");
            Assert.That(published, Is.Zero);
            Assert.That(session.CurrentView, Is.SameAs(before));
        }

        [Test]
        public void TickRefreshesCountdownWithoutRevisionEventUntilTurnExpires()
        {
            var session = Create();
            var before = session.CurrentView;
            int published = 0;
            session.ViewChanged += _ => published++;
            session.Tick(2.5);
            Assert.That(session.CurrentView.RemainingTurnSeconds, Is.EqualTo(42.5));
            Assert.That(session.CurrentView.Revision, Is.EqualTo(before.Revision));
            Assert.That(published, Is.Zero);
            session.Tick(42.5);
            Assert.That(published, Is.EqualTo(1));
            Assert.That(session.CurrentView.ActivePlayerId, Is.EqualTo(session.SeatOrder[1]));
            Assert.That(session.CurrentView.PlayerId, Is.EqualTo(session.ControlledPlayerId));
        }

        [Test]
        public void SwitchingSeatPublishesOnlyThatSeatsHandAtSameRevision()
        {
            var session = Create();
            var expected = GameSetup.Create(new GameConfig(3), new SystemRandomSource(Seed));
            var before = session.CurrentView;
            var views = new List<PlayerView>();
            session.ViewChanged += views.Add;
            int other = session.SeatOrder[1];
            session.SetControlledPlayer(other);
            Assert.That(session.ControlledPlayerId, Is.EqualTo(other));
            Assert.That(session.CurrentView.PlayerId, Is.EqualTo(other));
            Assert.That(session.CurrentView.Hand, Is.EqualTo(expected.Hands[other]));
            Assert.That(session.CurrentView.Hand.Intersect(before.Hand), Is.Empty);
            Assert.That(session.CurrentView.Revision, Is.EqualTo(before.Revision));
            Assert.That(views, Is.EqualTo(new[] { session.CurrentView }));
            session.SetControlledPlayer(other);
            Assert.That(views.Count, Is.EqualTo(1));
            Assert.Throws<ArgumentOutOfRangeException>(() => session.SetControlledPlayer(-1));
            Assert.Throws<ArgumentOutOfRangeException>(() => session.SetControlledPlayer(3));
            Assert.That(session.CurrentView, Is.SameAs(views[0]));
        }

        [Test]
        public void SeatsRemainFixedWhenControlledPlayerIsExcludedByTimeouts()
        {
            var session = Create();
            var seats = session.SeatOrder.ToArray();
            session.Tick(45 * 4); // First full ring, then the first seat's second timeout.
            Assert.That(session.SeatOrder, Is.EqualTo(seats));
            Assert.That(session.CurrentView.TurnOrder, Is.EqualTo(seats.Skip(1)));
            Assert.That(session.CurrentView.PlayerId, Is.EqualTo(seats[0]));
            Assert.That(session.CurrentView.Hand, Is.Empty);
            session.SetControlledPlayer(seats[1]);
            Assert.That(session.CurrentView.Hand.Count, Is.GreaterThan(0));
            Assert.That(session.SeatOrder, Is.EqualTo(seats));
        }

        [Test]
        public void DoubtAndResolvePublishRevelationThenResolution()
        {
            var session = Create();
            var card = session.CurrentView.Hand[0];
            var lie = card.Rank == CardRank.Ace ? CardRank.King : CardRank.Ace;
            session.Play(new[] { card }, lie);
            session.SetControlledPlayer(session.CurrentView.ActivePlayerId);
            int published = 0;
            session.ViewChanged += _ => published++;
            session.Doubt();
            Assert.That(session.CurrentView.Phase, Is.EqualTo(GamePhase.Revelation));
            Assert.That(session.CurrentView.Revelation.Cards, Is.EqualTo(new[] { card }));
            session.Tick(100);
            Assert.That(published, Is.EqualTo(1));
            session.ResolveRevelation();
            Assert.That(published, Is.EqualTo(2));
            Assert.That(session.CurrentView.Phase, Is.EqualTo(GamePhase.Playing));
            Assert.That(session.CurrentView.PileCount, Is.Zero);
            Assert.That(session.CurrentView.ActivePlayerId, Is.EqualTo(session.ControlledPlayerId));
        }

        [Test]
        public void DiscardUsesControlledSeatAndCurrentRevision()
        {
            var session = Create();
            session.Play(new[] { session.CurrentView.Hand[0] }, CardRank.Ace);
            var errors = new List<CommandError>();
            session.CommandRejected += errors.Add;
            session.Discard();
            Assert.That(errors, Is.EqualTo(new[] { CommandError.NotYourTurn }));
            session.SetControlledPlayer(session.CurrentView.ActivePlayerId);
            var before = session.CurrentView;
            session.Discard();
            Assert.That(session.CurrentView.Revision, Is.EqualTo(before.Revision + 1));
            Assert.That(session.CurrentView.PileCount, Is.Zero);
            Assert.That(session.CurrentView.Hand, Is.EqualTo(before.Hand));
            Assert.That(errors.Count, Is.EqualTo(1));
        }

        [Test]
        public void HostOperationErrorsAreReportedWithoutPublication()
        {
            var session = Create();
            var errors = new List<CommandError>();
            int published = 0;
            session.CommandRejected += errors.Add;
            session.ViewChanged += _ => published++;
            session.Tick(-1);
            session.ResolveRevelation();
            Assert.That(errors, Is.EqualTo(new[] { CommandError.InvalidElapsedTime, CommandError.NoRevelation }));
            Assert.That(published, Is.Zero);
        }

        [Test]
        public void PublicSessionSurfaceCannotRetrieveOtherHandsOrHostState()
        {
            foreach (var type in new[] { typeof(IGameSession), typeof(LocalGameSession) })
            {
                var properties = type.GetProperties();
                Assert.That(properties.Select(p => p.Name), Is.EquivalentTo(new[]
                    { "ControlledPlayerId", "SeatOrder", "CurrentView" }));
                Assert.That(properties.All(p => p.GetIndexParameters().Length == 0 && p.GetSetMethod() == null), Is.True);
                Assert.That(type.GetFields(BindingFlags.Public | BindingFlags.Instance | BindingFlags.Static), Is.Empty);
                var methods = type.GetMethods(BindingFlags.Public | BindingFlags.Instance | BindingFlags.DeclaredOnly)
                    .Where(m => !m.IsSpecialName).ToArray();
                Assert.That(methods.All(m => m.ReturnType == typeof(void)), Is.True);
                Assert.That(type.GetEvents().Select(e => e.EventHandlerType), Is.EquivalentTo(new[]
                    { typeof(Action<PlayerView>), typeof(Action<CommandError>) }));
            }
            Assert.That(typeof(LocalGameSession).Assembly.GetReferencedAssemblies()
                .Select(a => a.Name).Where(n => n.StartsWith("Unity") || n.StartsWith("Mirror")), Is.Empty);
        }
    }
}
