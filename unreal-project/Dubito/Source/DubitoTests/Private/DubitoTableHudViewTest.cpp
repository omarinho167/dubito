#include "Misc/AutomationTest.h"
#include "DubitoAnnouncement.h"
#include "DubitoConstants.h"
#include "DubitoMatchState.h"
#include "DubitoTableHudView.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace
{
	constexpr EAutomationTestFlags DubitoTableHudFlags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	// A default active turn, local player 10, no prior claim, empty pile.
	FDubitoTableHudInputs MakeActiveTurnInputs()
	{
		FDubitoTableHudInputs In;
		In.Phase = EDubitoPhase::PlayerTurn;
		In.ActivePlayerId = 10;
		In.LocalPlayerId = 10;
		In.LocalHandCount = 5;
		In.RoundValue = DubitoConstants::NoRoundValue;
		In.ClaimedPileCount = 0;
		return In;
	}
}

// --- Whose turn ------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDubitoTableHudTurnTest, "Dubito.Unreal.TableHud.WhoseTurn", DubitoTableHudFlags)
bool FDubitoTableHudTurnTest::RunTest(const FString&)
{
	FDubitoTableHudInputs In = MakeActiveTurnInputs();
	FDubitoTableHudSnapshot Local = BuildDubitoTableHudSnapshot(In);
	TestTrue(TEXT("Match reads as in progress on a player turn"), Local.bMatchInProgress);
	TestEqual(TEXT("Active player id passes through"), Local.ActivePlayerId, 10);
	TestTrue(TEXT("Local player is flagged active when it is their turn"), Local.bIsLocalPlayerActive);

	In.LocalPlayerId = 20;
	FDubitoTableHudSnapshot Remote = BuildDubitoTableHudSnapshot(In);
	TestFalse(TEXT("Local player is not active when someone else has the turn"), Remote.bIsLocalPlayerActive);

	In.LocalPlayerId = DubitoConstants::NoPlayerId;
	FDubitoTableHudSnapshot Unknown = BuildDubitoTableHudSnapshot(In);
	TestFalse(TEXT("No local id is never treated as active"), Unknown.bIsLocalPlayerActive);
	return true;
}

