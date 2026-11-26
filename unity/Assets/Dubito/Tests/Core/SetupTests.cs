using System;
using System.Collections.Generic;
using System.Linq;
using Dubito.Core;
using NUnit.Framework;

namespace Dubito.Tests.Core
{
    public class SetupTests
    {
        [TestCase(TestName = "C-SET-01")]
        public void TwoPlayersReceiveTwentySixCards()
        {
            var game = CreateGame(2);
            Assert.That(game.Hands.Select(hand => hand.Count), Is.EqualTo(new[] { 26, 26 }));
            AssertInitialInvariants(game);
        }

        [TestCase(TestName = "C-SET-02")]
        public void FivePlayersReceiveTenOrElevenCards()
        {
            var game = CreateGame(5);
            Assert.That(game.Hands.Count(hand => hand.Count == 11), Is.EqualTo(2));
            Assert.That(game.Hands.Count(hand => hand.Count == 10), Is.EqualTo(3));
            AssertInitialInvariants(game);
        }

        [TestCase(TestName = "C-SET-03")]
        public void SixPlayersReceiveEightOrNineCards()
        {
            var game = CreateGame(6);
            Assert.That(game.Hands.Count(hand => hand.Count == 9), Is.EqualTo(4));
            Assert.That(game.Hands.Count(hand => hand.Count == 8), Is.EqualTo(2));
            AssertInitialInvariants(game);
        }

        [TestCase(TestName = "C-SET-04")]
        public void EveryCardIsDealtExactlyOnce()
        {
            var deck = Deck.CreateStandard();
            Assert.That(deck.Count, Is.EqualTo(52));
            Assert.That(deck.Distinct().Count(), Is.EqualTo(52));
            Assert.That(deck.Select(card => card.Rank).Distinct().Count(), Is.EqualTo(13));
            foreach (CardRank rank in Enum.GetValues(typeof(CardRank)))
                Assert.That(deck.Count(card => card.Rank == rank), Is.EqualTo(4));

            for (int players = 2; players <= 6; players++)
            for (int seed = 0; seed < 20; seed++)
            {
                var game = CreateGame(players, seed);
                AssertInitialInvariants(game);
                Assert.That(game.Hands.SelectMany(hand => hand), Is.EquivalentTo(deck));
                Assert.That(game.Hands.All(hand => hand.Count == 52 / players ||
                    hand.Count == 52 / players + 1), Is.True);
            }
            // The injected seed controls the complete setup, including deck and turn order.
            var first = CreateGame(5, 123);
            var repeat = CreateGame(5, 123);
            Assert.That(repeat.TurnOrder, Is.EqualTo(first.TurnOrder));
            Assert.That(repeat.Hands.SelectMany(hand => hand),
                Is.EqualTo(first.Hands.SelectMany(hand => hand)));
            var other = CreateGame(5, 124);
            Assert.That(other.Hands.SelectMany(hand => hand),
                Is.Not.EqualTo(first.Hands.SelectMany(hand => hand)));
        }

        [TestCase(TestName = "C-SET-05")]
        public void FirstInRandomOrderStartsWithEmptyPileAndFullTimer()
        {
            var draws = IdentityShuffleDraws(52);
            draws.AddRange(new[] { 0, 0, 0, 0 });
            draws.AddRange(new[] { 0, 0 });
            var random = new ScriptedRandom(draws);
            var game = GameSetup.Create(new GameConfig(5), random);
            Assert.That(game.TurnOrder, Is.EqualTo(new[] { 1, 2, 3, 4, 0 }));
            Assert.That(game.ActivePlayerId, Is.EqualTo(1));
            Assert.That(game.RemainingTurnSeconds, Is.EqualTo(45));
            AssertInitialInvariants(game);
            Assert.That(random.RemainingDraws, Is.EqualTo(0));
        }

        [TestCase(TestName = "C-SET-06")]
        public void UnsupportedPlayerCountsAreRejected()
        {
            foreach (int players in new[] { -1, 0, 1, 7, 100 })
            {
                var error = Assert.Throws<ArgumentOutOfRangeException>(() => new GameConfig(players));
                Assert.That(error.ParamName, Is.EqualTo("playerCount"));
                Assert.That(error.Message, Does.Contain("2").And.Contain("6"));
            }
        }

