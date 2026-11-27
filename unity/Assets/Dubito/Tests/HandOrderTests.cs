using Dubito.Core;
using Dubito.Presentation;
using NUnit.Framework;

namespace Dubito.Tests
{
    public sealed class HandOrderTests
    {
        [Test]
        public void RealPlayerViewSortsByRankThenSuitWithoutMutatingSnapshot()
        {
            var state = GameSetup.Create(new GameConfig(3), new SystemRandomSource(2741));
            var view = PlayerView.Create(state, state.TurnOrder[0]);
            var before = new Card[view.Hand.Count];
            for (int i = 0; i < before.Length; i++) before[i] = view.Hand[i];
            var sorted = HandOrder.Sort(view.Hand);
            Assert.That(sorted, Is.EquivalentTo(before));
            Assert.That(view.Hand, Is.EqualTo(before));
            for (int i = 1; i < sorted.Length; i++)
            {
                Assert.That((int)sorted[i].Rank, Is.GreaterThanOrEqualTo((int)sorted[i - 1].Rank));
                if (sorted[i].Rank == sorted[i - 1].Rank)
                    Assert.That((int)sorted[i].Suit, Is.GreaterThanOrEqualTo((int)sorted[i - 1].Suit));
            }
        }
    }
}
