using System;
using System.Linq;
using Dubito.Core;
using NUnit.Framework;

namespace Dubito.Tests.Core
{
    public partial class ActionTests
    {
        private static void Tick(GameEngine e, double seconds)
        {
            Assert.That(e.AdvanceTime(seconds).Error, Is.EqualTo(CommandError.None));
            Invariants(e.State);
        }
        private static void Leave(GameEngine e, int player)
        {
            Assert.That(e.RemovePlayer(player).Error, Is.EqualTo(CommandError.None));
            Invariants(e.State);
        }
        private static void Won(GameEngine e, int player)
        {
            Assert.That(e.State.WinnerId, Is.EqualTo(player));
            Assert.That(e.State.PendingWinnerId, Is.Null);
            Assert.That(e.State.Phase, Is.EqualTo(GamePhase.Finished));
        }

        [TestCase(TestName = "C-CHR-01")]
        public void TimeoutPreservesPileAndClosesWindow()
        {
            var e = Open(); var pile = e.State.Pile.ToArray(); Tick(e, 45);
            Assert.That(e.State.Pile, Is.EqualTo(pile));
            Assert.That(e.State.RoundRank, Is.EqualTo(CardRank.Seven));
            Assert.That(e.State.ActivePlayerId, Is.EqualTo(2));
            Assert.That(e.State.SkippedTurns[1], Is.EqualTo(1));
            Assert.That(e.State.LastPlay.IsDoubtWindowOpen, Is.False);
        }
        [Test]
        public void ViewExposesOwnSkippedTurnsForTheExclusionWarning()
        {
            var e = Open(); Tick(e, 45);
            Assert.That(PlayerView.Create(e.State, 1).OwnSkippedTurns, Is.EqualTo(1));
            Assert.That(PlayerView.Create(e.State, 2).OwnSkippedTurns, Is.EqualTo(0));
        }
        [TestCase(TestName = "C-CHR-02")]
        public void TimeoutOnEmptyPile() { var e = Game(); Tick(e, 45); AssertEmptyRound(e, 1); }
        [TestCase(TestName = "C-CHR-03")]
        public void SecondConsecutiveSkipExcludes()
        {
            var e = Open(); Tick(e, 45); Play(e, 2, null, e.State.Hands[2].First());
            Discard(e, 0);
            var hand = e.State.Hands[1].ToArray(); Tick(e, 45);
            Assert.That(e.State.TurnOrder, Is.EqualTo(new[] { 0, 2 }));
            Assert.That(e.State.Hands[1], Is.Empty);
            Assert.That(hand.All(c => e.State.RemovedCards.Contains(c)), Is.True);
            Assert.That(e.State.ActivePlayerId, Is.EqualTo(2));
        }
        [TestCase(TestName = "C-CHR-04")]
        public void ConfirmedActionResetsSkips()
        {
            var e = Open(); Tick(e, 45); Play(e, 2, null, e.State.Hands[2].First());
            Discard(e, 0); Play(e, 1, CardRank.Seven, C(CardRank.Four));
            Assert.That(e.State.SkippedTurns[1], Is.Zero);
            Play(e, 2, null, e.State.Hands[2].First());
            Discard(e, 0); Tick(e, 45);
            Assert.That(e.State.TurnOrder.Contains(1), Is.True);
            Assert.That(e.State.SkippedTurns[1], Is.EqualTo(1));
        }
        [TestCase(TestName = "C-CHR-05")]
        public void ElapsedTimeDuringRevelationHasNoEffect()
        { var e = Open(); Doubt(e, 1); var before = e.State; Tick(e, 900); Assert.That(e.State, Is.SameAs(before)); }
        [TestCase(TestName = "C-CHR-06")]
        public void CorrectDoubtResetsElapsedTimer()
        { var e = Open(); Tick(e, 35); Doubt(e, 1); Resolve(e); AssertEmptyRound(e, 1); }
        [TestCase(TestName = "C-CHR-07")]
        public void ActionBeforeExpirationIsAccepted()
        { var e = Open(); Tick(e, 44.9); Play(e, 1, CardRank.Seven, C(CardRank.Four)); Assert.That(e.State.RemainingTurnSeconds, Is.EqualTo(45)); }
        [Test]
        public void CountdownAloneDoesNotMakeCommandsStale()
        {
            // The host ticks time every frame: a command prepared before a tick must stay valid.
            var e = Open();
            var command = new GameCommand(Guid.NewGuid(), e.State.Revision, CommandKind.Play, new[] { C(CardRank.Four) });
            Tick(e, 10);
            Assert.That(e.State.RemainingTurnSeconds, Is.EqualTo(35));
            Send(e, 1, command);
        }
        [TestCase(TestName = "C-CHR-08")]
        public void ActionAfterExpirationIsRejected()
        { var e = Open(); Tick(e, 45); Send(e, 1, new GameCommand(Guid.NewGuid(), e.State.Revision, CommandKind.Play, new[] { C(CardRank.Four) }), CommandError.NotYourTurn); }
        [TestCase(TestName = "C-DOU-09")]
        public void NextPlayerCannotDoubtAfterSkip()
        { var e = Open(); Tick(e, 45); Doubt(e, 2, CommandError.NoDoubtablePlay); }
        [TestCase(TestName = "C-DOU-12")]
        public void WindowNeverReopensAfterFullCycle()
        { var e = Open(false, 2); Tick(e, 45); Tick(e, 45); Doubt(e, 1, CommandError.NoDoubtablePlay); }
        [TestCase(TestName = "C-DOU-13")]
        public void DepartedRespondentClosesWindow()
        { var e = Open(); Leave(e, 1); Doubt(e, 2, CommandError.NoDoubtablePlay); }
        [TestCase(TestName = "C-VIC-04")]
        public void PendingVictoryConfirmedByTimeout() { var e = Pending(false); Tick(e, 45); Won(e, 1); }
        [TestCase(TestName = "C-VIC-05")]
        public void PendingVictoryConfirmedByRespondentDeparture() { var e = Pending(false); Leave(e, 2); Won(e, 1); }
        [TestCase(false, TestName = "C-VIC-06")]
        [TestCase(true, TestName = "C-VIC-09")]
        public void DepartedPendingWinnerIsAutomaticallyRevealed(bool honest)
        {
            var e = Pending(honest); var cards = e.State.LastPlay.Cards; Leave(e, 1);
            Assert.That(e.State.Phase, Is.EqualTo(GamePhase.Revelation));
            Assert.That(e.State.Revelation.IsAutomatic, Is.True);
            Assert.That(e.State.Revelation.Cards, Is.EqualTo(cards));
            var before = e.State; Tick(e, 300); Assert.That(e.State, Is.SameAs(before));
            Doubt(e, 2, CommandError.RevelationInProgress); Resolve(e);
            Assert.That(e.State.TurnOrder.Contains(1), Is.False);
            if (honest) Won(e, 1);
            else { AssertEmptyRound(e, 2); Assert.That(e.State.RemovedCards.Count, Is.EqualTo(2)); Assert.That(e.State.PendingWinnerId, Is.Null); }
        }
        private static GameEngine DuelPending(bool honest)
        {
            var card = C(CardRank.Seven);
            var e = Game(2, new[] { card }); Play(e, 0, honest ? CardRank.Seven : CardRank.Ace, card);
            return e;
        }
        [TestCase(false, 1, TestName = "C-VIC-10")]
        [TestCase(true, 0, TestName = "C-VIC-11")]
        public void AutomaticVerdictPrecedesLastSurvivor(bool honest, int winner)
        { var e = DuelPending(honest); Leave(e, 0); Assert.That(e.State.WinnerId, Is.Null); Resolve(e); Won(e, winner); }
        [TestCase(false, TestName = "C-VIC-12")]
        [TestCase(true, TestName = "C-VIC-13")]
        public void RespondentLeavesDuringAutomaticRevelation(bool honest)
        {
            var e = Game(6, new[] { C(CardRank.Seven) }); Play(e, 0, honest ? CardRank.Seven : CardRank.Ace, C(CardRank.Seven));
            Leave(e, 0); Leave(e, 1); Assert.That(e.State.WinnerId, Is.Null); Resolve(e);
            if (honest) Won(e, 0); else { AssertEmptyRound(e, 2); Assert.That(e.State.PendingWinnerId, Is.Null); }
            Assert.That(e.State.TurnOrder, Is.EqualTo(new[] { 2, 3, 4, 5 }));
        }
        [TestCase(TestName = "C-VIC-07")]
        public void LastSurvivorWins() { var e = Open(); Leave(e, 0); Leave(e, 1); Won(e, 2); }
        [TestCase(TestName = "C-VIC-14")]
        [TestCase(TestName = "C-DEC-06")]
        public void HostCanStopWithoutWinner()
        {
            var e = Pending(true); Assert.That(e.StopGame().Error, Is.EqualTo(CommandError.None));
            Assert.That(e.State.Phase, Is.EqualTo(GamePhase.Finished)); Assert.That(e.State.WinnerId, Is.Null);
            Assert.That(e.State.PendingWinnerId, Is.Null); Assert.That(e.State.Revelation, Is.Null); Invariants(e.State);
        }
        [TestCase(TestName = "C-DEC-01")]
        public void ActiveDeparturePreservesPile()
        {
            var e = Open(); var pile = e.State.Pile.ToArray(); var hand = e.State.Hands[1].ToArray(); Tick(e, 20); Leave(e, 1);
            Assert.That(e.State.ActivePlayerId, Is.EqualTo(2)); Assert.That(e.State.RemainingTurnSeconds, Is.EqualTo(45));
            Assert.That(e.State.Pile, Is.EqualTo(pile)); Assert.That(e.State.LastPlay.IsDoubtWindowOpen, Is.False);
            Assert.That(e.State.RemovedCards, Is.EqualTo(hand)); Assert.That(e.State.Hands[1], Is.Empty);
        }
        [TestCase(TestName = "C-DEC-02")]
        public void AccusedDepartureClosesWindow()
        { var e = Open(); Leave(e, 0); Doubt(e, 1, CommandError.NoDoubtablePlay); Discard(e, 1); }
        [TestCase(TestName = "C-DEC-03")]
        public void UnrelatedDeparturePreservesTurnAndTimer()
        {
            var e = Open(); Tick(e, 10); var before = e.State; Leave(e, 2);
            Assert.That(e.State.ActivePlayerId, Is.EqualTo(1)); Assert.That(e.State.RemainingTurnSeconds, Is.EqualTo(35));
            Assert.That(e.State.Pile, Is.EqualTo(before.Pile)); Assert.That(e.State.LastPlay.IsDoubtWindowOpen, Is.True);
            Assert.That(e.State.RemovedCards, Is.EqualTo(before.Hands[2])); Assert.That(before.TurnOrder.Count, Is.EqualTo(3));
        }
        [TestCase(false, 0, 1, TestName = "C-DEC-04")]
        [TestCase(true, 1, 2, TestName = "C-DEC-05")]
        [TestCase(false, 1, 2, TestName = "C-DEC-08")]
        public void DepartureDuringDoubt(bool honest, int departing, int active)
        {
            var e = Open(honest); var before = e.State; Doubt(e, 1); Leave(e, departing);
            Assert.That(e.State.Phase, Is.EqualTo(GamePhase.Revelation)); Resolve(e); AssertEmptyRound(e, active);
            Assert.That(e.State.Hands[departing], Is.Empty); Assert.That(e.State.TurnOrder.Contains(departing), Is.False);
            if (!honest && departing == 1) Assert.That(e.State.Hands[0].Count, Is.EqualTo(3));
            else Assert.That(before.Pile.All(c => e.State.RemovedCards.Contains(c)), Is.True);
        }
        [TestCase(false, TestName = "C-DEC-09")]
        [TestCase(true, TestName = "C-DEC-10")]
        public void DoubterDepartureDoesNotOverridePendingVerdict(bool honest)
        {
            var e = Pending(honest); Doubt(e, 2); Leave(e, 2); Resolve(e);
            if (honest) Won(e, 1);
            else { AssertEmptyRound(e, 0); Assert.That(e.State.Hands[1].Count, Is.EqualTo(2)); Assert.That(e.State.PendingWinnerId, Is.Null); }
        }

