using System;
using System.Collections.Generic;

namespace Dubito.Core
{
    // Authoritative host state, not a view to send to a client. Player IDs are stable
    // zero-based indices into Hands; TurnOrder contains these IDs in seating order.
    public sealed class GameState
    {
        public GameConfig Config { get; }
        public IReadOnlyList<int> TurnOrder { get; }
        public IReadOnlyList<IReadOnlyList<Card>> Hands { get; }
        public IReadOnlyList<Card> Pile { get; }
        public IReadOnlyList<Card> RemovedCards { get; }
        public CardRank? RoundRank { get; }
        public int ActivePlayerId { get; }
        public double RemainingTurnSeconds { get; }

        internal GameState(GameConfig config, List<int> turnOrder, List<List<Card>> hands)
        {
            Config = config;
            TurnOrder = new List<int>(turnOrder).AsReadOnly();
            var readOnlyHands = new List<IReadOnlyList<Card>>(hands.Count);
            foreach (var hand in hands)
                readOnlyHands.Add(new List<Card>(hand).AsReadOnly());
            Hands = readOnlyHands.AsReadOnly();
            Pile = Array.AsReadOnly(Array.Empty<Card>());
            RemovedCards = Array.AsReadOnly(Array.Empty<Card>());
            RoundRank = null;
            ActivePlayerId = TurnOrder[0];
            RemainingTurnSeconds = config.TurnDurationSeconds;
        }
    }
}
