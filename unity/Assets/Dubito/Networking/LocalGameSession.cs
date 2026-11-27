using System;
using System.Collections.Generic;
using Dubito.Core;

namespace Dubito.Networking
{
    // Sequential local host adapter. Hot-seat switching is explicit; callers only
    // receive the controlled seat's filtered snapshot, never the host state.
    public sealed class LocalGameSession : IGameSession
    {
        private readonly GameEngine engine;

        public int ControlledPlayerId { get; private set; }
        public IReadOnlyList<int> SeatOrder { get; }
        public PlayerView CurrentView { get; private set; }
        public event Action<PlayerView> ViewChanged;
        public event Action<CommandError> CommandRejected;

        public LocalGameSession(GameConfig config, IRandomSource random)
        {
            var state = GameSetup.Create(config, random);
            engine = new GameEngine(state);
            SeatOrder = new List<int>(state.TurnOrder).AsReadOnly();
            ControlledPlayerId = SeatOrder[0];
            CurrentView = PlayerView.Create(state, ControlledPlayerId);
        }

        public void Play(IReadOnlyList<Card> cards, CardRank announcedRank) =>
            Execute(new GameCommand(Guid.NewGuid(), CurrentView.Revision,
                CommandKind.Play, cards, announcedRank));

        public void Doubt() => Execute(new GameCommand(Guid.NewGuid(),
            CurrentView.Revision, CommandKind.Doubt));

        public void Discard() => Execute(new GameCommand(Guid.NewGuid(),
            CurrentView.Revision, CommandKind.Discard));

        public void Tick(double seconds) => Complete(engine.AdvanceTime(seconds));

        public void ResolveRevelation() =>
            Complete(engine.ResolveRevelation(CurrentView.Revision));

        // Debug hot-seat (UX §9): a seat leaves the game, as on a disconnection (RULES §7).
        public void RemovePlayer(int playerId) => Complete(engine.RemovePlayer(playerId));

        public void SetControlledPlayer(int playerId)
        {
            // Validate before changing either property; departed seats remain valid.
            if (playerId < 0 || playerId >= SeatOrder.Count)
                throw new ArgumentOutOfRangeException(nameof(playerId));
            if (playerId == ControlledPlayerId) return;
            ControlledPlayerId = playerId;
            RefreshView(true);
        }

        private void Execute(GameCommand command) =>
            Complete(engine.Execute(ControlledPlayerId, command));

        private void Complete(CommandResult result)
        {
            if (result.Status == CommandStatus.Rejected)
            {
                CommandRejected?.Invoke(result.Error);
                return;
            }
            RefreshView(false);
        }

        private void RefreshView(bool seatChanged)
        {
            bool publish = seatChanged || CurrentView.Revision != engine.State.Revision;
            // Countdown-only ticks change the snapshot but deliberately keep the
            // revision in Core. Poll CurrentView for that timer between events.
            CurrentView = PlayerView.Create(engine.State, ControlledPlayerId);
            if (publish) ViewChanged?.Invoke(CurrentView);
        }
    }
}
