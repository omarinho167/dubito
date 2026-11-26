using System;
using System.Collections.Generic;

namespace Dubito.Core
{
    public static class Deck
    {
        public static IReadOnlyList<Card> CreateStandard()
        {
            var cards = new List<Card>(52);
            foreach (CardSuit suit in Enum.GetValues(typeof(CardSuit)))
            foreach (CardRank rank in Enum.GetValues(typeof(CardRank)))
                cards.Add(new Card(rank, suit));
            return cards.AsReadOnly();
        }
    }
}
