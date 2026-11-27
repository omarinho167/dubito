using System.Linq;
using Dubito.Core;
using Dubito.Networking;
using Dubito.Presentation;
using NUnit.Framework;

namespace Dubito.Tests
{
    // Step 2.5 wording and timing, on real games (no rule is re-implemented here).
    public sealed class RevelationScriptTests
    {
        static LocalGameSession NewSession() => new LocalGameSession(new GameConfig(3), new SystemRandomSource(7));

        // The controlled player opens with one card announced as `announced`, the next player doubts.
        static PlayerView PlayThenDoubt(LocalGameSession session, Card card, CardRank announced)
        {
            int accused = session.ControlledPlayerId;
            session.Play(new[] { card }, announced);
            int next = session.CurrentView.ActivePlayerId;
            Assert.That(next, Is.Not.EqualTo(accused));
            session.SetControlledPlayer(next);
            session.Doubt();
            Assert.That(session.CurrentView.Phase, Is.EqualTo(GamePhase.Revelation));
            return session.CurrentView;
        }

        [Test]
        public void LieIsMensongeAndTheLiarPicksUp()
        {
            var session = NewSession();
            int liar = session.ControlledPlayerId;
            var card = session.CurrentView.Hand[0];
            var announced = card.Rank == CardRank.King ? CardRank.Queen : CardRank.King;
            var view = PlayThenDoubt(session, card, announced);

            Assert.That(RevelationScript.Verdict(view.Revelation), Is.EqualTo("MENSONGE"));
            Assert.That(RevelationScript.PickerId(view.Revelation), Is.EqualTo(liar));
            Assert.That(RevelationScript.Outcome(view.Revelation, view.PileCount),
                Is.EqualTo($"{CardNames.PlayerName(liar)} a menti et ramasse le tas (1 carte)"));
        }

        [Test]
        public void TruthIsVeriteAndTheDoubterPicksUp()
        {
            var session = NewSession();
            var card = session.CurrentView.Hand[0];
            var view = PlayThenDoubt(session, card, card.Rank);

            Assert.That(RevelationScript.Verdict(view.Revelation), Is.EqualTo("VÉRITÉ"));
            Assert.That(RevelationScript.PickerId(view.Revelation), Is.EqualTo(view.Revelation.DoubterId));
            StringAssert.Contains("ramasse le tas", RevelationScript.Outcome(view.Revelation, view.PileCount));
        }

        [Test]
        public void RevelationLastsAboutThreeSeconds()
        {
            Assert.That(RevelationScript.Duration(2), Is.InRange(2.5f, 3.5f));
            Assert.That(RevelationScript.FlipsEnd(4), Is.LessThan(RevelationScript.SlideStart(4)));
        }

        [Test]
        public void TimeoutMessageNamesTheSkippedPlayer()
        {
            var session = NewSession();
            var before = session.CurrentView;
            session.Tick(45);
            var after = session.CurrentView;
            Assert.That(TableMessageText.AfterTimeout(before, after),
                Is.EqualTo($"Temps écoulé : tour de {CardNames.PlayerName(before.ActivePlayerId)} sauté"));
        }

        [Test]
        public void ExclusionWarningOnlyAfterOneSkippedTurnOnOwnTurn()
        {
            var session = NewSession();
            int first = session.ControlledPlayerId;
            Assert.That(TableMessageText.ExclusionWarning(session.CurrentView), Is.Empty);
            // Everyone times out once: the turn comes back to the first player with one skipped turn.
            for (int i = 0; i < 3; i++) session.Tick(45);
            var view = session.CurrentView;
            Assert.That(view.ActivePlayerId, Is.EqualTo(first));
            Assert.That(TableMessageText.ExclusionWarning(view), Is.EqualTo("Encore un tour sauté et tu seras exclu"));
        }

        [Test]
        public void NoBannerOrWinnerAtTheStart()
        {
            var view = NewSession().CurrentView;
            Assert.That(TableMessageText.PendingVictory(view), Is.Empty);
            Assert.That(TableMessageText.Winner(view), Is.Empty);
        }
    }
}
