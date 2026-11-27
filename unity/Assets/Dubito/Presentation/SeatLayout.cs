using System;
using UnityEngine;

namespace Dubito.Presentation
{
    public static class SeatLayout
    {
        public const float PelvisRadius = 1.05f;
        public const float EyeHeight = 1.15f;
        public const float ChairCenterRadius = 1.12f;
        public const float TableHeight = 0.75f;
        // Resting gaze: a bit above the table centre so opponents' heads and names stay in view.
        public const float LookTargetHeight = 0.85f;
        public const int MinSeats = 2;
        public const int MaxSeats = 6;

        public static float YawDegrees(int seat, int seatCount, int localSeat)
        {
            if (seatCount < MinSeats || seatCount > MaxSeats)
                throw new ArgumentOutOfRangeException(nameof(seatCount));
            if (seat < 0 || seat >= seatCount)
                throw new ArgumentOutOfRangeException(nameof(seat));
            if (localSeat < 0 || localSeat >= seatCount)
                throw new ArgumentOutOfRangeException(nameof(localSeat));
            return -360f * ((seat - localSeat + seatCount) % seatCount) / seatCount;
        }

        public static Vector3 SeatPosition(int seat, int seatCount, int localSeat, float radius)
        {
            if (float.IsNaN(radius) || float.IsInfinity(radius) || radius <= 0f)
                throw new ArgumentOutOfRangeException(nameof(radius));
            return SeatRotation(seat, seatCount, localSeat) * (Vector3.back * radius);
        }

        public static Quaternion SeatRotation(int seat, int seatCount, int localSeat) =>
            Quaternion.Euler(0f, YawDegrees(seat, seatCount, localSeat), 0f);
    }
}
