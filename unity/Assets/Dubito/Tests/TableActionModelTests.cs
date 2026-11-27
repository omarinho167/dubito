using System.Linq;
using Dubito.Core;
using Dubito.Networking;
using Dubito.Presentation;
using NUnit.Framework;

namespace Dubito.Tests
{
    public sealed class TableActionModelTests
    {
        static LocalGameSession Create() => new LocalGameSession(new GameConfig(3), new SystemRandomSource(2303));

        [Test]
        public void SelectionTogglesRejectsFifthAndResets()
        {
            var hand = Create().CurrentView.Hand;
            var selection = new CardSelection();
            Assert.That(selection.Toggle(hand[0]), Is.True);
            Assert.That(selection.Contains(hand[0]), Is.True);
            Assert.That(selection.Toggle(hand[0]), Is.True);
            Assert.That(selection.Count, Is.Zero);
            foreach (var card in hand.Take(4)) Assert.That(selection.Toggle(card), Is.True);
            Assert.That(selection.Toggle(hand[4]), Is.False);
            Assert.That(selection.Count, Is.EqualTo(4));
            Assert.That(selection.Contains(hand[4]), Is.False);
            selection.Reset();
            Assert.That(selection.Count, Is.Zero);
        }

        [Test]
        public void OpeningChoosesRankAndNeedsSelection()
        {
            var view = Create().CurrentView;
            var model = ActionPanelModel.From(view, 2, CardRank.Queen);
            Assert.That(model.Visible, Is.True);
            Assert.That(model.ChooseRank, Is.True);
            Assert.That(model.Announcement, Is.EqualTo("2 × Dame"));
            Assert.That(model.CanPlay, Is.True);
            Assert.That(model.CanDoubt, Is.False);
            Assert.That(model.DoubtReason, Is.EqualTo("Rien à douter : le tas est vide"));
            Assert.That(model.CanDiscard, Is.False);
            model = ActionPanelModel.From(view, 0, null);
            Assert.That(model.CanPlay, Is.False);
            Assert.That(model.PlayReason, Is.EqualTo("Sélectionne 1 à 4 cartes"));
        }

        [Test]
        public void RoundUsesFixedRankAndAuthoritativeActions()
        {
            var session = Create();
            session.Play(session.CurrentView.Hand.Take(2).ToArray(), CardRank.Queen);
            var inactive = ActionPanelModel.From(session.CurrentView, 1, CardRank.King);
            Assert.That(inactive.Visible, Is.False);
            Assert.That(inactive.CanPlay || inactive.CanDoubt || inactive.CanDiscard, Is.False);
            session.SetControlledPlayer(session.CurrentView.ActivePlayerId);
            var view = session.CurrentView;
            var model = ActionPanelModel.From(view, 2, CardRank.King);
            Assert.That(model.ChooseRank, Is.False);
            Assert.That(model.Announcement, Is.EqualTo("2 × Dame"));
            AssertActions(view, model);
            Assert.That(model.HighlightDoubt, Is.True);
            session.Doubt();
            model = ActionPanelModel.From(session.CurrentView, 2, null);
            Assert.That(model.Visible, Is.False);
            AssertActions(session.CurrentView, model);
        }

        [Test]
        public void PendingVictoryHighlightsDoubtAndExplainsDiscard()
        {
            var session = Create();
            // Reach a genuine pending victory using only legal plays and discards.
            int opener = session.ControlledPlayerId;
            for (int steps = 0; steps < 100 && !session.CurrentView.PendingWinnerId.HasValue; steps++)
            {
                session.SetControlledPlayer(session.CurrentView.ActivePlayerId);
                if (session.ControlledPlayerId == opener || session.CurrentView.PileCount == 0)
                    session.Play(session.CurrentView.Hand.Take(4).ToArray(), session.CurrentView.RoundRank ?? CardRank.Ace);
                else session.Discard();
            }
            Assert.That(session.CurrentView.PendingWinnerId, Is.EqualTo(opener));
            session.SetControlledPlayer(session.CurrentView.ActivePlayerId);
            var model = ActionPanelModel.From(session.CurrentView, 1, null);
            Assert.That(model.HighlightDoubt && model.UrgentDoubt, Is.True);
            Assert.That(model.CanDiscard, Is.False);
            Assert.That(model.DiscardReason, Is.EqualTo($"Interdit : {CardNames.PlayerName(opener)} attend sa victoire"));
            AssertActions(session.CurrentView, model);
        }

        static void AssertActions(PlayerView view, ActionPanelModel model)
        {
            Assert.That(model.CanPlay, Is.EqualTo(view.AllowedActions.Contains(CommandKind.Play)));
            Assert.That(model.CanDoubt, Is.EqualTo(view.AllowedActions.Contains(CommandKind.Doubt)));
            Assert.That(model.CanDiscard, Is.EqualTo(view.AllowedActions.Contains(CommandKind.Discard)));
        }

        [Test]
        public void HoldRequiresContinuousDurationAndFiresOnce()
        {
            var gauge = new HoldGauge();
            Assert.That(gauge.Tick(.3f, true, true), Is.False);
            Assert.That(gauge.Progress, Is.EqualTo(.5f).Within(.001));
            Assert.That(gauge.Tick(.3f, true, true), Is.True);
            Assert.That(gauge.Tick(1, true, true), Is.False);
            gauge.Tick(0, false, true);
            Assert.That(gauge.Progress, Is.Zero);
            gauge.Tick(.4f, true, true);
            gauge.Tick(.3f, true, false);
            Assert.That(gauge.Progress, Is.Zero);
            Assert.That(gauge.Tick(.3f, true, true), Is.False);
            gauge.Reset();
            Assert.That(gauge.Progress, Is.Zero);
        }
    }
}