        [TestCase(TestName = "C-SET-07")]
        public void ConfiguredTimerIsStoredAndCannotBeChanged()
        {
            foreach (int seconds in new[] { 30, 45, 60, 90 })
            {
                var config = new GameConfig(3, seconds);
                var game = GameSetup.Create(config, new SystemRandomSource(17));
                Assert.That(game.Config, Is.SameAs(config));
                Assert.That(game.Config.TurnDurationSeconds, Is.EqualTo(seconds));
                Assert.That(game.RemainingTurnSeconds, Is.EqualTo(seconds));
                AssertInitialInvariants(game);
            }
            Assert.That(typeof(GameConfig).GetProperties().All(p => !p.CanWrite), Is.True);
            Assert.That(typeof(GameState).GetProperty("Config").CanWrite, Is.False);
        }

        [TestCase(TestName = "C-SET-08")]
        public void UnsupportedTimerIsRejected()
        {
            foreach (int seconds in new[] { -1, 0, 29, 50, 91 })
            {
                var error = Assert.Throws<ArgumentOutOfRangeException>(() => new GameConfig(2, seconds));
                Assert.That(error.ParamName, Is.EqualTo("turnDurationSeconds"));
                Assert.That(error.Message, Does.Contain("30").And.Contain("45").And.Contain("60").And.Contain("90"));
            }
        }

        [TestCase(TestName = "C-SET-09")]
        public void ExtraCardsGoToDistinctRandomPlayersThirdAndFifthInOrder()
        {
            var draws = IdentityShuffleDraws(52);
            // Non-identity turn order ensures recipients are interpreted in turn order (J1...J5).
            draws.AddRange(new[] { 0, 0, 0, 0 });
            // Partial forward shuffle: choose position 2, then position 1 + 3 = 4.
            draws.AddRange(new[] { 2, 3 });
            var random = new ScriptedRandom(draws);
            var game = GameSetup.Create(new GameConfig(5), random);
            Assert.That(game.TurnOrder.Select(id => game.Hands[id].Count),
                Is.EqualTo(new[] { 10, 10, 11, 10, 11 }));
            Assert.That(random.RemainingDraws, Is.EqualTo(0));
            AssertInitialInvariants(game);
        }

        private static GameState CreateGame(int players, int seed = 42)
        {
            return GameSetup.Create(new GameConfig(players), new SystemRandomSource(seed));
        }

        private static void AssertInitialInvariants(GameState game)
        {
            var cards = game.Hands.SelectMany(hand => hand).Concat(game.Pile).Concat(game.RemovedCards).ToArray();
            Assert.That(cards.Length, Is.EqualTo(52));
            Assert.That(cards.Distinct().Count(), Is.EqualTo(52));
            Assert.That(game.Pile, Is.Empty);
            Assert.That(game.RemovedCards, Is.Empty);
            Assert.That(game.RoundRank, Is.Null);
            Assert.That(game.TurnOrder, Is.EquivalentTo(Enumerable.Range(0, game.Config.PlayerCount)));
            Assert.That(game.TurnOrder.Count(id => id == game.ActivePlayerId), Is.EqualTo(1));
            Assert.That(game.ActivePlayerId, Is.EqualTo(game.TurnOrder[0]));
        }

        private static List<int> IdentityShuffleDraws(int size)
        {
            return Enumerable.Range(1, size - 1).Reverse().ToList();
        }

        private sealed class ScriptedRandom : IRandomSource
        {
            private readonly Queue<int> draws;
            public int RemainingDraws => draws.Count;

            public ScriptedRandom(IEnumerable<int> draws)
            {
                this.draws = new Queue<int>(draws);
            }

            public int Next(int exclusiveMax)
            {
                Assert.That(draws.Count, Is.GreaterThan(0), "Unexpected random draw");
                int value = draws.Dequeue();
                Assert.That(value, Is.GreaterThanOrEqualTo(0).And.LessThan(exclusiveMax));
                return value;
            }
        }
    }
}
