using System;
using System.Collections.Generic;

namespace Dubito.Core
{
    // Call sequentially on the host. Snapshots stay unchanged after publication.
    // Create one engine per game; command IDs are scoped to that game and player.
    public sealed class GameEngine
    {
        private readonly HashSet<(int PlayerId, Guid CommandId)> processed =
            new HashSet<(int, Guid)>();
        public GameState State { get; private set; }

        public GameEngine(GameState initialState)
        {
            State = initialState ?? throw new ArgumentNullException(nameof(initialState));
        }

        public CommandResult Execute(int playerId, GameCommand command)
        {
            if (command == null || command.Id == Guid.Empty)
                return Result(CommandError.InvalidCommand);
            if (!processed.Add((playerId, command.Id)))
                return new CommandResult(CommandStatus.Ignored, CommandError.None, State.Revision);

            var error = Validate(playerId, command);
            if (error != CommandError.None) return Result(error);

            var next = new GameState(State);
            switch (command.Kind)
            {
                case CommandKind.Play:
                    if (next.PendingWinnerId.HasValue)
                    {
                        Finish(next, next.PendingWinnerId.Value);
                        break;
                    }
                    next.RoundRank = next.RoundRank ?? command.AnnouncedRank;
                    foreach (var card in command.Cards)
                    {
                        next.MutableHands[playerId].Remove(card);
                        next.MutablePile.Add(card);
                    }
                    Activate(next, NextPlayer(next, playerId));
                    next.LastPlay = new LastPlay(playerId, next.ActivePlayerId,
                        next.RoundRank.Value, command.Cards);
                    if (next.Hands[playerId].Count == 0) next.PendingWinnerId = playerId;
                    break;
                case CommandKind.Doubt:
                    next.Revelation = new DoubtRevelation(next.LastPlay, playerId);
                    next.LastPlay = next.LastPlay.CloseDoubtWindow();
                    next.Phase = GamePhase.Revelation;
                    break;
                case CommandKind.Discard:
                    next.MutableRemovedCards.AddRange(next.Pile);
                    ClearRound(next);
                    Activate(next, NextPlayer(next, playerId));
                    break;
            }
            Publish(next);
            return Result(CommandError.None);
        }

        // Host-only continuation, after presenting the revelation. No clock in Core.
        public CommandResult ResolveRevelation(long expectedRevision)
        {
            if (expectedRevision != State.Revision) return Result(CommandError.StaleRevision);
            if (State.Phase == GamePhase.Finished) return Result(CommandError.GameFinished);
            if (State.Phase != GamePhase.Revelation) return Result(CommandError.NoRevelation);
            var next = new GameState(State);
            var reveal = next.Revelation;
            int recipient = reveal.IsDoubtCorrect ? reveal.AccusedPlayerId : reveal.DoubterId;
            next.MutableHands[recipient].AddRange(next.Pile);
            int? winner = reveal.IsDoubtCorrect ? null : next.PendingWinnerId;
            next.PendingWinnerId = null;
            ClearRound(next);
            next.Phase = GamePhase.Playing;
            if (winner.HasValue) Finish(next, winner.Value);
            else Activate(next, reveal.IsDoubtCorrect ? reveal.DoubterId : NextPlayer(next, reveal.DoubterId));
            Publish(next);
            return Result(CommandError.None);
        }

        private CommandError Validate(int playerId, GameCommand c)
        {
            if (c.ExpectedRevision != State.Revision) return CommandError.StaleRevision;
            if (State.Phase == GamePhase.Finished) return CommandError.GameFinished;
            if (State.Phase == GamePhase.Revelation) return CommandError.RevelationInProgress;
            if (playerId != State.ActivePlayerId) return CommandError.NotYourTurn;
            switch (c.Kind)
            {
                case CommandKind.Play:
                    if (c.Cards.Count < 1 || c.Cards.Count > 4) return CommandError.InvalidCardCount;
                    var distinct = new HashSet<Card>();
                    foreach (var card in c.Cards)
                    {
                        if (!distinct.Add(card)) return CommandError.DuplicateCard;
                        if (!State.MutableHands[playerId].Contains(card)) return CommandError.CardNotInHand;
                    }
                    if (c.AnnouncedRank.HasValue &&
                        (c.AnnouncedRank < CardRank.Ace || c.AnnouncedRank > CardRank.King))
                        return CommandError.InvalidAnnouncement;
                    if (!State.RoundRank.HasValue && !c.AnnouncedRank.HasValue)
                        return CommandError.OpeningAnnouncementRequired;
                    if (State.RoundRank.HasValue && c.AnnouncedRank.HasValue && c.AnnouncedRank != State.RoundRank)
                        return CommandError.AnnouncementMustMatchRound;
                    return CommandError.None;
                case CommandKind.Doubt:
                    var play = State.LastPlay;
                    return play != null && play.IsDoubtWindowOpen && play.RespondentId == playerId
                        ? CommandError.None : CommandError.NoDoubtablePlay;
                case CommandKind.Discard:
                    if (State.PendingWinnerId.HasValue) return CommandError.PendingVictory;
                    return State.Pile.Count == 0 ? CommandError.EmptyPile : CommandError.None;
                default:
                    return CommandError.InvalidCommand;
            }
        }

        private CommandResult Result(CommandError error) => new CommandResult(
            error == CommandError.None ? CommandStatus.Applied : CommandStatus.Rejected, error, State.Revision);
        private void Publish(GameState next)
        {
            next.Revision = State.Revision + 1;
            State = next;
        }
        private static int NextPlayer(GameState s, int id)
        {
            for (int i = 0; i < s.TurnOrder.Count; i++)
                if (s.TurnOrder[i] == id) return s.TurnOrder[(i + 1) % s.TurnOrder.Count];
            throw new InvalidOperationException("Player is absent from turn order.");
        }
        private static void Activate(GameState s, int id)
        {
            s.ActivePlayerId = id;
            s.RemainingTurnSeconds = s.Config.TurnDurationSeconds;
        }
        private static void ClearRound(GameState s)
        {
            s.MutablePile.Clear();
            s.RoundRank = null;
            s.LastPlay = null;
            s.Revelation = null;
        }
        private static void Finish(GameState s, int winnerId)
        {
            s.WinnerId = winnerId;
            s.PendingWinnerId = null;
            s.Phase = GamePhase.Finished;
            if (s.LastPlay != null) s.LastPlay = s.LastPlay.CloseDoubtWindow();
        }
    }
}
