using Dubito.Presentation;
using NUnit.Framework;
using UnityEngine;

namespace Dubito.Tests
{
    public class LookClampTests
    {
        [TestCase(60f, 15f, 60f, 15f), TestCase(-60f, -30f, -60f, -30f),
         TestCase(1000f, 1000f, 60f, 15f), TestCase(-1000f, -1000f, -60f, -30f)]
        public void ReachesLimitsWithoutExceedingThem(float yaw, float pitch, float expectedYaw, float expectedPitch)
        {
            Assert.That(LookClamp.Apply(Vector2.zero, new Vector2(yaw, pitch), 60f, 15f, 30f),
                Is.EqualTo(new Vector2(expectedYaw, expectedPitch)));
        }

        [TestCase(60f, 15f), TestCase(-60f, -30f)]
        public void OppositeDeltaCanReturnFromTheLimitToTheCentre(float yaw, float pitch)
        {
            Vector2 edge = LookClamp.Apply(Vector2.zero, new Vector2(yaw * 10, pitch * 10), 60f, 15f, 30f);
            Assert.That(LookClamp.Apply(edge, -edge, 60f, 15f, 30f), Is.EqualTo(Vector2.zero));
        }

        [Test]
        public void SmallDeltasAccumulateWithoutChangingTheirSign()
        {
            Assert.That(LookClamp.Apply(new Vector2(10f, -10f), new Vector2(2f, 3f), 60f, 15f, 30f),
                Is.EqualTo(new Vector2(12f, -7f)));
        }

        [Test]
        public void RepeatedOutwardInputStaysAtTheLimit()
        {
            Vector2 look = Vector2.zero;
            for (int i = 0; i < 100; i++)
                look = LookClamp.Apply(look, new Vector2(10f, -10f), 60f, 15f, 30f);
            Assert.That(look, Is.EqualTo(new Vector2(60f, -30f)));
        }
    }
}
