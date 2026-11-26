using System;

namespace Dubito.Core
{
    public enum CardRank
    {
        Ace = 1, Two, Three, Four, Five, Six, Seven, Eight, Nine, Ten, Jack, Queen, King
    }

    // Suit identifies a physical card and supports display; rules use Rank only.
    public enum CardSuit
    {
        Clubs, Diamonds, Hearts, Spades
    }

    public readonly struct Card : IEquatable<Card>
    {
        public CardRank Rank { get; }
        public CardSuit Suit { get; }

        public Card(CardRank rank, CardSuit suit)
        {
            if (rank < CardRank.Ace || rank > CardRank.King)
                throw new ArgumentOutOfRangeException(nameof(rank));
            if (suit < CardSuit.Clubs || suit > CardSuit.Spades)
                throw new ArgumentOutOfRangeException(nameof(suit));
            Rank = rank;
            Suit = suit;
        }

        public bool Equals(Card other) => Rank == other.Rank && Suit == other.Suit;
        public override bool Equals(object obj) => obj is Card other && Equals(other);
        public override int GetHashCode() => ((int)Rank * 4) + (int)Suit;
        public static bool operator ==(Card left, Card right) => left.Equals(right);
        public static bool operator !=(Card left, Card right) => !left.Equals(right);
        public override string ToString() => $"{Rank} of {Suit}";
    }
}
