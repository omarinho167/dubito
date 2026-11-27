using System;
using System.Collections.Generic;
using Dubito.Core;

namespace Dubito.Presentation
{
    public static class CardNames
    {
        static readonly string[] Names = { "As", "2", "3", "4", "5", "6", "7", "8", "9", "10", "Valet", "Dame", "Roi" };
        public static string Rank(CardRank rank)
        {
            if (rank < CardRank.Ace || rank > CardRank.King) throw new ArgumentOutOfRangeException(nameof(rank));
            return Names[(int)rank - 1];
        }
        public static string RankPlural(CardRank rank, int count) =>
            Rank(rank) + (count > 1 && rank >= CardRank.Jack ? "s" : "");
        public static IReadOnlyDictionary<int, string> PlayerNames { get; set; }
        public static string PlayerName(int playerId) =>
            PlayerNames != null && PlayerNames.TryGetValue(playerId, out string name) ? name : $"Joueur {playerId + 1}";
        // French: 0 and 1 take the singular ("1 carte", "12 cartes").
        public static string CardCount(int count) => count <= 1 ? $"{count} carte" : $"{count} cartes";
    }
}