// --- Action availability mirrors the server legality predicates ------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDubitoTableHudActionsTest, "Dubito.Unreal.TableHud.ActionMatrix", DubitoTableHudFlags)
bool FDubitoTableHudActionsTest::RunTest(const FString&)
{
	// Empty pile, my turn, no claim: Play only (like DubitoRules::CanPlay).
	FDubitoTableHudInputs In = MakeActiveTurnInputs();
	FDubitoTableHudSnapshot Empty = BuildDubitoTableHudSnapshot(In);
	TestTrue(TEXT("Empty pile: can attempt Play"), Empty.bCanAttemptPlay);
	TestFalse(TEXT("Empty pile: cannot Doubt"), Empty.bCanAttemptDoubt);
	TestFalse(TEXT("Empty pile: cannot Discard"), Empty.bCanAttemptDiscard);

	// Round open, a doubtable claim, non-empty pile: Play + Doubt + Discard.
	In.RoundValue = 7;
	In.ClaimedPileCount = 3;
	In.bHasPreviousPublicClaim = true;
	In.bPreviousClaimDoubtable = true;
	In.PreviousClaimantId = 20;
	In.PreviousPublicAnnouncement = FDubitoAnnouncement(7, 3);
	FDubitoTableHudSnapshot Open = BuildDubitoTableHudSnapshot(In);
	TestTrue(TEXT("Round open: can Play"), Open.bCanAttemptPlay);
	TestTrue(TEXT("Doubtable claim: can Doubt"), Open.bCanAttemptDoubt);
	TestTrue(TEXT("Non-empty pile: can Discard"), Open.bCanAttemptDiscard);

	// Pending win: Play and (final) Doubt stay, Discard is forbidden while a win is pending.
	FDubitoTableHudInputs Pending = In;
	Pending.bHasPendingWin = true;
	FDubitoTableHudSnapshot PendingSnap = BuildDubitoTableHudSnapshot(Pending);
	TestTrue(TEXT("Pending win: can still Play"), PendingSnap.bCanAttemptPlay);
	TestTrue(TEXT("Pending win: can still Doubt"), PendingSnap.bCanAttemptDoubt);
	TestFalse(TEXT("Pending win: cannot Discard"), PendingSnap.bCanAttemptDiscard);

	// Stale claim (visible but not doubtable): Doubt is unavailable.
	FDubitoTableHudInputs Stale = In;
	Stale.bPreviousClaimDoubtable = false;
	FDubitoTableHudSnapshot StaleSnap = BuildDubitoTableHudSnapshot(Stale);
	TestTrue(TEXT("Stale claim is still shown"), StaleSnap.bHasPublicClaim);
	TestFalse(TEXT("Stale claim is not doubtable"), StaleSnap.bClaimIsDoubtable);
	TestFalse(TEXT("Stale claim: cannot Doubt"), StaleSnap.bCanAttemptDoubt);

	// Not my turn: nothing is available.
	FDubitoTableHudInputs NotMine = In;
	NotMine.LocalPlayerId = 99;
	FDubitoTableHudSnapshot NotMineSnap = BuildDubitoTableHudSnapshot(NotMine);
	TestFalse(TEXT("Not my turn: cannot Play"), NotMineSnap.bCanAttemptPlay);
	TestFalse(TEXT("Not my turn: cannot Doubt"), NotMineSnap.bCanAttemptDoubt);
	TestFalse(TEXT("Not my turn: cannot Discard"), NotMineSnap.bCanAttemptDiscard);

	// Empty hand cannot Play even on your turn.
	FDubitoTableHudInputs NoCards = MakeActiveTurnInputs();
	NoCards.LocalHandCount = 0;
	TestFalse(TEXT("Empty hand: cannot Play"), BuildDubitoTableHudSnapshot(NoCards).bCanAttemptPlay);

	// Actions are unavailable outside an active turn (e.g. doubt reveal phase).
	FDubitoTableHudInputs Reveal = In;
	Reveal.Phase = EDubitoPhase::DoubtReveal;
	FDubitoTableHudSnapshot RevealSnap = BuildDubitoTableHudSnapshot(Reveal);
	TestFalse(TEXT("Reveal phase: cannot Play"), RevealSnap.bCanAttemptPlay);
	TestFalse(TEXT("Reveal phase: cannot Doubt"), RevealSnap.bCanAttemptDoubt);
	TestFalse(TEXT("Reveal phase: cannot Discard"), RevealSnap.bCanAttemptDiscard);
	return true;
}

// --- Claim + stake presentation --------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDubitoTableHudClaimTest, "Dubito.Unreal.TableHud.ClaimPresentation", DubitoTableHudFlags)
bool FDubitoTableHudClaimTest::RunTest(const FString&)
{
	FDubitoTableHudInputs In = MakeActiveTurnInputs();
	In.ActivePlayerId = 20;
	In.LocalPlayerId = 20;
	In.RoundValue = 7;
	In.ClaimedPileCount = 4;
	In.bHasPreviousPublicClaim = true;
	In.bPreviousClaimDoubtable = true;
	In.PreviousClaimantId = 10;
	In.PreviousPublicAnnouncement = FDubitoAnnouncement(7, 2);

	FDubitoTableHudSnapshot Snap = BuildDubitoTableHudSnapshot(In);
	TestTrue(TEXT("Round value flagged present"), Snap.bHasRoundValue);
	TestEqual(TEXT("Round value passes through"), Snap.RoundValue, 7);
	TestTrue(TEXT("Public claim present"), Snap.bHasPublicClaim);
	TestEqual(TEXT("Claimant id passes through"), Snap.ClaimantId, 10);
	TestEqual(TEXT("Claimed count passes through"), Snap.PublicClaim.ClaimedCount, 2);
	TestEqual(TEXT("Claimed value passes through"), Snap.PublicClaim.ClaimedValue, 7);
	TestEqual(TEXT("Public stake passes through"), Snap.ClaimedPileCount, 4);

	// A claimant id of NoPlayerId cancels the claim even if the flag is set.
	FDubitoTableHudInputs NoClaimant = In;
	NoClaimant.PreviousClaimantId = DubitoConstants::NoPlayerId;
	FDubitoTableHudSnapshot NoClaimantSnap = BuildDubitoTableHudSnapshot(NoClaimant);
	TestFalse(TEXT("No claimant means no public claim"), NoClaimantSnap.bHasPublicClaim);
	return true;
}

