using System;
using System.Collections.Generic;
using System.Linq;
using Dubito.Core;
using NUnit.Framework;

namespace Dubito.Tests.Core
{
    public class ActionTests
    {
        private static Card C(CardRank rank, CardSuit suit = CardSuit.Clubs) => new Card(rank, suit);

        // Keep all 52 physical cards in every scenario, even with deliberately small hands.
        private static GameEngine Game(int players = 3, params Card[][] specifiedHands)
        {
            var remaining = Deck.CreateStandard().ToList();
            var hands = Enumerable.Range(0, players).Select(_ => new List<Card>()).ToList();
            for (int i = 0; i < specifiedHands.Length; i++)
            foreach (var card in specifiedHands[i])
            {
                Assert.That(remaining.Remove(card), Is.True);
                hands[i].Add(card);
            }
            foreach (var card in remaining) hands[players - 1].Add(card);
            var engine = new GameEngine(new GameState(new GameConfig(players), Enumerable.Range(0, players).ToList(), hands));
            Invariants(engine.State);
            return engine;
        }

        private static void Invariants(GameState s)
        {
            var all = s.Hands.SelectMany(h => h).Concat(s.Pile).Concat(s.RemovedCards).ToArray();
            Assert.That(all.Length, Is.EqualTo(52));
            Assert.That(all.Distinct().Count(), Is.EqualTo(52));
            Assert.That(s.Pile.Count == 0, Is.EqualTo(!s.RoundRank.HasValue));
            if (s.Phase != GamePhase.Finished)
                Assert.That(s.TurnOrder.Count(p => p == s.ActivePlayerId), Is.EqualTo(1));
            if (s.PendingWinnerId.HasValue)
            {
                Assert.That(s.Hands[s.PendingWinnerId.Value], Is.Empty);
                Assert.That(s.LastPlay.PlayerId, Is.EqualTo(s.PendingWinnerId.Value));
            }
        }

        private static CommandResult Send(GameEngine e, int player, GameCommand c, CommandError error = CommandError.None)
        {
            var before = e.State;
            var oldCards = before.Hands.SelectMany(h => h).Concat(before.Pile).Concat(before.RemovedCards).ToArray();
            var oldPile = before.Pile.ToArray();
            var oldRank = before.RoundRank;
            var oldActive = before.ActivePlayerId;
            var oldRevision = before.Revision;
            var result = e.Execute(player, c);
            Assert.That(result.Error, Is.EqualTo(error));
            Assert.That(result.Status, Is.EqualTo(error == CommandError.None ? CommandStatus.Applied : CommandStatus.Rejected));
            if (error != CommandError.None) Assert.That(e.State, Is.SameAs(before));
            else Assert.That(e.State.Revision, Is.EqualTo(before.Revision + 1));
            Assert.That(before.Hands.SelectMany(h => h).Concat(before.Pile).Concat(before.RemovedCards), Is.EqualTo(oldCards));
            Assert.That(before.Pile, Is.EqualTo(oldPile));
            Assert.That(before.RoundRank, Is.EqualTo(oldRank));
            Assert.That(before.ActivePlayerId, Is.EqualTo(oldActive));
            Assert.That(before.Revision, Is.EqualTo(oldRevision));
            Assert.That(result.Revision, Is.EqualTo(e.State.Revision));
            Invariants(e.State);
            return result;
        }

