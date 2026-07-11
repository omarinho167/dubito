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
	constexpr EAutomationTestFlags DubitoDisconnectFlags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	FDubitoCard C(EDubitoSuit Suit, int32 Rank) { return FDubitoCard(Suit, Rank); }

	// Builds a state from a turn order, giving every player a two-card filler hand.
	FDubitoMatchState MakeSeatedState(const TArray<int32>& Order)
	{
		TMap<int32, FDubitoHand> Hands;
		int32 SuitIndex = 0;
		for (const int32 Id : Order)
		{
			FDubitoHand Hand;
			Hand.Add(C(static_cast<EDubitoSuit>(SuitIndex % DubitoConstants::NumSuits), 5));
			Hand.Add(C(static_cast<EDubitoSuit>(SuitIndex % DubitoConstants::NumSuits), 6));
			Hands.Add(Id, Hand);
			++SuitIndex;
		}
		FDubitoMatchState State;
		DubitoRules::SetupMatch(State, Order, Hands);
		return State;
	}
}

// --- Non-active player disconnects: removed, active unchanged ---------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDubitoDisconnectNonActiveTest, "Dubito.Core.Disconnect.NonActive", DubitoDisconnectFlags)
bool FDubitoDisconnectNonActiveTest::RunTest(const FString&)
{
	FDubitoMatchState S = MakeSeatedState({ 10, 20, 30 }); // active 10
	DubitoRules::HandleDisconnect(S, 20);

	TestFalse(TEXT("Disconnected player removed from turn order"), S.TurnOrder.Contains(20));
	TestFalse(TEXT("Disconnected hand removed"), S.Hands.Contains(20));
	TestEqual(TEXT("Two players remain"), S.TurnOrder.Num(), 2);
	TestEqual(TEXT("Active player unchanged"), S.ActivePlayerId(), 10);
	return true;
}

// --- Active player disconnects: turn advances to the next player ------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDubitoDisconnectActiveTest, "Dubito.Core.Disconnect.Active", DubitoDisconnectFlags)
bool FDubitoDisconnectActiveTest::RunTest(const FString&)
{
	FDubitoMatchState S = MakeSeatedState({ 10, 20, 30 }); // active 10
	DubitoRules::HandleDisconnect(S, 10);

	TestFalse(TEXT("Active player removed"), S.TurnOrder.Contains(10));
	TestEqual(TEXT("Turn advances to the next player (20)"), S.ActivePlayerId(), 20);
	return true;
}

// --- Disconnected previous claimant: claim no longer doubtable --------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDubitoDisconnectClaimantTest, "Dubito.Core.Disconnect.ClaimUndoubtable", DubitoDisconnectFlags)
bool FDubitoDisconnectClaimantTest::RunTest(const FString&)
{
	FDubitoMatchState S = MakeSeatedState({ 10, 20, 30 });
	S.LastClaimantId = 20;
	S.bLastPlayDoubtable = true;
	S.ActiveSeatIndex = 2; // 30 is up to doubt

	DubitoRules::HandleDisconnect(S, 20);

	TestFalse(TEXT("Claim is no longer doubtable"), S.bLastPlayDoubtable);
	TestEqual(TEXT("Last claimant cleared"), S.LastClaimantId, DubitoConstants::NoPlayerId);
	return true;
}

// --- Last remaining player wins --------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDubitoDisconnectLastStandingTest, "Dubito.Core.Disconnect.LastStanding", DubitoDisconnectFlags)
bool FDubitoDisconnectLastStandingTest::RunTest(const FString&)
{
	FDubitoMatchState S = MakeSeatedState({ 10, 20 }); // active 10
	DubitoRules::HandleDisconnect(S, 20);

	TestEqual(TEXT("Only remaining player wins"), S.WinnerId, 10);
	TestEqual(TEXT("Match is over"), static_cast<int32>(S.Phase), static_cast<int32>(EDubitoPhase::GameOver));
	return true;
}

// --- Disconnect as the pending-win responder confirms the pending winner ----
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDubitoDisconnectResponderTest, "Dubito.Core.Disconnect.ResponderConfirmsWin", DubitoDisconnectFlags)
bool FDubitoDisconnectResponderTest::RunTest(const FString&)
{
	FDubitoMatchState S = MakeSeatedState({ 10, 20, 30 });
	S.PendingWinnerId = 10;
	S.ActiveSeatIndex = 1; // 20 is the responder

	DubitoRules::HandleDisconnect(S, 20);

	TestEqual(TEXT("Pending winner confirmed when responder disconnects"), S.WinnerId, 10);
	TestEqual(TEXT("Match is over"), static_cast<int32>(S.Phase), static_cast<int32>(EDubitoPhase::GameOver));
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