        [TestCase(false, 0)]
        [TestCase(false, 1)]
        [TestCase(false, 2)]
        [TestCase(true, 0)]
        [TestCase(true, 1)]
        [TestCase(true, 2)]
        public void EveryDoubtDepartureTableCell(bool honest, int departing)
        {
            var e = Open(honest, 4); var before = e.State;
            Doubt(e, 1); Leave(e, departing); Resolve(e);
            int recipient = honest ? 1 : 0;
            int active = honest || departing == 1 ? 2 : 1;
            if (active == departing) active++;
            AssertEmptyRound(e, active);
            Assert.That(e.State.TurnOrder.Contains(departing), Is.False);
            Assert.That(e.State.Hands[departing], Is.Empty);
            if (recipient == departing)
                Assert.That(e.State.RemovedCards, Is.EquivalentTo(before.Hands[departing].Concat(before.Pile)));
            else
            {
                Assert.That(e.State.Hands[recipient], Is.EquivalentTo(before.Hands[recipient].Concat(before.Pile)));
                Assert.That(e.State.RemovedCards, Is.EquivalentTo(before.Hands[departing]));
            }
        }

        [TestCase(false, 0)]
        [TestCase(false, 1)]
        [TestCase(true, 0)]
        [TestCase(true, 1)]
        public void DuelDoubtVerdictPrecedesSurvivor(bool honest, int departing)
        {
            var e = DuelPending(honest); Doubt(e, 1); Leave(e, departing); Resolve(e);
            Won(e, honest ? 0 : 1 - departing);
            Assert.That(e.State.Hands[departing], Is.Empty);
        }