        private static void Play(GameEngine e, int p, CardRank? rank, params Card[] cards) =>
            Send(e, p, new GameCommand(Guid.NewGuid(), e.State.Revision, CommandKind.Play, cards, rank));
        private static void Doubt(GameEngine e, int p, CommandError error = CommandError.None) =>
            Send(e, p, new GameCommand(Guid.NewGuid(), e.State.Revision, CommandKind.Doubt), error);
        private static void Discard(GameEngine e, int p, CommandError error = CommandError.None) =>
            Send(e, p, new GameCommand(Guid.NewGuid(), e.State.Revision, CommandKind.Discard), error);
        private static void Resolve(GameEngine e)
        {
            var revision = e.State.Revision;
            Assert.That(e.ResolveRevelation(revision).Status, Is.EqualTo(CommandStatus.Applied));
            Assert.That(e.State.Revision, Is.EqualTo(revision + 1));
            Invariants(e.State);
        }
        private static GameEngine Open(bool honest = false, int players = 3)
        {
            var a = C(CardRank.Seven); var b = C(honest ? CardRank.Seven : CardRank.Three, CardSuit.Hearts);
            var e = Game(players, new[] { a, b, C(CardRank.Ace) }, new[] { C(CardRank.Four), C(CardRank.Nine), C(CardRank.Queen) });
            Play(e, 0, CardRank.Seven, a, b);
            return e;
        }

        [TestCase(TestName = "C-PLA-01")]
        public void OpeningSetsRankAndPassesTurn()
        {
            var e = Game(3, new[] { C(CardRank.Four), C(CardRank.Nine), C(CardRank.Ace) });
            Play(e, 0, CardRank.Queen, C(CardRank.Four), C(CardRank.Nine));
            Assert.That(e.State.Pile.Count, Is.EqualTo(2));
            Assert.That(e.State.RoundRank, Is.EqualTo(CardRank.Queen));
            Assert.That(e.State.ActivePlayerId, Is.EqualTo(1));
            Assert.That(e.State.RemainingTurnSeconds, Is.EqualTo(45));
            Assert.That(e.State.LastPlay.RespondentId, Is.EqualTo(1));
            Assert.That(e.State.LastPlay.IsDoubtWindowOpen, Is.True);
        }
        [TestCase(TestName = "C-PLA-02")]
        public void OpeningCanAnnounceAbsentRank()
        {
            var e = Game(3, new[] { C(CardRank.Four), C(CardRank.Nine) });
            Play(e, 0, CardRank.King, C(CardRank.Four));
            Assert.That(e.State.RoundRank, Is.EqualTo(CardRank.King));
        }
        [TestCase(TestName = "C-PLA-03")]
        public void WrongRoundAnnouncementIsRejected()
        {
            var e = Open();
            Send(e, 1, new GameCommand(Guid.NewGuid(), e.State.Revision, CommandKind.Play,
                new[] { C(CardRank.Four) }, CardRank.Five), CommandError.AnnouncementMustMatchRound);
        }
        [TestCase(TestName = "C-PLA-04")]
        public void MixedRanksAreLegalBluff()
        {
            var e = Game(3, new[] { C(CardRank.Two), C(CardRank.Three), C(CardRank.Ace) },
                new[] { C(CardRank.Four), C(CardRank.Nine), C(CardRank.Queen), C(CardRank.King) });
            Play(e, 0, CardRank.Queen, C(CardRank.Two), C(CardRank.Three));
            Play(e, 1, null, C(CardRank.Four), C(CardRank.Nine), C(CardRank.Queen));
            Assert.That(e.State.Pile.Count, Is.EqualTo(5));
            Assert.That(e.State.LastPlay.AnnouncedRank, Is.EqualTo(CardRank.Queen));
            Assert.That(e.State.LastPlay.Cards.Count, Is.EqualTo(3));
        }
        [TestCase(0, TestName = "C-PLA-05")]
        [TestCase(5, TestName = "C-PLA-06")]
        public void InvalidCardCount(int count)
        {
            var e = GameSetup.Create(new GameConfig(3), new SystemRandomSource(12));
            var engine = new GameEngine(e);
            Send(engine, e.ActivePlayerId, new GameCommand(Guid.NewGuid(), 0, CommandKind.Play,
                e.Hands[e.ActivePlayerId].Take(count), CardRank.Ace), CommandError.InvalidCardCount);
        }
        [TestCase(TestName = "C-PLA-07")]
        public void ForeignCardIsRejected()
        {
            var e = Open();
            Send(e, 1, new GameCommand(Guid.NewGuid(), e.State.Revision, CommandKind.Play,
                new[] { C(CardRank.Ace) }), CommandError.CardNotInHand);
        }
        [TestCase(TestName = "C-PLA-08")]
        public void RepeatedCardIsRejected()
        {
            var e = Open();
            Send(e, 1, new GameCommand(Guid.NewGuid(), e.State.Revision, CommandKind.Play,
                new[] { C(CardRank.Four), C(CardRank.Four) }), CommandError.DuplicateCard);
        }
        [TestCase(TestName = "C-PLA-09")]
        public void InactivePlayerIsRejected()
        {
            var e = Open();
            Send(e, 2, new GameCommand(Guid.NewGuid(), e.State.Revision, CommandKind.Play,
                e.State.Hands[2].Take(1)), CommandError.NotYourTurn);
        }
        [TestCase(TestName = "C-PLA-10")]
        public void DuplicateMessageIsIgnoredEvenWithOldRevision()
        {
            var e = Open();
            var c = new GameCommand(Guid.NewGuid(), e.State.Revision, CommandKind.Play, new[] { C(CardRank.Four) });
            Send(e, 1, c);
            var before = e.State;
            Assert.That(e.Execute(1, c).Status, Is.EqualTo(CommandStatus.Ignored));
            Assert.That(e.State, Is.SameAs(before));
            Invariants(e.State);
        }
        [TestCase(TestName = "C-PLA-11")]
        public void EmptyHandCreatesPendingVictory()
        {
            var e = Pending(false);
            Assert.That(e.State.PendingWinnerId, Is.EqualTo(1));
            Assert.That(e.State.WinnerId, Is.Null);
            Assert.That(e.State.Phase, Is.EqualTo(GamePhase.Playing));
        }

