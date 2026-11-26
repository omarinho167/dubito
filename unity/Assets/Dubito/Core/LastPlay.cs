using System.Collections.Generic;

namespace Dubito.Core
{
    public enum GamePhase { Playing = 0, Revelation = 1, Finished = 2 }

    // Host-only information. This is not a public/network view (RULES §9).
    public sealed class LastPlay
    {
        public int PlayerId { get; }
        public int RespondentId { get; }
        public CardRank AnnouncedRank { get; }
        public IReadOnlyList<Card> Cards { get; }
        public bool IsDoubtWindowOpen { get; }

        internal LastPlay(int playerId, int respondentId, CardRank rank,
            IEnumerable<Card> cards, bool isDoubtWindowOpen = true)
        {
            PlayerId = playerId;
            RespondentId = respondentId;
            AnnouncedRank = rank;
            Cards = new List<Card>(cards).AsReadOnly();
            IsDoubtWindowOpen = isDoubtWindowOpen;
        }

        // Future timeout/departure events can close a window without erasing the last play.
        internal LastPlay CloseDoubtWindow() =>
            new LastPlay(PlayerId, RespondentId, AnnouncedRank, Cards, false);
    }

    public sealed class DoubtRevelation
    {
        public int AccusedPlayerId { get; }
        public int DoubterId { get; }
        public IReadOnlyList<Card> Cards { get; }
        public bool IsDoubtCorrect { get; }

        internal DoubtRevelation(LastPlay play, int doubterId)
        {
            AccusedPlayerId = play.PlayerId;
            DoubterId = doubterId;
            Cards = play.Cards;
            foreach (var card in Cards)
                if (card.Rank != play.AnnouncedRank) IsDoubtCorrect = true;
        }
    }
}
