using System;
using System.Collections.Generic;
using Dubito.Core;

namespace Dubito.Networking
{
    public interface IGameSession
    {
        int ControlledPlayerId { get; }
        // Initial turn order: seats never move when a player leaves.
        IReadOnlyList<int> SeatOrder { get; }
        PlayerView CurrentView { get; }
        event Action<PlayerView> ViewChanged;
        event Action<CommandError> CommandRejected;
        void Play(IReadOnlyList<Card> cards, CardRank announcedRank);
        void Doubt();
        void Discard();
    }
}
