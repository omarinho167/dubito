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
	constexpr EAutomationTestFlags DubitoTimeoutFlags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	FDubitoCard C(EDubitoSuit Suit, int32 Rank) { return FDubitoCard(Suit, Rank); }

	// Builds a two-player state with the given turn order and hands.
	FDubitoMatchState MakeState(const TArray<int32>& Order, const TArray<FDubitoCard>& First, const TArray<FDubitoCard>& Second)
	{
		TMap<int32, FDubitoHand> Hands;
		FDubitoHand A; A.Cards = First;
		FDubitoHand B; B.Cards = Second;
		Hands.Add(Order[0], A);
		Hands.Add(Order[1], B);
		FDubitoMatchState State;
		DubitoRules::SetupMatch(State, Order, Hands);
		return State;
	}
}

// --- Discard removes the pile, clears the round, and skips the turn ---------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDubitoDiscardTest, "Dubito.Core.Discard.Apply", DubitoTimeoutFlags)
bool FDubitoDiscardTest::RunTest(const FString&)
{
	// 10 opens; 20 then discards the pile.
	FDubitoMatchState S = MakeState({ 10, 20 }, { C(EDubitoSuit::Clubs, 7), C(EDubitoSuit::Clubs, 2) }, { C(EDubitoSuit::Spades, 3), C(EDubitoSuit::Spades, 4) });
	DubitoRules::ApplyPlay(S, 10, { C(EDubitoSuit::Clubs, 7) }, FDubitoAnnouncement(7, 1));

	const int32 Hand10Before = S.Hands[10].Num();
	const int32 Hand20Before = S.Hands[20].Num();

	DubitoRules::ApplyDiscard(S, 20);

	TestTrue(TEXT("Discard empties the pile"), S.IsPileEmpty());
	TestFalse(TEXT("Discard clears the round value"), S.IsRoundOpen());
	TestEqual(TEXT("Claimed pile ledger reset"), S.ClaimedPileCount, 0);
	TestEqual(TEXT("Discarding player skips the turn: next active is 10"), S.ActivePlayerId(), 10);
	TestEqual(TEXT("Hands are not affected by discard (10)"), S.Hands[10].Num(), Hand10Before);
	TestEqual(TEXT("Hands are not affected by discard (20)"), S.Hands[20].Num(), Hand20Before);
	return true;
}

// --- Timeout on an open round with a matching card: truthful auto-play ------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDubitoTimeoutTruthfulTest, "Dubito.Core.Timeout.Truthful", DubitoTimeoutFlags)
bool FDubitoTimeoutTruthfulTest::RunTest(const FString&)
{
	// 20 opens value 7; then 10 (holding a 7) times out.
	FDubitoMatchState S = MakeState({ 20, 10 }, { C(EDubitoSuit::Spades, 7), C(EDubitoSuit::Spades, 3) }, { C(EDubitoSuit::Hearts, 7), C(EDubitoSuit::Clubs, 2) });
	DubitoRules::ApplyPlay(S, 20, { C(EDubitoSuit::Spades, 7) }, FDubitoAnnouncement(7, 1));

	DubitoRules::ResolveTimeout(S, 10);

	TestEqual(TEXT("Auto-play claims count 1"), S.LastAnnouncement.ClaimedCount, 1);
	TestEqual(TEXT("Auto-play claims the locked value"), S.LastAnnouncement.ClaimedValue, 7);
	TestEqual(TEXT("Auto-play used exactly one card"), S.LastActualCards.Num(), 1);
	TestEqual(TEXT("Auto-played card is truthfully the round value"), S.LastActualCards[0].Rank, 7);
	TestEqual(TEXT("Timeout streak incremented"), S.ConsecutiveTimeouts[10], 1);
	return true;
}

