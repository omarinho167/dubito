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
	constexpr EAutomationTestFlags DubitoClaimsFlags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	// Builds a hand of `Count` distinct Clubs cards with ranks 1..Count.
	FDubitoHand MakeClubsHand(int32 Count)
	{
		FDubitoHand Hand;
		for (int32 Rank = 1; Rank <= Count; ++Rank)
		{
			Hand.Add(FDubitoCard(EDubitoSuit::Clubs, Rank));
		}
		return Hand;
	}
}

// --- Announcement validity and round-value lock ---------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDubitoClaimValidityTest, "Dubito.Core.Claim.Validity", DubitoClaimsFlags)
bool FDubitoClaimValidityTest::RunTest(const FString&)
{
	using namespace DubitoRules;

	// Opening an empty pile: any well-formed value/count is legal.
	TestTrue(TEXT("Opening with value 7 count 1 is legal"), IsAnnouncementValidForRound(FDubitoAnnouncement(7, 1), DubitoConstants::NoRoundValue));
	TestTrue(TEXT("Opening with value 13 count 4 is legal"), IsAnnouncementValidForRound(FDubitoAnnouncement(13, 4), DubitoConstants::NoRoundValue));

	// Count must be within 1..4.
	TestFalse(TEXT("Count 0 is illegal"), IsAnnouncementValidForRound(FDubitoAnnouncement(7, 0), DubitoConstants::NoRoundValue));
	TestFalse(TEXT("Count 5 is illegal"), IsAnnouncementValidForRound(FDubitoAnnouncement(7, 5), DubitoConstants::NoRoundValue));

	// Value must be a real rank 1..13.
	TestFalse(TEXT("Value 0 is illegal"), IsAnnouncementValidForRound(FDubitoAnnouncement(0, 1), DubitoConstants::NoRoundValue));
	TestFalse(TEXT("Value 14 is illegal"), IsAnnouncementValidForRound(FDubitoAnnouncement(14, 1), DubitoConstants::NoRoundValue));

	// Round open: claimed value is locked to the round value; count is still free.
	TestTrue(TEXT("Matching locked value is legal"), IsAnnouncementValidForRound(FDubitoAnnouncement(7, 3), 7));
	TestFalse(TEXT("Non-matching locked value is illegal"), IsAnnouncementValidForRound(FDubitoAnnouncement(5, 1), 7));

	return true;
}

// --- Setup initialises public ledgers from actual hand sizes ---------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDubitoSetupLedgerTest, "Dubito.Core.Ledger.Setup", DubitoClaimsFlags)
bool FDubitoSetupLedgerTest::RunTest(const FString&)
{
	TMap<int32, FDubitoHand> Hands;
	Hands.Add(10, MakeClubsHand(5));
	Hands.Add(20, MakeClubsHand(5));

	FDubitoMatchState State;
	DubitoRules::SetupMatch(State, { 10, 20 }, Hands);

	TestEqual(TEXT("Phase is PlayerTurn"), static_cast<int32>(State.Phase), static_cast<int32>(EDubitoPhase::PlayerTurn));
	TestEqual(TEXT("Active player is first in turn order"), State.ActivePlayerId(), 10);
	TestFalse(TEXT("Round starts closed"), State.IsRoundOpen());
	TestTrue(TEXT("Pile starts empty"), State.IsPileEmpty());
	TestEqual(TEXT("Public ledger P10 equals dealt size"), State.PublicHandCounts[10], 5);
	TestEqual(TEXT("Public ledger P20 equals dealt size"), State.PublicHandCounts[20], 5);
	TestEqual(TEXT("Claimed pile ledger starts at zero"), State.ClaimedPileCount, 0);

	return true;
}

// --- Ledgers track CLAIMED counts, never the hidden actual counts ----------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDubitoLedgerHidesActualTest, "Dubito.Core.Ledger.HidesActual", DubitoClaimsFlags)
bool FDubitoLedgerHidesActualTest::RunTest(const FString&)
{
	TMap<int32, FDubitoHand> Hands;
	Hands.Add(10, MakeClubsHand(5)); // ranks 1..5 of Clubs
	Hands.Add(20, MakeClubsHand(5));

	FDubitoMatchState State;
	DubitoRules::SetupMatch(State, { 10, 20 }, Hands);

	// Player 10 plays 4 real cards but claims only 1, opening the round on value 7.
	const TArray<FDubitoCard> Actual = {
		FDubitoCard(EDubitoSuit::Clubs, 1),
		FDubitoCard(EDubitoSuit::Clubs, 2),
		FDubitoCard(EDubitoSuit::Clubs, 3),
		FDubitoCard(EDubitoSuit::Clubs, 4)
	};
	DubitoRules::ApplyPlay(State, 10, Actual, FDubitoAnnouncement(7, 1));

	// Hidden actual state reflects the real 4 cards.
	TestEqual(TEXT("Actual pile grew by the real count (4)"), State.ActualPile.Num(), 4);
	TestEqual(TEXT("Actual hand shrank by the real count"), State.Hands[10].Num(), 1);

	// Public ledgers reflect only the claimed count (1) — they do NOT expose the 4.
	TestEqual(TEXT("Claimed pile ledger grew by the claimed count (1)"), State.ClaimedPileCount, 1);
	TestEqual(TEXT("Public hand ledger dropped by the claimed count (1)"), State.PublicHandCounts[10], 4);

	// Round locked to the claimed value; last play recorded and doubtable; turn advanced.
	TestEqual(TEXT("Round value locked to claimed value"), State.RoundValue, 7);
	TestEqual(TEXT("Last claimant recorded"), State.LastClaimantId, 10);
	TestTrue(TEXT("Last play is doubtable"), State.bLastPlayDoubtable);
	TestEqual(TEXT("Turn advanced to next player"), State.ActivePlayerId(), 20);

	return true;
}

// --- The divergence also runs the other way (few real, many claimed) -------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDubitoLedgerBluffUpTest, "Dubito.Core.Ledger.BluffUp", DubitoClaimsFlags)
bool FDubitoLedgerBluffUpTest::RunTest(const FString&)
{
	TMap<int32, FDubitoHand> Hands;
	Hands.Add(10, MakeClubsHand(2)); // only 2 cards, public ledger starts at 2
	Hands.Add(20, MakeClubsHand(5));

	FDubitoMatchState State;
	DubitoRules::SetupMatch(State, { 10, 20 }, Hands);

	// Player 10 plays 1 real card but claims 4.
	const TArray<FDubitoCard> Actual = { FDubitoCard(EDubitoSuit::Clubs, 1) };
	DubitoRules::ApplyPlay(State, 10, Actual, FDubitoAnnouncement(9, 4));

	TestEqual(TEXT("Actual pile grew by the real count (1)"), State.ActualPile.Num(), 1);
	TestEqual(TEXT("Claimed pile ledger grew by the claimed count (4)"), State.ClaimedPileCount, 4);

	// Public hand ledger would go 2 - 4 = -2; it clamps at zero.
	TestEqual(TEXT("Public hand ledger clamps at zero"), State.PublicHandCounts[10], 0);
	// The actual hand only lost the one card really played.
	TestEqual(TEXT("Actual hand shrank by the real count (1)"), State.Hands[10].Num(), 1);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
