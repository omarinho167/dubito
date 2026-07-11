#include "Misc/AutomationTest.h"
#include "DubitoRules.h"
#include "DubitoMatchState.h"
#include "DubitoAnnouncement.h"
#include "DubitoCard.h"
#include "DubitoHand.h"
#include "DubitoConstants.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace
{
	constexpr EAutomationTestFlags DubitoLegalityFlags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	FDubitoHand MakeClubs(int32 Count)
	{
		FDubitoHand Hand;
		for (int32 Rank = 1; Rank <= Count; ++Rank)
		{
			Hand.Add(FDubitoCard(EDubitoSuit::Clubs, Rank));
		}
		return Hand;
	}

	// Two-player state with both hands dealt; active player is 10.
	FDubitoMatchState MakeTwoPlayerState()
	{
		TMap<int32, FDubitoHand> Hands;
		Hands.Add(10, MakeClubs(5));
		Hands.Add(20, MakeClubs(5));
		FDubitoMatchState State;
		DubitoRules::SetupMatch(State, { 10, 20 }, Hands);
		return State;
	}
}

// --- Action Matrix availability -------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDubitoActionMatrixTest, "Dubito.Core.Legality.ActionMatrix", DubitoLegalityFlags)
bool FDubitoActionMatrixTest::RunTest(const FString&)
{
	using namespace DubitoRules;

	// My turn, empty pile: Play only.
	FDubitoMatchState S = MakeTwoPlayerState();
	TestTrue(TEXT("Empty pile: active can Play"), CanPlay(S, 10));
	TestFalse(TEXT("Empty pile: active cannot Doubt"), CanDoubt(S, 10));
	TestFalse(TEXT("Empty pile: active cannot Discard"), CanDiscard(S, 10));
	TestFalse(TEXT("Non-active player cannot Play"), CanPlay(S, 20));
	TestFalse(TEXT("Non-active player cannot Doubt"), CanDoubt(S, 20));
	TestFalse(TEXT("Non-active player cannot Discard"), CanDiscard(S, 20));

	// Player 10 opens; turn passes to 20, round open, last play doubtable.
	ApplyPlay(S, 10, { FDubitoCard(EDubitoSuit::Clubs, 1) }, FDubitoAnnouncement(7, 1));
	TestTrue(TEXT("Round open: active can Play"), CanPlay(S, 20));
	TestTrue(TEXT("Round open + doubtable: active can Doubt"), CanDoubt(S, 20));
	TestTrue(TEXT("Round open, non-empty pile: active can Discard"), CanDiscard(S, 20));

	// No doubtable claim (e.g. claimant disconnected): Play + Discard, no Doubt.
	S.bLastPlayDoubtable = false;
	TestFalse(TEXT("No doubtable claim: cannot Doubt"), CanDoubt(S, 20));
	TestTrue(TEXT("No doubtable claim: can still Play"), CanPlay(S, 20));
	TestTrue(TEXT("No doubtable claim: can still Discard"), CanDiscard(S, 20));

	// Pending win: Doubt emphasized, Discard blocked, Play available.
	S.bLastPlayDoubtable = true;
	S.PendingWinnerId = 10;
	TestTrue(TEXT("Pending win: active can Doubt"), CanDoubt(S, 20));
	TestTrue(TEXT("Pending win: active can Play"), CanPlay(S, 20));
	TestFalse(TEXT("Pending win: Discard is blocked"), CanDiscard(S, 20));

	// Reveal / Game over: everything off.
	S.PendingWinnerId = DubitoConstants::NoPlayerId;
	S.Phase = EDubitoPhase::DoubtReveal;
	TestFalse(TEXT("Reveal: cannot Play"), CanPlay(S, 20));
	TestFalse(TEXT("Reveal: cannot Doubt"), CanDoubt(S, 20));
	TestFalse(TEXT("Reveal: cannot Discard"), CanDiscard(S, 20));
	S.Phase = EDubitoPhase::GameOver;
	TestFalse(TEXT("Game over: cannot Play"), CanPlay(S, 20));

	return true;
}

// --- ValidatePlay contents boundaries -------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDubitoValidatePlayTest, "Dubito.Core.Legality.ValidatePlay", DubitoLegalityFlags)
bool FDubitoValidatePlayTest::RunTest(const FString&)
{
	using namespace DubitoRules;

	FDubitoMatchState S = MakeTwoPlayerState(); // active 10, round closed, holds clubs 1..5

	const TArray<FDubitoCard> TwoOwned = { FDubitoCard(EDubitoSuit::Clubs, 1), FDubitoCard(EDubitoSuit::Clubs, 2) };

	// Valid opening play.
	TestEqual(TEXT("Valid open play"), static_cast<int32>(ValidatePlay(S, 10, TwoOwned, FDubitoAnnouncement(7, 2))), static_cast<int32>(EDubitoPlayValidity::Valid));

	// Wrong caller.
	TestEqual(TEXT("Not your turn"), static_cast<int32>(ValidatePlay(S, 20, TwoOwned, FDubitoAnnouncement(7, 2))), static_cast<int32>(EDubitoPlayValidity::NotYourTurn));

	// Actual count out of range.
	TestEqual(TEXT("Zero actual cards"), static_cast<int32>(ValidatePlay(S, 10, {}, FDubitoAnnouncement(7, 1))), static_cast<int32>(EDubitoPlayValidity::BadActualCount));
	const TArray<FDubitoCard> FiveOwned = {
		FDubitoCard(EDubitoSuit::Clubs, 1), FDubitoCard(EDubitoSuit::Clubs, 2), FDubitoCard(EDubitoSuit::Clubs, 3),
		FDubitoCard(EDubitoSuit::Clubs, 4), FDubitoCard(EDubitoSuit::Clubs, 5)
	};
	TestEqual(TEXT("Five actual cards"), static_cast<int32>(ValidatePlay(S, 10, FiveOwned, FDubitoAnnouncement(7, 4))), static_cast<int32>(EDubitoPlayValidity::BadActualCount));

	// Malformed announcement (count 0).
	TestEqual(TEXT("Bad announcement count"), static_cast<int32>(ValidatePlay(S, 10, TwoOwned, FDubitoAnnouncement(7, 0))), static_cast<int32>(EDubitoPlayValidity::BadAnnouncement));

	// Card not owned.
	TestEqual(TEXT("Do not own the card"), static_cast<int32>(ValidatePlay(S, 10, { FDubitoCard(EDubitoSuit::Clubs, 11) }, FDubitoAnnouncement(7, 1))), static_cast<int32>(EDubitoPlayValidity::DontOwnCards));

	// Round open: claimed value must match the locked value.
	S.RoundValue = 7;
	S.ActualPile.Add(FDubitoCard(EDubitoSuit::Clubs, 1)); // pile non-empty
	TestEqual(TEXT("Value locked mismatch"), static_cast<int32>(ValidatePlay(S, 10, { FDubitoCard(EDubitoSuit::Clubs, 2) }, FDubitoAnnouncement(5, 1))), static_cast<int32>(EDubitoPlayValidity::ValueLocked));
	TestEqual(TEXT("Matching locked value is valid"), static_cast<int32>(ValidatePlay(S, 10, { FDubitoCard(EDubitoSuit::Clubs, 2) }, FDubitoAnnouncement(7, 1))), static_cast<int32>(EDubitoPlayValidity::Valid));

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
