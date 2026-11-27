using Dubito.Core;
using Dubito.Presentation;
using NUnit.Framework;

namespace Dubito.Tests
{
    public sealed class CardNamesTests
    {
        [TestCase(CardRank.Ace, "As")]
        [TestCase(CardRank.Two, "2")]
        [TestCase(CardRank.Three, "3")]
        [TestCase(CardRank.Four, "4")]
        [TestCase(CardRank.Five, "5")]
        [TestCase(CardRank.Six, "6")]
        [TestCase(CardRank.Seven, "7")]
        [TestCase(CardRank.Eight, "8")]
        [TestCase(CardRank.Nine, "9")]
        [TestCase(CardRank.Ten, "10")]
        [TestCase(CardRank.Jack, "Valet")]
        [TestCase(CardRank.Queen, "Dame")]
        [TestCase(CardRank.King, "Roi")]
        public void FrenchRank(CardRank rank, string name) => Assert.That(CardNames.Rank(rank), Is.EqualTo(name));

        [Test]
        public void PlayerNamesAndPlurals()
        {
            Assert.That(CardNames.PlayerName(0), Is.EqualTo("Joueur 1"));
            Assert.That(CardNames.PlayerName(5), Is.EqualTo("Joueur 6"));
            Assert.That(CardNames.RankPlural(CardRank.Ace, 3), Is.EqualTo("As"));
            Assert.That(CardNames.RankPlural(CardRank.Queen, 3), Is.EqualTo("Dames"));
            Assert.That(CardNames.RankPlural(CardRank.King, 1), Is.EqualTo("Roi"));
        }
    }
}