// --- Timeout on an open round without a matching card: forced minimal bluff -
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDubitoTimeoutBluffTest, "Dubito.Core.Timeout.ForcedBluff", DubitoTimeoutFlags)
bool FDubitoTimeoutBluffTest::RunTest(const FString&)
{
	// 20 opens value 7; 10 holds no 7 and must bluff.
	FDubitoMatchState S = MakeState({ 20, 10 }, { C(EDubitoSuit::Spades, 7), C(EDubitoSuit::Spades, 3) }, { C(EDubitoSuit::Clubs, 2), C(EDubitoSuit::Clubs, 3) });
	DubitoRules::ApplyPlay(S, 20, { C(EDubitoSuit::Spades, 7) }, FDubitoAnnouncement(7, 1));

	DubitoRules::ResolveTimeout(S, 10);

	TestEqual(TEXT("Forced bluff claims the locked value"), S.LastAnnouncement.ClaimedValue, 7);
	TestEqual(TEXT("Forced bluff claims count 1"), S.LastAnnouncement.ClaimedCount, 1);
	TestEqual(TEXT("Forced bluff plays one card"), S.LastActualCards.Num(), 1);
	TestNotEqual(TEXT("Auto-played card is NOT the round value (a bluff)"), S.LastActualCards[0].Rank, 7);
	return true;
}

// --- Timeout on an empty pile: truthfully opens the round ------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDubitoTimeoutOpenTest, "Dubito.Core.Timeout.OpensTruthfully", DubitoTimeoutFlags)
bool FDubitoTimeoutOpenTest::RunTest(const FString&)
{
	// Fresh state, empty pile; active player 10 times out and opens on their first card.
	FDubitoMatchState S = MakeState({ 10, 20 }, { C(EDubitoSuit::Clubs, 5), C(EDubitoSuit::Clubs, 2) }, { C(EDubitoSuit::Spades, 9), C(EDubitoSuit::Spades, 8) });

	DubitoRules::ResolveTimeout(S, 10);

	TestEqual(TEXT("Opening timeout locks the round to the played card's value"), S.RoundValue, 5);
	TestEqual(TEXT("Opening timeout claims that value"), S.LastAnnouncement.ClaimedValue, 5);
	TestEqual(TEXT("Opening timeout claims count 1"), S.LastAnnouncement.ClaimedCount, 1);
	TestEqual(TEXT("Opening timeout is truthful"), S.LastActualCards[0].Rank, 5);
	return true;
}

// --- Timeout during a pending-win response confirms the pending winner ------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDubitoTimeoutPendingWinTest, "Dubito.Core.Timeout.PendingWinConfirms", DubitoTimeoutFlags)
bool FDubitoTimeoutPendingWinTest::RunTest(const FString&)
{
	FDubitoMatchState S = MakeState({ 10, 20 }, { C(EDubitoSuit::Clubs, 5) }, { C(EDubitoSuit::Spades, 9), C(EDubitoSuit::Spades, 8) });
	// 10 is a pending winner; 20 is the responder whose turn it is.
	S.PendingWinnerId = 10;
	S.ActiveSeatIndex = 1; // player 20 responds

	DubitoRules::ResolveTimeout(S, 20);

	TestEqual(TEXT("Pending winner is confirmed on responder timeout"), S.WinnerId, 10);
	TestEqual(TEXT("Match is over"), static_cast<int32>(S.Phase), static_cast<int32>(EDubitoPhase::GameOver));
	return true;
}

// --- Third consecutive timeout disconnects the player ----------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDubitoTimeoutDisconnectTest, "Dubito.Core.Timeout.ThirdIsDisconnect", DubitoTimeoutFlags)
bool FDubitoTimeoutDisconnectTest::RunTest(const FString&)
{
	// 20 opens; 10 is on its second consecutive timeout already and has two cards.
	FDubitoMatchState S = MakeState({ 20, 10 }, { C(EDubitoSuit::Spades, 7), C(EDubitoSuit::Spades, 3) }, { C(EDubitoSuit::Clubs, 2), C(EDubitoSuit::Clubs, 3) });
	DubitoRules::ApplyPlay(S, 20, { C(EDubitoSuit::Spades, 7) }, FDubitoAnnouncement(7, 1));
	S.ConsecutiveTimeouts[10] = 2;

	DubitoRules::ResolveTimeout(S, 10); // becomes the 3rd timeout

	TestFalse(TEXT("Disconnected player is removed from the turn order"), S.TurnOrder.Contains(10));
	TestFalse(TEXT("Disconnected player's hand is removed"), S.Hands.Contains(10));
	TestEqual(TEXT("Only player left (20) wins"), S.WinnerId, 20);
	TestEqual(TEXT("Match is over"), static_cast<int32>(S.Phase), static_cast<int32>(EDubitoPhase::GameOver));
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
