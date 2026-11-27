using System;
using System.Collections.Generic;

namespace Dubito.Core
{
    // Detached snapshot for both presentation and protocol. Never retain host objects.
    public sealed class PlayerView
    {
        public int PlayerId { get; }
        public IReadOnlyList<Card> Hand { get; }
        // Stable player ID indexes, including departed players (whose count is zero).
        public IReadOnlyList<int> CardCounts { get; }
        public IReadOnlyList<int> TurnOrder { get; }
        public int PileCount { get; }
        public CardRank? RoundRank { get; }
        public PlayAnnouncement LastAnnouncement { get; }
        public int ActivePlayerId { get; }
        public double RemainingTurnSeconds { get; }
        public bool IsTimerPaused { get; }
        public GamePhase Phase { get; }
        public RevelationView Revelation { get; }
        public int? PendingWinnerId { get; }
        public int? WinnerId { get; }
        // This player's consecutive skipped turns (RULES §5), for the exclusion warning (UX §5).
        public int OwnSkippedTurns { get; }
        // Finished with WinnerId == null represents a session stopped without winner.
        public long Revision { get; }
        public IReadOnlyList<CommandKind> AllowedActions { get; }

        public static PlayerView Create(GameState state, int playerId)
        {
            if (state == null) throw new ArgumentNullException(nameof(state));
            if (playerId < 0 || playerId >= state.Hands.Count)
                throw new ArgumentOutOfRangeException(nameof(playerId));
            return new PlayerView(state, playerId);
        }

        private PlayerView(GameState state, int playerId)
        {
            PlayerId = playerId;
            Hand = new List<Card>(state.Hands[playerId]).AsReadOnly();
            var counts = new List<int>(state.Hands.Count);
            foreach (var hand in state.Hands) counts.Add(hand.Count);
            CardCounts = counts.AsReadOnly();
            TurnOrder = new List<int>(state.TurnOrder).AsReadOnly();
            PileCount = state.Pile.Count;
            RoundRank = state.RoundRank;
            LastAnnouncement = state.LastPlay == null ? null : new PlayAnnouncement(state.LastPlay);
            ActivePlayerId = state.ActivePlayerId;
            RemainingTurnSeconds = state.RemainingTurnSeconds;
            IsTimerPaused = state.IsTimerPaused;
            Phase = state.Phase;
            Revelation = state.Phase == GamePhase.Revelation && state.Revelation != null
                ? new RevelationView(state.Revelation) : null;
            PendingWinnerId = state.PendingWinnerId;
            WinnerId = state.WinnerId;
            OwnSkippedTurns = state.SkippedTurns[playerId];
            Revision = state.Revision;
            var actions = new List<CommandKind>();
            if (state.Phase == GamePhase.Playing && state.ActivePlayerId == playerId)
            {
                if (Hand.Count > 0) actions.Add(CommandKind.Play);
                var play = state.LastPlay;
                if (play != null && play.IsDoubtWindowOpen && play.RespondentId == playerId)
                    actions.Add(CommandKind.Doubt);
                if (PileCount > 0 && !PendingWinnerId.HasValue) actions.Add(CommandKind.Discard);
            }
            AllowedActions = actions.AsReadOnly();
        }
    }

    public sealed class PlayAnnouncement
    {
        public int PlayerId { get; }
        public int CardCount { get; }
        public CardRank AnnouncedRank { get; }

        internal PlayAnnouncement(LastPlay play)
        {
            PlayerId = play.PlayerId;
            CardCount = play.Cards.Count;
            AnnouncedRank = play.AnnouncedRank;
        }
    }

    public sealed class RevelationView
    {
        public int AccusedPlayerId { get; }
        public int DoubterId { get; }
        public bool IsAutomatic { get; }
        public IReadOnlyList<Card> Cards { get; }
        // All revealed cards and their verdict are public (RULES §4.2, UX §5).
        // Core knows the verdict when revelation starts; presentation controls its timing.
        public bool IsDoubtCorrect { get; }

        internal RevelationView(DoubtRevelation revelation)
        {
            AccusedPlayerId = revelation.AccusedPlayerId;
            DoubterId = revelation.DoubterId;
            IsAutomatic = revelation.IsAutomatic;
            Cards = new List<Card>(revelation.Cards).AsReadOnly();
            IsDoubtCorrect = revelation.IsDoubtCorrect;
        }
    }
}
