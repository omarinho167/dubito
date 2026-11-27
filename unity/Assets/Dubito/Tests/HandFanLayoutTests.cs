using Dubito.Presentation;
using NUnit.Framework;
using UnityEngine;

namespace Dubito.Tests
{
    public sealed class HandFanLayoutTests
    {
        [TestCase(0)] [TestCase(1)] [TestCase(13)] [TestCase(52)]
        public void FanIsFiniteOrderedAndBounded(int count)
        {
            var poses = HandFanLayout.Calculate(count);
            Assert.That(poses.Length, Is.EqualTo(count));
            for (int i = 0; i < count; i++)
            {
                Assert.That(float.IsNaN(poses[i].position.x), Is.False);
                Assert.That(poses[i].position.magnitude, Is.LessThan(.2f));
                Assert.That(Mathf.Abs(poses[i].position.x) + .055f, Is.LessThan(.18f));
                if (i == 0) continue;
                Assert.That(poses[i].position.x, Is.GreaterThan(poses[i - 1].position.x));
                Assert.That(poses[i].position.y, Is.GreaterThan(poses[i - 1].position.y));
            }
            if (count == 1)
            {
                Assert.That(poses[0].position, Is.EqualTo(Vector3.zero));
                Assert.That(Quaternion.Angle(poses[0].rotation, Quaternion.identity), Is.LessThan(.001));
            }
        }

        [Test]
        public void FullDeckCompressesSpacingInsteadOfExpandingArc()
        {
            var a = HandFanLayout.Calculate(26);
            var b = HandFanLayout.Calculate(52);
            Assert.That(b[51].position.x - b[0].position.x,
                Is.EqualTo(a[25].position.x - a[0].position.x).Within(.00001));
        }

        [TestCase(-1)] [TestCase(53)]
        public void RejectsInvalidCounts(int count) =>
            Assert.Throws<System.ArgumentOutOfRangeException>(() => HandFanLayout.Calculate(count));
    }
}