        [TestCase(TestName = "C-DOU-01")]
        public void FalseDoubtCollectsAndSkipsDoubter()
        {
            var e = Open(true);
            var count = e.State.Hands[1].Count;
            Doubt(e, 1);
            Assert.That(e.State.Revelation.IsDoubtCorrect, Is.False);
            Assert.That(e.State.Pile.Count, Is.EqualTo(2));
            Assert.That(e.State.Hands[1].Count, Is.EqualTo(count));
            Resolve(e);
            Assert.That(e.State.Hands[1].Count, Is.EqualTo(count + 2));
            AssertEmptyRound(e, 2);
        }
        [TestCase(TestName = "C-DOU-02")]
        public void CorrectDoubtReturnsPileToLiar()
        {
            var e = Open();
            Doubt(e, 1);
            Assert.That(e.State.Revelation.IsDoubtCorrect, Is.True);
            Resolve(e);
            Assert.That(e.State.Hands[0].Count, Is.EqualTo(3));
            AssertEmptyRound(e, 1);
        }
        [TestCase(TestName = "C-DOU-03")]
        public void OnlyLastPlayRevealedButEntirePileCollected()
        {
            var first = new[] { C(CardRank.King), C(CardRank.King, CardSuit.Hearts), C(CardRank.King, CardSuit.Spades), C(CardRank.King, CardSuit.Diamonds) };
            var last = new[] { C(CardRank.Eight), C(CardRank.Two) };
            var e = Game(3, first.Concat(new[] { C(CardRank.Ace) }).ToArray(), last.Concat(new[] { C(CardRank.Three) }).ToArray());
            Play(e, 0, CardRank.King, first);
            Play(e, 1, null, last);
            Doubt(e, 2);
            Assert.That(e.State.Revelation.Cards, Is.EqualTo(last));
            Assert.That(e.State.Pile.Count, Is.EqualTo(6));
            Resolve(e);
            Assert.That(e.State.Hands[1].Count, Is.EqualTo(7));
            AssertEmptyRound(e, 2);
        }
        [TestCase(TestName = "C-DOU-04")]
        public void FalseDoubtInDuelReturnsTurnToAccused()
        {
            var e = Open(true, 2);
            var count = e.State.Hands[1].Count;
            Doubt(e, 1); Resolve(e);
            Assert.That(e.State.Hands[1].Count, Is.EqualTo(count + 2));
            AssertEmptyRound(e, 0);
        }
        [TestCase(TestName = "C-DOU-05")]
        public void EmptyPileCannotBeDoubted() => Doubt(Game(), 0, CommandError.NoDoubtablePlay);
        [TestCase(TestName = "C-DOU-06")]
        public void InactiveDoubterIsRejected() => Doubt(Open(), 2, CommandError.NotYourTurn);
        [TestCase(TestName = "C-DOU-07")]
        public void DoubtBeforeAnyConfirmedPlayIsAccepted()
        {
            var e = Open(); Doubt(e, 1);
            Assert.That(e.State.Phase, Is.EqualTo(GamePhase.Revelation));
        }
        [TestCase(TestName = "C-DOU-08")]
        public void DoubtAfterConfirmedPlayIsRejected()
        {
            var e = Open(); Play(e, 1, null, C(CardRank.Four));
            Doubt(e, 1, CommandError.NotYourTurn);
        }
        [TestCase(TestName = "C-DOU-10")]
        public void RevelationPausesTimerAndRejectsEveryAction()
        {
            var e = Open(); Doubt(e, 1);
            Assert.That(e.State.IsTimerPaused, Is.True);
            foreach (int p in e.State.TurnOrder)
            foreach (CommandKind kind in Enum.GetValues(typeof(CommandKind)))
                Send(e, p, new GameCommand(Guid.NewGuid(), e.State.Revision, kind,
                    e.State.Hands[p].Take(1)), CommandError.RevelationInProgress);
        }
        [TestCase(TestName = "C-DOU-11")]
        public void CorrectDoubterCanReopenWithAnyRank()
        {
            foreach (CardRank rank in Enum.GetValues(typeof(CardRank)))
            {
                var e = Open(); Doubt(e, 1); Resolve(e);
                Play(e, 1, rank, C(CardRank.Four));
                Assert.That(e.State.RoundRank, Is.EqualTo(rank));
            }
        }

