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
        public CardRank? RoundRank { get; internal set; }
        public int ActivePlayerId { get; internal set; }
        public double RemainingTurnSeconds { get; internal set; }
        public long Revision { get; internal set; }
        public GamePhase Phase { get; internal set; }
        public LastPlay LastPlay { get; internal set; }
        public DoubtRevelation Revelation { get; internal set; }
        public int? PendingWinnerId { get; internal set; }
        public int? WinnerId { get; internal set; }
        public bool IsTimerPaused => Phase != GamePhase.Playing;

        internal readonly List<List<Card>> MutableHands;
        internal readonly List<Card> MutablePile = new List<Card>();
        internal readonly List<Card> MutableRemovedCards = new List<Card>();

        internal GameState(GameConfig config, List<int> turnOrder, List<List<Card>> hands)
        {
            Config = config;
            TurnOrder = new List<int>(turnOrder).AsReadOnly();
            MutableHands = new List<List<Card>>(hands.Count);
            var readOnlyHands = new List<IReadOnlyList<Card>>(hands.Count);
            foreach (var hand in hands)
            {
                var copy = new List<Card>(hand);
                MutableHands.Add(copy);
                readOnlyHands.Add(copy.AsReadOnly());
            }
            Hands = readOnlyHands.AsReadOnly();
            Pile = MutablePile.AsReadOnly();
            RemovedCards = MutableRemovedCards.AsReadOnly();
            RoundRank = null;
            ActivePlayerId = TurnOrder[0];
            RemainingTurnSeconds = config.TurnDurationSeconds;
        }

        internal GameState(GameState source)
            : this(source.Config, new List<int>(source.TurnOrder), source.MutableHands)
        {
            MutablePile.AddRange(source.Pile);
            MutableRemovedCards.AddRange(source.RemovedCards);
            RoundRank = source.RoundRank;
            ActivePlayerId = source.ActivePlayerId;
            RemainingTurnSeconds = source.RemainingTurnSeconds;
            Revision = source.Revision;
            Phase = source.Phase;
            LastPlay = source.LastPlay;
            Revelation = source.Revelation;
            PendingWinnerId = source.PendingWinnerId;
            WinnerId = source.WinnerId;
        }
    }
}