        [TestCase(false)]
        [TestCase(true)]
        public void MultipleDeparturesDuringDoubtAreAppliedAfterVerdict(bool honest)
        {
            var e = Open(honest, 4); var pile = e.State.Pile.ToArray(); Doubt(e, 1);
            Leave(e, 0); Leave(e, 1); Resolve(e); AssertEmptyRound(e, 2);
            Assert.That(e.State.TurnOrder, Is.EqualTo(new[] { 2, 3 }));
            Assert.That(pile.All(c => e.State.RemovedCards.Contains(c)), Is.True);
        }

        [Test]
        public void HostStopOverridesAutomaticRevelation()
        {
            var e = DuelPending(true); Leave(e, 0);
            Assert.That(e.StopGame().Error, Is.EqualTo(CommandError.None));
            Assert.That(e.State.Phase, Is.EqualTo(GamePhase.Finished));
            Assert.That(e.State.WinnerId, Is.Null); Assert.That(e.State.Revelation, Is.Null);
            Assert.That(e.ResolveRevelation(e.State.Revision).Error, Is.EqualTo(CommandError.GameFinished));
            Invariants(e.State);
        }

        [Test]
        public void ElapsedTimeAndDepartureSnapshotsRemainFrozen()
        {
            var e = Open(); var before = e.State; var hands = before.Hands.SelectMany(h => h).ToArray();
            Tick(e, 10); Assert.That(before.RemainingTurnSeconds, Is.EqualTo(45));
            Assert.That(e.State, Is.Not.SameAs(before)); // new snapshot, same revision (countdown only)
            Assert.That(before.Revision, Is.EqualTo(e.State.Revision));
            Leave(e, 2); Assert.That(before.TurnOrder, Is.EqualTo(new[] { 0, 1, 2 }));
            Assert.That(before.Hands.SelectMany(h => h), Is.EqualTo(hands));
            Assert.That(before.RemovedCards, Is.Empty); Assert.That(before.SkippedTurns, Is.EqualTo(new[] { 0, 0, 0 }));
            Doubt(e, 1); before = e.State; Leave(e, 0);
            Assert.That(before.PendingDepartures, Is.Empty); Resolve(e);
            Assert.That(before.TurnOrder, Is.EqualTo(new[] { 0, 1 }));
            Assert.That(before.Pile.Count, Is.EqualTo(2));
        }

