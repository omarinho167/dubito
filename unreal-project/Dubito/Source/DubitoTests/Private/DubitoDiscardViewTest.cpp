#include "Misc/AutomationTest.h"
#include "DubitoConstants.h"
#include "DubitoDiscardView.h"
#include "DubitoMatchState.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace
{
	constexpr EAutomationTestFlags DubitoDiscardFlags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	// Active player's turn on a non-empty pile with no pending win: the discardable case.
	FDubitoDiscardInputs MakeDiscardableInputs()
	{
		FDubitoDiscardInputs In;
		In.Phase = EDubitoPhase::PlayerTurn;
		In.bIsLocalPlayerActive = true;
		In.ClaimedPileCount = 3;
		In.bHasPendingWin = false;
		return In;
	}
}

// --- The available case mirrors DubitoRules::CanDiscard ---------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDubitoDiscardAvailableTest, "Dubito.Unreal.Discard.Available", DubitoDiscardFlags)
bool FDubitoDiscardAvailableTest::RunTest(const FString&)
{
	FDubitoDiscardView View = BuildDubitoDiscardView(MakeDiscardableInputs());
	TestTrue(TEXT("Discard available on my turn, non-empty pile, no pending win"), View.bCanDiscard);
	TestEqual(TEXT("No blocked reason when available"), View.BlockedReason, EDubitoDiscardBlockedReason::None);
	return true;
}

// --- Blocked states each explain themselves --------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDubitoDiscardBlockedTest, "Dubito.Unreal.Discard.BlockedReasons", DubitoDiscardFlags)
bool FDubitoDiscardBlockedTest::RunTest(const FString&)
{
	// Not my turn.
	FDubitoDiscardInputs NotMine = MakeDiscardableInputs();
	NotMine.bIsLocalPlayerActive = false;
	FDubitoDiscardView NotMineView = BuildDubitoDiscardView(NotMine);
	TestFalse(TEXT("Not my turn: cannot discard"), NotMineView.bCanDiscard);
	TestEqual(TEXT("Not-my-turn reason"), NotMineView.BlockedReason, EDubitoDiscardBlockedReason::NotYourTurn);

	// Wrong phase (a reveal is not a discard window) reads as not-your-turn too.
	FDubitoDiscardInputs Reveal = MakeDiscardableInputs();
	Reveal.Phase = EDubitoPhase::DoubtReveal;
	TestEqual(TEXT("Reveal phase blocks as not-your-turn"),
		BuildDubitoDiscardView(Reveal).BlockedReason, EDubitoDiscardBlockedReason::NotYourTurn);

	// Empty pile: nothing to discard.
	FDubitoDiscardInputs Empty = MakeDiscardableInputs();
	Empty.ClaimedPileCount = 0;
	FDubitoDiscardView EmptyView = BuildDubitoDiscardView(Empty);
	TestFalse(TEXT("Empty pile: cannot discard"), EmptyView.bCanDiscard);
	TestEqual(TEXT("Empty-pile reason"), EmptyView.BlockedReason, EDubitoDiscardBlockedReason::EmptyPile);

	// Pending win takes precedence over a non-empty pile.
	FDubitoDiscardInputs Pending = MakeDiscardableInputs();
	Pending.bHasPendingWin = true;
	FDubitoDiscardView PendingView = BuildDubitoDiscardView(Pending);
	TestFalse(TEXT("Pending win: cannot discard"), PendingView.bCanDiscard);
	TestEqual(TEXT("Pending-win reason"), PendingView.BlockedReason, EDubitoDiscardBlockedReason::PendingWin);

	// Not-my-turn is resolved before pending-win / empty-pile.
	FDubitoDiscardInputs NotMineEmpty = MakeDiscardableInputs();
	NotMineEmpty.bIsLocalPlayerActive = false;
	NotMineEmpty.ClaimedPileCount = 0;
	TestEqual(TEXT("Turn blocker wins over empty pile"),
		BuildDubitoDiscardView(NotMineEmpty).BlockedReason, EDubitoDiscardBlockedReason::NotYourTurn);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
