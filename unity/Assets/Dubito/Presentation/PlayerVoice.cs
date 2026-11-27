using Dubito.Core;

namespace Dubito.Presentation
{
    // Who the interface talks to (D-59). Online, each player only sees their own seat: "tu" is right.
    // In hot-seat (UX §9) one person plays every seat, so texts name the player instead of "tu".
    public static class PlayerVoice
    {
        public static bool HotSeat { get; set; }

        // True when a text about `playerId` should address the reader directly ("tu", "toi", "ta").
        public static bool Addresses(PlayerView view, int playerId) => !HotSeat && playerId == view.PlayerId;
    }
}
