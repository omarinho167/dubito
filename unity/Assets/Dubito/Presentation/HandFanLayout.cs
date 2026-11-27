using System;
using UnityEngine;

namespace Dubito.Presentation
{
    public static class HandFanLayout
    {
        public const float DefaultRadius = .32f;
        public const float DefaultMaxAngle = 42f;
        public const float DefaultSpacing = .018f;
        public const float LayerSpacing = .0008f;

        // XZ is the card plane, +Y is its face. Increasing layers expose the left corners.
        // Saturating the total arc compresses spacing after about 14 cards.
        public static Pose CardPose(int index, int count, float radius = DefaultRadius,
            float maxAngle = DefaultMaxAngle, float spacing = DefaultSpacing)
        {
            Validate(count, radius, maxAngle, spacing);
            if (index < 0 || index >= count) throw new ArgumentOutOfRangeException(nameof(index));
            float total = Mathf.Min(maxAngle * Mathf.Deg2Rad, (count - 1) * spacing / radius);
            float angle = count <= 1 ? 0 : total * (index / (float)(count - 1) - .5f);
            return new Pose(new Vector3(radius * Mathf.Sin(angle), index * LayerSpacing,
                radius * (Mathf.Cos(angle) - 1)), Quaternion.Euler(0, angle * Mathf.Rad2Deg, 0));
        }

        public static Pose[] Calculate(int count, float radius = DefaultRadius,
            float maxAngle = DefaultMaxAngle, float spacing = DefaultSpacing)
        {
            Validate(count, radius, maxAngle, spacing);
            var result = new Pose[count];
            for (int i = 0; i < count; i++) result[i] = CardPose(i, count, radius, maxAngle, spacing);
            return result;
        }

        static void Validate(int count, float radius, float maxAngle, float spacing)
        {
            if (count < 0 || count > 52) throw new ArgumentOutOfRangeException(nameof(count));
            if (float.IsNaN(radius) || float.IsInfinity(radius) || radius <= 0) throw new ArgumentOutOfRangeException(nameof(radius));
            if (float.IsNaN(maxAngle) || maxAngle < 0 || maxAngle > 120) throw new ArgumentOutOfRangeException(nameof(maxAngle));
            if (float.IsNaN(spacing) || float.IsInfinity(spacing) || spacing <= 0) throw new ArgumentOutOfRangeException(nameof(spacing));
        }
    }
}
