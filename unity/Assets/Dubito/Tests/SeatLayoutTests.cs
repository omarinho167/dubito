using System;
using Dubito.Presentation;
using NUnit.Framework;
using UnityEngine;

namespace Dubito.Tests
{
    public class SeatLayoutTests
    {
        const float Tolerance = 0.0001f;

        [TestCase(2), TestCase(3), TestCase(4), TestCase(5), TestCase(6)]
        public void EveryLocalSeatIsAtTheBackAndFacesForward(int count)
        {
            for (int local = 0; local < count; local++)
            {
                AssertVector(new Vector3(0, 0, -SeatLayout.PelvisRadius),
                    SeatLayout.SeatPosition(local, count, local, SeatLayout.PelvisRadius));
                AssertVector(Vector3.forward, SeatLayout.SeatRotation(local, count, local) * Vector3.forward);
                Assert.That(SeatLayout.YawDegrees(local, count, local), Is.Zero);
            }
        }

        [TestCase(2), TestCase(3), TestCase(4), TestCase(5), TestCase(6)]
        public void SeatsHaveRegularSpacingCorrectRadiusAndFaceTheCentre(int count)
        {
            const float radius = 2.3f;
            float chord = 2f * radius * Mathf.Sin(Mathf.PI / count);
            for (int local = 0; local < count; local++)
            for (int seat = 0; seat < count; seat++)
            {
                Vector3 position = SeatLayout.SeatPosition(seat, count, local, radius);
                Vector3 next = SeatLayout.SeatPosition((seat + 1) % count, count, local, radius);
                Assert.That(position.y, Is.EqualTo(0f).Within(Tolerance));
                Assert.That(position.magnitude, Is.EqualTo(radius).Within(Tolerance));
                Assert.That(Vector3.Distance(position, next), Is.EqualTo(chord).Within(Tolerance));
                AssertVector(-position.normalized, SeatLayout.SeatRotation(seat, count, local) * Vector3.forward);
            }
        }

        [TestCase(3), TestCase(4), TestCase(5), TestCase(6)]
        public void NextSeatIsOnTheLocalPlayersRightIncludingIndexWrap(int count)
        {
            for (int local = 0; local < count; local++)
                Assert.That(SeatLayout.SeatPosition((local + 1) % count, count, local, 1f).x, Is.GreaterThan(0f));
        }

        [Test]
        public void TwoPlayersSitOppositeEachOther()
        {
            AssertVector(Vector3.forward, SeatLayout.SeatPosition(1, 2, 0, 1f));
        }

        [TestCase(2), TestCase(3), TestCase(4), TestCase(5), TestCase(6)]
        public void ChangingLocalSeatOnlyShiftsSeatIndices(int count)
        {
            for (int local = 0; local < count; local++)
            for (int offset = 0; offset < count; offset++)
            {
                int seat = (local + offset) % count;
                AssertVector(SeatLayout.SeatPosition(offset, count, 0, 1f),
                    SeatLayout.SeatPosition(seat, count, local, 1f));
                Assert.That(Quaternion.Angle(SeatLayout.SeatRotation(offset, count, 0),
                    SeatLayout.SeatRotation(seat, count, local)), Is.LessThan(Tolerance));
                Assert.That(SeatLayout.YawDegrees(seat, count, local),
                    Is.EqualTo(-360f * offset / count).Within(Tolerance));
            }
        }

        [TestCase(0, 1, 0), TestCase(0, 7, 0), TestCase(-1, 4, 0),
         TestCase(4, 4, 0), TestCase(0, 4, -1), TestCase(0, 4, 4)]
        public void AllLayoutMethodsRejectInvalidIndicesOrCounts(int seat, int count, int local)
        {
            Assert.Throws<ArgumentOutOfRangeException>(() => SeatLayout.YawDegrees(seat, count, local));
            Assert.Throws<ArgumentOutOfRangeException>(() => SeatLayout.SeatPosition(seat, count, local, 1f));
            Assert.Throws<ArgumentOutOfRangeException>(() => SeatLayout.SeatRotation(seat, count, local));
        }

        [TestCase(0f), TestCase(-1f), TestCase(float.NaN), TestCase(float.PositiveInfinity), TestCase(float.NegativeInfinity)]
        public void PositionRejectsInvalidRadius(float radius)
        {
            Assert.Throws<ArgumentOutOfRangeException>(() => SeatLayout.SeatPosition(0, 4, 0, radius));
        }

        static void AssertVector(Vector3 expected, Vector3 actual) =>
            Assert.That(Vector3.Distance(expected, actual), Is.LessThan(Tolerance));
    }
}
