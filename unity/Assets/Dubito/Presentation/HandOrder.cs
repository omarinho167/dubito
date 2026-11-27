using System;
using System.Collections.Generic;
using Dubito.Core;

namespace Dubito.Presentation
{
    public static class HandOrder
    {
        public static Card[] Sort(IReadOnlyList<Card> hand)
        {
            if (hand == null) throw new ArgumentNullException(nameof(hand));
            var result = new Card[hand.Count];
            CopySorted(hand, result);
            return result;
        }

        // Caller-owned buffer keeps frequent view refreshes allocation-free.
        public static void CopySorted(IReadOnlyList<Card> hand, Card[] destination)
        {
            if (hand == null) throw new ArgumentNullException(nameof(hand));
            if (destination == null || destination.Length < hand.Count) throw new ArgumentException("Buffer too small.", nameof(destination));
            for (int i = 0; i < hand.Count; i++)
            {
                Card card = hand[i];
                int j = i;
                while (j > 0 && Compare(destination[j - 1], card) > 0)
                { destination[j] = destination[j - 1]; j--; }
                destination[j] = card;
            }
        }
        static int Compare(Card a, Card b) => a.Rank != b.Rank ? a.Rank.CompareTo(b.Rank) : a.Suit.CompareTo(b.Suit);
    }
}