// --- Turn timer ------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDubitoTableHudTimerTest, "Dubito.Unreal.TableHud.TurnTimer", DubitoTableHudFlags)
bool FDubitoTableHudTimerTest::RunTest(const FString&)
{
	FDubitoTableHudInputs In = MakeActiveTurnInputs();
	In.NowServerTimeSeconds = 100.0;
	In.TurnDeadlineServerTimeSeconds = 142.4;
	FDubitoTableHudSnapshot Snap = BuildDubitoTableHudSnapshot(In);
	TestTrue(TEXT("Turn timer running"), Snap.bHasTurnTimer);
	TestEqual(TEXT("Remaining seconds ceil to whole seconds"), Snap.TurnSecondsRemaining, 43);

	// Past the deadline clamps to zero, never negative.
	FDubitoTableHudInputs Expired = In;
	Expired.NowServerTimeSeconds = 200.0;
	FDubitoTableHudSnapshot ExpiredSnap = BuildDubitoTableHudSnapshot(Expired);
	TestTrue(TEXT("Expired timer still shown as running"), ExpiredSnap.bHasTurnTimer);
	TestEqual(TEXT("Expired timer clamps to zero"), ExpiredSnap.TurnSecondsRemaining, 0);

	// No deadline set, or not an active turn: no timer.
	FDubitoTableHudInputs NoDeadline = MakeActiveTurnInputs();
	NoDeadline.TurnDeadlineServerTimeSeconds = 0.0;
	TestFalse(TEXT("No deadline: no timer"), BuildDubitoTableHudSnapshot(NoDeadline).bHasTurnTimer);

	FDubitoTableHudInputs Lobby = In;
	Lobby.Phase = EDubitoPhase::Lobby;
	TestFalse(TEXT("Lobby phase: no turn timer"), BuildDubitoTableHudSnapshot(Lobby).bHasTurnTimer);
	return true;
}

// --- Seat ledgers ----------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDubitoTableHudSeatsTest, "Dubito.Unreal.TableHud.SeatLedgers", DubitoTableHudFlags)
bool FDubitoTableHudSeatsTest::RunTest(const FString&)
{
	FDubitoTableHudInputs In = MakeActiveTurnInputs();
	In.ActivePlayerId = 20;
	In.LocalPlayerId = 10;

	FDubitoSeatLedgerInput SeatB;
	SeatB.PlayerId = 20;
	SeatB.SeatIndex = 1;
	SeatB.DisplayName = TEXT("Bob");
	SeatB.PublicHandCount = 4;

	FDubitoSeatLedgerInput SeatA;
	SeatA.PlayerId = 10;
	SeatA.SeatIndex = 0;
	SeatA.DisplayName = TEXT("Alice");
	SeatA.PublicHandCount = 6;

	// Add out of seat order to prove the builder sorts by seat index.
	In.Seats.Add(SeatB);
	In.Seats.Add(SeatA);

	FDubitoTableHudSnapshot Snap = BuildDubitoTableHudSnapshot(In);
	if (!TestEqual(TEXT("Both seats present"), Snap.Seats.Num(), 2))
	{
		return false;
	}

	TestEqual(TEXT("Seats sorted by seat index"), Snap.Seats[0].SeatIndex, 0);
	TestEqual(TEXT("First seat is Alice"), Snap.Seats[0].PlayerId, 10);
	TestTrue(TEXT("Alice is the local player"), Snap.Seats[0].bIsLocal);
	TestFalse(TEXT("Alice is not the active player"), Snap.Seats[0].bIsActive);
	TestEqual(TEXT("Alice public hand count"), Snap.Seats[0].PublicHandCount, 6);

	TestEqual(TEXT("Second seat is Bob"), Snap.Seats[1].PlayerId, 20);
	TestTrue(TEXT("Bob is the active player"), Snap.Seats[1].bIsActive);
	TestFalse(TEXT("Bob is not the local player"), Snap.Seats[1].bIsLocal);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