        [TestCase(TestName = "C-DEF-01")]
        public void DiscardRemovesEntirePileAndPassesTurn()
        {
            var e = Game(4, new[] { C(CardRank.Ace), C(CardRank.Two), C(CardRank.Three), C(CardRank.Four), C(CardRank.Five) },
                new[] { C(CardRank.Six), C(CardRank.Seven), C(CardRank.Eight), C(CardRank.Ten) });
            Play(e, 0, CardRank.Nine, e.State.Hands[0].Take(4).ToArray());
            Play(e, 1, null, e.State.Hands[1].Take(3).ToArray());
            var pile = e.State.Pile.ToArray();
            var hands = e.State.Hands.SelectMany(h => h).ToArray();
            Discard(e, 2);
            Assert.That(e.State.RemovedCards, Is.EqualTo(pile));
            Assert.That(e.State.Hands.SelectMany(h => h), Is.EqualTo(hands));
            Assert.That(e.State.Revelation, Is.Null);
            AssertEmptyRound(e, 3);
        }
        [TestCase(TestName = "C-DEF-02")]
        public void EmptyPileCannotBeDiscarded() => Discard(Game(), 0, CommandError.EmptyPile);
        [TestCase(TestName = "C-DEF-03")]
        public void DiscardClosesDoubt()
        {
            var e = Open(); Discard(e, 1);
            Assert.That(e.State.LastPlay, Is.Null);
            Doubt(e, 2, CommandError.NoDoubtablePlay);
        }
        [TestCase(TestName = "C-DEF-04")]
        public void PendingVictoryCannotBeDiscarded() => Discard(Pending(false), 2, CommandError.PendingVictory);
        [TestCase(TestName = "C-DEF-05")]
        public void DiscardConservesCards()
        {
            var e = Open(); Discard(e, 1);
            Assert.That(e.State.Hands.Sum(h => h.Count) + e.State.RemovedCards.Count, Is.EqualTo(52));
        }
        private static void AssertEmptyRound(GameEngine e, int active)
        {
            Assert.That(e.State.Pile, Is.Empty);
            Assert.That(e.State.RoundRank, Is.Null);
            Assert.That(e.State.LastPlay, Is.Null);
            Assert.That(e.State.Revelation, Is.Null);
            Assert.That(e.State.ActivePlayerId, Is.EqualTo(active));
            Assert.That(e.State.RemainingTurnSeconds, Is.EqualTo(e.State.Config.TurnDurationSeconds));
            Assert.That(e.State.IsTimerPaused, Is.False);
        }
        private static GameEngine Pending(bool honest)
        {
            var e = Game(3, new[] { C(CardRank.Seven), C(CardRank.Ace) },
                new[] { C(honest ? CardRank.Seven : CardRank.Three, CardSuit.Hearts) });
            Play(e, 0, CardRank.Seven, C(CardRank.Seven));
            Play(e, 1, null, e.State.Hands[1].ToArray());
            return e;
        }
        [TestCase(TestName = "C-VIC-01")]
        public void HonestPendingWinnerSurvivesDoubt()
        {
            var e = Pending(true); var count = e.State.Hands[2].Count;
            Doubt(e, 2); Resolve(e);
            Assert.That(e.State.WinnerId, Is.EqualTo(1));
            Assert.That(e.State.Phase, Is.EqualTo(GamePhase.Finished));
            Assert.That(e.State.Hands[2].Count, Is.EqualTo(count + 2));
        }
        [TestCase(TestName = "C-VIC-02")]
        public void LyingPendingWinnerCollectsAndContinues()
        {
            var e = Pending(false); Doubt(e, 2); Resolve(e);
            Assert.That(e.State.PendingWinnerId, Is.Null);
            Assert.That(e.State.WinnerId, Is.Null);
            Assert.That(e.State.Hands[1].Count, Is.EqualTo(2));
            AssertEmptyRound(e, 2);
        }
        [TestCase(TestName = "C-VIC-03")]
        public void ConfirmingPlayEndsGameWithoutApplyingCards()
        {
            var e = Pending(false); var before = e.State;
            Play(e, 2, null, e.State.Hands[2].First());
            Assert.That(e.State.WinnerId, Is.EqualTo(1));
            Assert.That(e.State.Pile, Is.EqualTo(before.Pile));
            Assert.That(e.State.Hands.SelectMany(h => h), Is.EqualTo(before.Hands.SelectMany(h => h)));
        }
        [TestCase(TestName = "C-VIC-08")]
        public void FinishedGameRejectsAllActions()
        {
            var e = Pending(true); Play(e, 2, null, e.State.Hands[2].First());
            foreach (int p in e.State.TurnOrder)
            foreach (CommandKind kind in Enum.GetValues(typeof(CommandKind)))
                Send(e, p, new GameCommand(Guid.NewGuid(), e.State.Revision, kind), CommandError.GameFinished);
        }

