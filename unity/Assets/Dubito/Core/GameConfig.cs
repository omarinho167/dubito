using System;

namespace Dubito.Core
{
    public sealed class GameConfig
    {
        public int PlayerCount { get; }
        public int TurnDurationSeconds { get; }

        public GameConfig(int playerCount, int turnDurationSeconds = 45)
        {
            if (playerCount < 2 || playerCount > 6)
                throw new ArgumentOutOfRangeException(nameof(playerCount), playerCount,
                    "A game requires 2 to 6 players.");
            if (turnDurationSeconds != 30 && turnDurationSeconds != 45 &&
                turnDurationSeconds != 60 && turnDurationSeconds != 90)
                throw new ArgumentOutOfRangeException(nameof(turnDurationSeconds), turnDurationSeconds,
                    "Turn duration must be 30, 45, 60 or 90 seconds.");
            PlayerCount = playerCount;
            TurnDurationSeconds = turnDurationSeconds;
        }
    }
}
