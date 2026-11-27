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

        // Who picks up the pile, as computed by Core (departures and confirmed victory included).
        public static int? PickerId(RevelationView revelation) => revelation.PileRecipientId;

        public static string Outcome(RevelationView revelation, int pileCount, int? pendingWinnerId)
        {
            string accused = CardNames.PlayerName(revelation.AccusedPlayerId);
            if (revelation.IsAutomatic)
                return revelation.IsDoubtCorrect
                    ? $"{accused} avait menti : sa victoire est annulée, le tas est retiré"
                    : $"{accused} avait dit vrai : victoire confirmée";
            string pile = CardNames.CardCount(pileCount);
            string doubter = CardNames.PlayerName(revelation.DoubterId);
            if (revelation.IsDoubtCorrect)
                return revelation.PileRecipientId.HasValue
                    ? $"{accused} a menti et ramasse le tas ({pile})"
                    : $"{accused} a menti et est parti : le tas est retiré";
            if (revelation.PileRecipientId.HasValue) return $"{accused} a dit vrai : {doubter} ramasse le tas ({pile})";
            return pendingWinnerId != revelation.AccusedPlayerId
                ? $"{accused} a dit vrai et {doubter} est parti : le tas est retiré"
                : $"{accused} a dit vrai : victoire confirmée";
        }
    }

    // Banner and end-of-game wording (UX §5).
    public static class TableMessageText
    {
        public static string PendingVictory(PlayerView view)
        {
            if (!view.PendingWinnerId.HasValue || view.Phase != GamePhase.Playing) return "";
            string winner = CardNames.PlayerName(view.PendingWinnerId.Value);
            string next = PlayerVoice.Addresses(view, view.ActivePlayerId) ? "Tu peux douter…" : $"{CardNames.PlayerName(view.ActivePlayerId)} peut douter…";
            return $"{winner} n'a plus de cartes ! {next}";
        }

        // UX §5: after a first skipped turn, warn the player concerned when it is their turn again.
        public static string ExclusionWarning(PlayerView view) =>
            view.Phase == GamePhase.Playing && view.ActivePlayerId == view.PlayerId && view.OwnSkippedTurns == 1
                ? (PlayerVoice.HotSeat ? $"Encore un tour sauté et {CardNames.PlayerName(view.PlayerId)} sera exclu" : "Encore un tour sauté et tu seras exclu") : "";

        // What happened when a timer tick changed the game, comparing the view before and after.
        public static string AfterTimeout(PlayerView before, PlayerView after)
        {
            if (before == null || before.Phase != GamePhase.Playing) return "";
            // Only time passes in a tick: a new revision while playing is always a timeout (RULES §5),
            // even when it confirms a pending victory and ends the game.
            int skipped = before.ActivePlayerId;
            string name = CardNames.PlayerName(skipped);
            if (!Contains(after.TurnOrder, skipped))
                return $"Temps écoulé : {name} est exclu (2 tours sautés)";
            string message = $"Temps écoulé : tour de {name} sauté";
            // UX §5: warn at the first skipped turn, by name (the seat shown may change right after).
            if (after.PlayerId == skipped && after.OwnSkippedTurns == 1 && after.Phase != GamePhase.Finished)
                message += $". Encore un et {name} sera exclu";
            return message;
        }

        // Online (step 3.5): events announced by the host. `me` is the reader's player id.
        public static string ForEvent(Dubito.Protocol.TableEventKind kind, int playerId, int me)
        {
            bool mine = playerId == me && !PlayerVoice.HotSeat;
            string name = CardNames.PlayerName(playerId);
            switch (kind)
            {
                // A skipped turn that does not exclude is always the first one in a row (RULES §5).
                case Dubito.Protocol.TableEventKind.TurnSkipped:
                    return mine ? "Temps écoulé : ton tour est sauté. Encore un et tu seras exclu"
                        : $"Temps écoulé : tour de {name} sauté";
                case Dubito.Protocol.TableEventKind.Excluded:
                    return mine ? "Temps écoulé : tu es exclu (2 tours sautés)" : $"Temps écoulé : {name} est exclu (2 tours sautés)";
                case Dubito.Protocol.TableEventKind.Left:
                    return $"{name} a quitté la partie";
                default: return "";
            }
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