        [Test]
        public void StaleCommandsCannotChangeState()
        {
            var e = Open();
            foreach (CommandKind kind in Enum.GetValues(typeof(CommandKind)))
                Send(e, 1, new GameCommand(Guid.NewGuid(), e.State.Revision - 1, kind,
                    new[] { C(CardRank.Four) }), CommandError.StaleRevision);
        }

        [Test]
        public void RejectedCommandIdCannotBeRetriedWithChangedPayload()
        {
            var e = Open(); var id = Guid.NewGuid();
            Send(e, 1, new GameCommand(id, e.State.Revision - 1, CommandKind.Doubt), CommandError.StaleRevision);
            var before = e.State;
            Assert.That(e.Execute(1, new GameCommand(id, e.State.Revision, CommandKind.Discard)).Status,
                Is.EqualTo(CommandStatus.Ignored));
            Assert.That(e.State, Is.SameAs(before));
            Invariants(e.State);
        }

        [Test]
        public void CommandIdsAreScopedToAuthenticatedPlayer()
        {
            var e = Open(); var id = Guid.NewGuid();
            Send(e, 0, new GameCommand(id, e.State.Revision, CommandKind.Discard), CommandError.NotYourTurn);
            Send(e, 1, new GameCommand(id, e.State.Revision, CommandKind.Discard));
        }

