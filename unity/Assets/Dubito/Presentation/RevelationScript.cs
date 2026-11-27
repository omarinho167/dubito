using System.Collections.Generic;
using Dubito.Core;

namespace Dubito.Presentation
{
    // Pure timing and wording of the doubt revelation (UX §5). The verdict itself comes from Core
    // (RevelationView.IsDoubtCorrect); nothing here decides a rule.
    public static class RevelationScript
    {
        public const float ZoomIn = 0.4f;
        public const float FlipEach = 0.35f;
        public const float FlipTime = 0.3f;
        public const float VerdictHold = 1.3f;
        public const float Slide = 0.5f;

        // About 3 s for a usual 2-3 card play.
        public static float Duration(int cardCount) => ZoomIn + FlipEach * cardCount + VerdictHold + Slide;
        public static float FlipsEnd(int cardCount) => ZoomIn + FlipEach * cardCount;
        public static float SlideStart(int cardCount) => Duration(cardCount) - Slide;

        // "MENSONGE" when at least one revealed card is not the round rank (RULES §4.2).
        public static string Verdict(RevelationView revelation) => revelation.IsDoubtCorrect ? "MENSONGE" : "VÉRITÉ";

        // Who picks up the pile; null when nobody does (departure of a pending winner, RULES §7).
        public static int? PickerId(RevelationView revelation)
        {
            if (revelation.IsAutomatic) return null;
            return revelation.IsDoubtCorrect ? revelation.AccusedPlayerId : revelation.DoubterId;
        }

        public static string Outcome(RevelationView revelation, int pileCount)
        {
            string accused = CardNames.PlayerName(revelation.AccusedPlayerId);
            if (revelation.IsAutomatic)
                return revelation.IsDoubtCorrect
                    ? $"{accused} avait menti : sa victoire est annulée, le tas est retiré"
                    : $"{accused} avait dit vrai : victoire confirmée";
            string pile = CardNames.CardCount(pileCount);
            return revelation.IsDoubtCorrect
                ? $"{accused} a menti et ramasse le tas ({pile})"
                : $"{accused} a dit vrai : {CardNames.PlayerName(revelation.DoubterId)} ramasse le tas ({pile})";
        }
    }

    // Banner and end-of-game wording (UX §5).
    public static class TableMessageText
    {
        public static string PendingVictory(PlayerView view)
        {
            if (!view.PendingWinnerId.HasValue || view.Phase != GamePhase.Playing) return "";
            string winner = CardNames.PlayerName(view.PendingWinnerId.Value);
            string next = view.ActivePlayerId == view.PlayerId ? "Tu peux douter…" : $"{CardNames.PlayerName(view.ActivePlayerId)} peut douter…";
            return $"{winner} n'a plus de cartes ! {next}";
        }

        // UX §5: after a first skipped turn, warn the player concerned when it is their turn again.
        public static string ExclusionWarning(PlayerView view) =>
            view.Phase == GamePhase.Playing && view.ActivePlayerId == view.PlayerId && view.OwnSkippedTurns == 1
                ? "Encore un tour sauté et tu seras exclu" : "";

        // What happened when a timer tick changed the game, comparing the view before and after.
        public static string AfterTimeout(PlayerView before, PlayerView after)
        {
            if (before == null || before.Phase != GamePhase.Playing) return "";
            int skipped = before.ActivePlayerId;
            if (!Contains(after.TurnOrder, skipped))
                return $"Temps écoulé : {CardNames.PlayerName(skipped)} est exclu (2 tours sautés)";
            return after.ActivePlayerId != skipped ? $"Temps écoulé : tour de {CardNames.PlayerName(skipped)} sauté" : "";
        }

        public static string Winner(PlayerView view) =>
            view.Phase == GamePhase.Finished && view.WinnerId.HasValue
                ? $"{CardNames.PlayerName(view.WinnerId.Value)} gagne !" : view.Phase == GamePhase.Finished ? "Partie terminée" : "";

        static bool Contains(IReadOnlyList<int> list, int value)
        {
            for (int i = 0; i < list.Count; i++) if (list[i] == value) return true;
            return false;
        }
    }
}
