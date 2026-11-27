using System.Collections.Generic;
using System.Linq;
using Dubito.Core;

namespace Dubito.Presentation
{
    public sealed class CardSelection
    {
        readonly List<Card> cards = new List<Card>();
        public IReadOnlyList<Card> Cards => cards.AsReadOnly();
        public int Count => cards.Count;
        public bool Contains(Card card) => cards.Contains(card);
        public bool Toggle(Card card)
        {
            if (cards.Remove(card)) return true;
            if (cards.Count == 4) return false;
            cards.Add(card);
            return true;
        }
        public void Reset() => cards.Clear();
    }

    public sealed class HoldGauge
    {
        public const float Duration = .6f;
        public float Progress { get; private set; }
        bool fired;
        public bool Tick(float seconds, bool held, bool allowed)
        {
            if (!held || !allowed) { Reset(); return false; }
            if (fired) return false;
            Progress = System.Math.Min(1, Progress + System.Math.Max(0, seconds) / Duration);
            if (Progress < .99999f) return false;
            fired = true;
            return true;
        }
        public void Reset() { Progress = 0; fired = false; }
    }

    public sealed class ActionPanelModel
    {
        public bool Visible, CanPlay, CanDoubt, CanDiscard, ChooseRank, HighlightDoubt, UrgentDoubt;
        public CardRank Rank;
        public string Announcement, PlayReason, DoubtReason, DiscardReason;
        public static ActionPanelModel From(PlayerView view, int selectedCount, CardRank? chosenRank)
        {
            var m = new ActionPanelModel();
            m.Visible = view.Phase == GamePhase.Playing && view.ActivePlayerId == view.PlayerId;
            m.CanPlay = view.AllowedActions.Contains(CommandKind.Play) && selectedCount > 0 && selectedCount <= 4;
            m.CanDoubt = view.AllowedActions.Contains(CommandKind.Doubt);
            m.CanDiscard = view.AllowedActions.Contains(CommandKind.Discard);
            m.ChooseRank = !view.RoundRank.HasValue;
            m.Rank = view.RoundRank ?? chosenRank ?? CardRank.Ace;
            m.Announcement = $"{selectedCount} × {CardNames.Rank(m.Rank)}";
            string unavailable = view.Phase == GamePhase.Revelation ? "Révélation en cours" :
                view.Phase == GamePhase.Finished ? "Partie terminée" : "Ce n'est pas ton tour";
            m.PlayReason = m.CanPlay ? "" : !m.Visible ? unavailable : selectedCount == 0 || selectedCount > 4 ? "Sélectionne 1 à 4 cartes" : "Action indisponible";
            m.DoubtReason = m.CanDoubt ? "" : !m.Visible ? unavailable : view.PileCount == 0 ? "Rien à douter : le tas est vide" : "Ce coup n'est plus doutable";
            m.DiscardReason = m.CanDiscard ? "" : !m.Visible ? unavailable : view.PendingWinnerId.HasValue ?
                $"Interdit : {CardNames.PlayerName(view.PendingWinnerId.Value)} attend sa victoire" : view.PileCount == 0 ? "Le tas est vide" : "Action indisponible";
            m.HighlightDoubt = m.CanDoubt;
            m.UrgentDoubt = m.CanDoubt && view.PendingWinnerId.HasValue;
            return m;
        }
    }
}
