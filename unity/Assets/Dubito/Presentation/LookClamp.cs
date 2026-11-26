using System;
using UnityEngine;

namespace Dubito.Presentation
{
    public static class LookClamp
    {
        // Positive pitch means looking up (the camera's Euler X uses the opposite sign).
        public static Vector2 Apply(Vector2 currentYawPitch, Vector2 deltaYawPitch,
            float maxYaw, float maxPitchUp, float maxPitchDown)
        {
            ValidateLimit(maxYaw, nameof(maxYaw));
            ValidateLimit(maxPitchUp, nameof(maxPitchUp));
            ValidateLimit(maxPitchDown, nameof(maxPitchDown));
            return new Vector2(
                Mathf.Clamp(currentYawPitch.x + deltaYawPitch.x, -maxYaw, maxYaw),
                Mathf.Clamp(currentYawPitch.y + deltaYawPitch.y, -maxPitchDown, maxPitchUp));
        }

        static void ValidateLimit(float value, string name)
        {
            if (float.IsNaN(value) || float.IsInfinity(value) || value < 0f)
                throw new ArgumentOutOfRangeException(name);
        }
    }
}