        [Test]
        public void RevelationAndDiscardDuplicatesCannotApplyTwice()
        {
            foreach (var kind in new[] { CommandKind.Doubt, CommandKind.Discard })
            {
                var e = Open();
                var command = new GameCommand(Guid.NewGuid(), e.State.Revision, kind);
                Send(e, 1, command);
                if (kind == CommandKind.Doubt) Resolve(e);
                var before = e.State;
                Assert.That(e.Execute(1, command).Status, Is.EqualTo(CommandStatus.Ignored));
                Assert.That(e.State, Is.SameAs(before));
                Invariants(e.State);
            }
        }

        [Test]
        public void ClosedWindowCannotBeDoubtedEvenByOriginalRespondent()
        {
            var e = Open();
            // Simulates the state produced by a future host event; no timeout/departure rule here.
            var state = new GameState(e.State);
            state.LastPlay = state.LastPlay.CloseDoubtWindow();
            var closed = new GameEngine(state);
            Doubt(closed, 1, CommandError.NoDoubtablePlay);
            Discard(closed, 1);
        }

        [Test]
        public void HostResolutionIsGuardedAndCannotRunTwice()
        {
            var e = Open(); var before = e.State;
            Assert.That(e.ResolveRevelation(before.Revision).Error, Is.EqualTo(CommandError.NoRevelation));
            Assert.That(e.State, Is.SameAs(before)); Invariants(e.State);
            Doubt(e, 1); before = e.State;
            Assert.That(e.ResolveRevelation(before.Revision - 1).Error, Is.EqualTo(CommandError.StaleRevision));
            Assert.That(e.State, Is.SameAs(before)); Invariants(e.State);
            Resolve(e); before = e.State;
            Assert.That(e.ResolveRevelation(before.Revision).Error, Is.EqualTo(CommandError.NoRevelation));
            Assert.That(e.State, Is.SameAs(before)); Invariants(e.State);
        }

        [Test]
        public void ConfiguredFullTimerIsRestoredOnEveryNewTurnAndCorrectDoubt()
        {
            // Use a full deck from setup, then simulate time remaining without implementing a clock.
            var e = new GameEngine(GameSetup.Create(new GameConfig(3, 60), new SystemRandomSource(2)));
            int p = e.State.ActivePlayerId;
            var card = e.State.Hands[p].First();
            e.State.RemainingTurnSeconds = 7;
            Play(e, p, card.Rank == CardRank.Ace ? CardRank.King : CardRank.Ace, card);
            Assert.That(e.State.RemainingTurnSeconds, Is.EqualTo(60));
            p = e.State.ActivePlayerId;
            e.State.RemainingTurnSeconds = 9;
            Doubt(e, p);
            Assert.That(e.State.RemainingTurnSeconds, Is.EqualTo(9));
            Assert.That(e.State.IsTimerPaused, Is.True);
            Resolve(e);
            Assert.That(e.State.ActivePlayerId, Is.EqualTo(p));
            Assert.That(e.State.RemainingTurnSeconds, Is.EqualTo(60));
        }

        [Test]
        public void InvalidOpeningAndMalformedCommandsAreRejected()
        {
            var e = new GameEngine(GameSetup.Create(new GameConfig(3), new SystemRandomSource(2)));
            int p = e.State.ActivePlayerId;
            var cards = e.State.Hands[p].Take(1).ToArray();
            Send(e, p, new GameCommand(Guid.NewGuid(), 0, CommandKind.Play, cards), CommandError.OpeningAnnouncementRequired);
            Send(e, p, new GameCommand(Guid.NewGuid(), 0, CommandKind.Play, cards, (CardRank)99), CommandError.InvalidAnnouncement);
            Send(e, p, new GameCommand(Guid.NewGuid(), 0, (CommandKind)99), CommandError.InvalidCommand);
            Send(e, p, new GameCommand(Guid.Empty, 0, CommandKind.Discard), CommandError.InvalidCommand);
            Send(e, p, null, CommandError.InvalidCommand);
        }
    }
}