        [Test]
        public void InvalidHostEventsAndDuplicateDeparturesHaveNoEffect()
        {
            var e = Open(); var before = e.State;
            foreach (double seconds in new[] { -1, double.NaN, double.PositiveInfinity, double.NegativeInfinity })
            {
                Assert.That(e.AdvanceTime(seconds).Error, Is.EqualTo(CommandError.InvalidElapsedTime));
                Assert.That(e.State, Is.SameAs(before)); Invariants(e.State);
            }
            foreach (int p in new[] { -1, 3 })
            {
                Assert.That(e.RemovePlayer(p).Error, Is.EqualTo(CommandError.PlayerNotInGame));
                Assert.That(e.State, Is.SameAs(before)); Invariants(e.State);
            }
            Doubt(e, 1); Leave(e, 0); before = e.State;
            Assert.That(e.RemovePlayer(0).Error, Is.EqualTo(CommandError.PlayerNotInGame));
            Assert.That(e.State, Is.SameAs(before)); Resolve(e); before = e.State;
            Assert.That(e.RemovePlayer(0).Error, Is.EqualTo(CommandError.PlayerNotInGame));
            Assert.That(e.State, Is.SameAs(before)); Invariants(e.State);
        }

        [TestCase(CommandKind.Doubt)]
        [TestCase(CommandKind.Discard)]
        public void EveryConfirmedActionResetsInactivity(CommandKind kind)
        {
            var e = Open(); Tick(e, 45); Tick(e, 45); Tick(e, 45);
            Assert.That(e.State.SkippedTurns[1], Is.EqualTo(1));
            Doubt(e, 1, CommandError.NoDoubtablePlay);
            Assert.That(e.State.SkippedTurns[1], Is.EqualTo(1));
            // Create a new legal doubtable play while preserving the skip counter.
            var s = new GameState(e.State);
            s.LastPlay = new LastPlay(0, 1, CardRank.Seven, s.Pile);
            e = new GameEngine(s);
            Send(e, 1, new GameCommand(Guid.NewGuid(), s.Revision, kind));
            Assert.That(e.State.SkippedTurns[1], Is.Zero);
        }

        [Test]
        public void LargeElapsedDeltaSpansTurnsAndStopsAtWinner()
        {
            var e = Open(); Tick(e, 100);
            Assert.That(e.State.ActivePlayerId, Is.EqualTo(0));
            Assert.That(e.State.RemainingTurnSeconds, Is.EqualTo(35));
            Assert.That(e.State.SkippedTurns, Is.EqualTo(new[] { 0, 1, 1 }));
            Tick(e, 1000); Assert.That(e.State.Phase, Is.EqualTo(GamePhase.Finished));
            Assert.That(e.State.TurnOrder.Count, Is.EqualTo(1)); Won(e, e.State.TurnOrder[0]);
        }
    }
}
