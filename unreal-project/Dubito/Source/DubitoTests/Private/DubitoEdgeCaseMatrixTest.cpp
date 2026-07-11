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
	constexpr EAutomationTestFlags DubitoEdgeFlags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	FDubitoCard C(EDubitoSuit Suit, int32 Rank) { return FDubitoCard(Suit, Rank); }

	FDubitoHand H(std::initializer_list<FDubitoCard> Cards)
	{
		FDubitoHand Hand;
		for (const FDubitoCard& Card : Cards)
		{
			Hand.Add(Card);
		}
		return Hand;
	}

	FDubitoMatchState MakeState(const TArray<int32>& Order, const TMap<int32, FDubitoHand>& Hands)
	{
		FDubitoMatchState State;
		DubitoRules::SetupMatch(State, Order, Hands);
		return State;
	}

	TMap<int32, FDubitoHand> ThreePlayerHands()
	{
		TMap<int32, FDubitoHand> Hands;
		Hands.Add(10, H({ C(EDubitoSuit::Clubs, 7), C(EDubitoSuit::Clubs, 2), C(EDubitoSuit::Clubs, 3) }));
		Hands.Add(20, H({ C(EDubitoSuit::Spades, 7), C(EDubitoSuit::Spades, 2), C(EDubitoSuit::Spades, 3) }));
		Hands.Add(30, H({ C(EDubitoSuit::Hearts, 7), C(EDubitoSuit::Hearts, 2), C(EDubitoSuit::Hearts, 3) }));
		return Hands;
	}
}

// --- Turn/phase rejection boundaries from the edge-case matrix -------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDubitoEdgeTurnPhaseTest, "Dubito.Core.Edge.TurnPhaseBoundaries", DubitoEdgeFlags)
bool FDubitoEdgeTurnPhaseTest::RunTest(const FString&)
{
	using namespace DubitoRules;

	FDubitoMatchState S = MakeState({ 10, 20, 30 }, ThreePlayerHands());

	TestFalse(TEXT("Out-of-turn player cannot Play"), CanPlay(S, 20));
	TestFalse(TEXT("Out-of-turn player cannot Doubt"), CanDoubt(S, 20));
	TestFalse(TEXT("Out-of-turn player cannot Discard"), CanDiscard(S, 20));
	TestEqual(TEXT("Out-of-turn Play validates as NotYourTurn"),
		static_cast<int32>(ValidatePlay(S, 20, { C(EDubitoSuit::Spades, 7) }, FDubitoAnnouncement(7, 1))),
		static_cast<int32>(EDubitoPlayValidity::NotYourTurn));

	S.Phase = EDubitoPhase::DoubtReveal;
	TestFalse(TEXT("Reveal phase disables Play"), CanPlay(S, 10));
	TestFalse(TEXT("Reveal phase disables Doubt"), CanDoubt(S, 10));
	TestFalse(TEXT("Reveal phase disables Discard"), CanDiscard(S, 10));
	TestEqual(TEXT("Reveal phase Play validates as WrongPhase"),
		static_cast<int32>(ValidatePlay(S, 10, { C(EDubitoSuit::Clubs, 7) }, FDubitoAnnouncement(7, 1))),
		static_cast<int32>(EDubitoPlayValidity::WrongPhase));

	S.Phase = EDubitoPhase::GameOver;
	TestFalse(TEXT("Game over disables Play"), CanPlay(S, 10));
	TestFalse(TEXT("Game over disables Doubt"), CanDoubt(S, 10));
	TestFalse(TEXT("Game over disables Discard"), CanDiscard(S, 10));

	return true;
}

// --- Choosing Play or Discard closes the previous Doubt window -------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDubitoEdgeDoubtWindowForfeitTest, "Dubito.Core.Edge.DoubtWindowForfeit", DubitoEdgeFlags)
bool FDubitoEdgeDoubtWindowForfeitTest::RunTest(const FString&)
{
	using namespace DubitoRules;

	FDubitoMatchState PlayState = MakeState({ 10, 20, 30 }, ThreePlayerHands());
	ApplyPlay(PlayState, 10, { C(EDubitoSuit::Clubs, 7) }, FDubitoAnnouncement(7, 1));
	TestTrue(TEXT("Immediate next player can Doubt before acting"), CanDoubt(PlayState, 20));
	TestFalse(TEXT("Non-immediate player cannot Doubt"), CanDoubt(PlayState, 30));

	ApplyPlay(PlayState, 20, { C(EDubitoSuit::Spades, 7) }, FDubitoAnnouncement(7, 1));
	TestEqual(TEXT("New play replaces the doubtable claimant"), PlayState.LastClaimantId, 20);
	TestFalse(TEXT("Player who skipped Doubt cannot Doubt afterward"), CanDoubt(PlayState, 20));
	TestTrue(TEXT("Next player may Doubt the new claim"), CanDoubt(PlayState, 30));

	FDubitoMatchState DiscardState = MakeState({ 10, 20, 30 }, ThreePlayerHands());
	ApplyPlay(DiscardState, 10, { C(EDubitoSuit::Clubs, 7) }, FDubitoAnnouncement(7, 1));
	TestTrue(TEXT("Discarding player had a Doubt window"), CanDoubt(DiscardState, 20));

	ApplyDiscard(DiscardState, 20);
	TestFalse(TEXT("Discard closes the Doubt window"), DiscardState.bLastPlayDoubtable);
	TestEqual(TEXT("Discard clears previous claimant"), DiscardState.LastClaimantId, DubitoConstants::NoPlayerId);
	TestTrue(TEXT("Discard empties the pile"), DiscardState.IsPileEmpty());
	TestFalse(TEXT("Next player cannot Doubt after discard"), CanDoubt(DiscardState, 30));

	return true;
}

// --- Play/claim edge cases that belong to the pure rule model --------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDubitoEdgePlayClaimTest, "Dubito.Core.Edge.PlayClaimBoundaries", DubitoEdgeFlags)
bool FDubitoEdgePlayClaimTest::RunTest(const FString&)
{
	using namespace DubitoRules;

	TMap<int32, FDubitoHand> Hands;
	Hands.Add(10, H({ C(EDubitoSuit::Clubs, 2), C(EDubitoSuit::Clubs, 3) }));
	Hands.Add(20, H({ C(EDubitoSuit::Spades, 7), C(EDubitoSuit::Spades, 8) }));

	FDubitoMatchState S = MakeState({ 10, 20 }, Hands);
	TestEqual(TEXT("Claimed count may differ from actual count"),
		static_cast<int32>(ValidatePlay(S, 10, { C(EDubitoSuit::Clubs, 2) }, FDubitoAnnouncement(7, 4))),
		static_cast<int32>(EDubitoPlayValidity::Valid));

	S.RoundValue = 7;
	S.ActualPile.Add(C(EDubitoSuit::Diamonds, 7));
	TestEqual(TEXT("A non-matching actual card is legal under the locked claim value"),
		static_cast<int32>(ValidatePlay(S, 10, { C(EDubitoSuit::Clubs, 2) }, FDubitoAnnouncement(7, 1))),
		static_cast<int32>(EDubitoPlayValidity::Valid));
	TestEqual(TEXT("A mismatched claimed value is rejected while the round is locked"),
		static_cast<int32>(ValidatePlay(S, 10, { C(EDubitoSuit::Clubs, 2) }, FDubitoAnnouncement(5, 1))),
		static_cast<int32>(EDubitoPlayValidity::ValueLocked));

	FDubitoMatchState LedgerState = MakeState({ 10, 20 }, Hands);
	ApplyPlay(LedgerState, 10, { C(EDubitoSuit::Clubs, 2) }, FDubitoAnnouncement(7, 4));
	TestEqual(TEXT("Public hand ledger clamps at zero"), LedgerState.PublicHandCounts[10], 0);
	TestEqual(TEXT("Actual hand still contains the unplayed card"), LedgerState.Hands[10].Num(), 1);
	TestFalse(TEXT("Zero public ledger alone is not a pending win"), LedgerState.HasPendingWin());

	TMap<int32, FDubitoHand> PendingHands;
	PendingHands.Add(10, H({ C(EDubitoSuit::Clubs, 7) }));
	PendingHands.Add(20, H({ C(EDubitoSuit::Spades, 7), C(EDubitoSuit::Spades, 8) }));
	FDubitoMatchState PendingState = MakeState({ 10, 20 }, PendingHands);
	ApplyPlay(PendingState, 10, { C(EDubitoSuit::Clubs, 7) }, FDubitoAnnouncement(7, 1));
	TestEqual(TEXT("Pending win is driven by the actual empty hand"), PendingState.PendingWinnerId, 10);
	TestEqual(TEXT("Pending win responder is the next active player"), PendingState.ActivePlayerId(), 20);

	return true;
}

// --- Pending-win outcomes from the winning edge cases ----------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDubitoEdgePendingWinTest, "Dubito.Core.Edge.PendingWinOutcomes", DubitoEdgeFlags)
bool FDubitoEdgePendingWinTest::RunTest(const FString&)
{
	using namespace DubitoRules;

	TMap<int32, FDubitoHand> LyingHands;
	LyingHands.Add(10, H({ C(EDubitoSuit::Clubs, 3) }));
	LyingHands.Add(20, H({ C(EDubitoSuit::Spades, 7), C(EDubitoSuit::Spades, 8) }));
	FDubitoMatchState CorrectDoubtState = MakeState({ 10, 20 }, LyingHands);
	ApplyPlay(CorrectDoubtState, 10, { C(EDubitoSuit::Clubs, 3) }, FDubitoAnnouncement(7, 1));

	FDubitoRevealInfo CorrectReveal = ResolveDoubt(CorrectDoubtState, 20);
	TestTrue(TEXT("Correct final Doubt detects the lie"), CorrectReveal.bWasLie);
	TestEqual(TEXT("Pending winner takes the pile back"), CorrectReveal.LoserId, 10);
	TestEqual(TEXT("Pending win is cancelled"), CorrectDoubtState.PendingWinnerId, DubitoConstants::NoPlayerId);
	TestEqual(TEXT("No winner is confirmed after correct final Doubt"), CorrectDoubtState.WinnerId, DubitoConstants::NoPlayerId);
	TestEqual(TEXT("Game remains in PlayerTurn after correct final Doubt"),
		static_cast<int32>(CorrectDoubtState.Phase),
		static_cast<int32>(EDubitoPhase::PlayerTurn));
	TestEqual(TEXT("Doubter plays next after correct final Doubt"), CorrectDoubtState.ActivePlayerId(), 20);

	TMap<int32, FDubitoHand> HonestHands;
	HonestHands.Add(10, H({ C(EDubitoSuit::Clubs, 7) }));
	HonestHands.Add(20, H({ C(EDubitoSuit::Spades, 7), C(EDubitoSuit::Spades, 8) }));
	FDubitoMatchState WrongDoubtState = MakeState({ 10, 20 }, HonestHands);
	ApplyPlay(WrongDoubtState, 10, { C(EDubitoSuit::Clubs, 7) }, FDubitoAnnouncement(7, 1));

	FDubitoRevealInfo WrongReveal = ResolveDoubt(WrongDoubtState, 20);
	TestFalse(TEXT("Wrong final Doubt sees an honest play"), WrongReveal.bWasLie);
	TestTrue(TEXT("Wrong final Doubt marks the win as confirmed"), WrongReveal.bWinConfirmed);
	TestEqual(TEXT("Wrong final Doubt confirms the pending winner"), WrongDoubtState.WinnerId, 10);
	TestEqual(TEXT("Wrong final Doubt ends the game"),
		static_cast<int32>(WrongDoubtState.Phase),
		static_cast<int32>(EDubitoPhase::GameOver));

	FDubitoMatchState PlayInsteadState = MakeState({ 10, 20 }, HonestHands);
	ApplyPlay(PlayInsteadState, 10, { C(EDubitoSuit::Clubs, 7) }, FDubitoAnnouncement(7, 1));
	const int32 ResponderHandBefore = PlayInsteadState.Hands[20].Num();
	ApplyPlay(PlayInsteadState, 20, { C(EDubitoSuit::Spades, 7) }, FDubitoAnnouncement(7, 1));
	TestEqual(TEXT("Playing instead of Doubting confirms the pending winner"), PlayInsteadState.WinnerId, 10);
	TestEqual(TEXT("Responder's attempted play does not happen after win confirmation"), PlayInsteadState.Hands[20].Num(), ResponderHandBefore);

	return true;
}

// --- Timer edge cases: stale callbacks, Doubt decline, voluntary reset -----
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDubitoEdgeTimeoutTest, "Dubito.Core.Edge.TimeoutBoundaries", DubitoEdgeFlags)
bool FDubitoEdgeTimeoutTest::RunTest(const FString&)
{
	using namespace DubitoRules;

	TMap<int32, FDubitoHand> Hands;
	Hands.Add(10, H({ C(EDubitoSuit::Clubs, 7), C(EDubitoSuit::Clubs, 2) }));
	Hands.Add(20, H({ C(EDubitoSuit::Spades, 7), C(EDubitoSuit::Spades, 2) }));

	FDubitoMatchState StaleState = MakeState({ 10, 20 }, Hands);
	ResolveTimeout(StaleState, 20);
	TestEqual(TEXT("Out-of-turn timeout leaves active player unchanged"), StaleState.ActivePlayerId(), 10);
	TestTrue(TEXT("Out-of-turn timeout does not create a pile"), StaleState.IsPileEmpty());
	TestEqual(TEXT("Out-of-turn timeout does not record a claimant"), StaleState.LastClaimantId, DubitoConstants::NoPlayerId);

	StaleState.Phase = EDubitoPhase::DoubtReveal;
	ResolveTimeout(StaleState, 10);
	TestTrue(TEXT("Reveal-phase timeout is ignored"), StaleState.IsPileEmpty());
	TestEqual(TEXT("Reveal-phase timeout does not remove cards"), StaleState.Hands[10].Num(), 2);

	FDubitoMatchState DoubtWindowState = MakeState({ 20, 10 }, Hands);
	ApplyPlay(DoubtWindowState, 20, { C(EDubitoSuit::Spades, 7) }, FDubitoAnnouncement(7, 1));
	TestTrue(TEXT("Timeout player had a Doubt window"), CanDoubt(DoubtWindowState, 10));
	ResolveTimeout(DoubtWindowState, 10);
	TestEqual(TEXT("Timeout declines Doubt and records the timed-out player as claimant"), DoubtWindowState.LastClaimantId, 10);
	TestEqual(TEXT("Timeout auto-play claims count 1"), DoubtWindowState.LastAnnouncement.ClaimedCount, 1);
	TestEqual(TEXT("Timeout auto-play keeps the locked value"), DoubtWindowState.LastAnnouncement.ClaimedValue, 7);
	TestEqual(TEXT("Pile contains opener plus timeout play"), DoubtWindowState.ActualPile.Num(), 2);
	TestEqual(TEXT("Turn advances after timeout auto-play"), DoubtWindowState.ActivePlayerId(), 20);

	DoubtWindowState.ConsecutiveTimeouts[10] = 2;
	NoteVoluntaryAction(DoubtWindowState, 10);
	TestEqual(TEXT("Voluntary action resets timeout streak"), DoubtWindowState.ConsecutiveTimeouts[10], 0);

	return true;
}

// --- Disconnect edge cases owned by the pure state model -------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDubitoEdgeDisconnectTest, "Dubito.Core.Edge.DisconnectBoundaries", DubitoEdgeFlags)
bool FDubitoEdgeDisconnectTest::RunTest(const FString&)
{
	using namespace DubitoRules;

	FDubitoMatchState ClaimantLeavesState = MakeState({ 10, 20, 30 }, ThreePlayerHands());
	ApplyPlay(ClaimantLeavesState, 10, { C(EDubitoSuit::Clubs, 7) }, FDubitoAnnouncement(7, 1));
	HandleDisconnect(ClaimantLeavesState, 10);
	TestFalse(TEXT("Disconnected claimant's play is no longer doubtable"), ClaimantLeavesState.bLastPlayDoubtable);
	TestEqual(TEXT("Disconnected claimant id is cleared"), ClaimantLeavesState.LastClaimantId, DubitoConstants::NoPlayerId);
	TestEqual(TEXT("Pile remains after claimant disconnect"), ClaimantLeavesState.ActualPile.Num(), 1);
	TestEqual(TEXT("Round value remains while pile remains"), ClaimantLeavesState.RoundValue, 7);
	TestEqual(TEXT("Active responder remains active after earlier seat removal"), ClaimantLeavesState.ActivePlayerId(), 20);

	FDubitoMatchState ActiveLeavesState = MakeState({ 10, 20, 30 }, ThreePlayerHands());
	ApplyPlay(ActiveLeavesState, 10, { C(EDubitoSuit::Clubs, 7) }, FDubitoAnnouncement(7, 1));
	HandleDisconnect(ActiveLeavesState, 20);
	TestEqual(TEXT("Pile remains after active disconnect"), ActiveLeavesState.ActualPile.Num(), 1);
	TestEqual(TEXT("Active disconnect advances to the next player"), ActiveLeavesState.ActivePlayerId(), 30);

	FDubitoMatchState PendingWinnerLeavesState = MakeState({ 10, 20, 30 }, ThreePlayerHands());
	PendingWinnerLeavesState.PendingWinnerId = 10;
	PendingWinnerLeavesState.ActiveSeatIndex = 1;
	HandleDisconnect(PendingWinnerLeavesState, 10);
	TestFalse(TEXT("Pending winner leaving cancels the pending win"), PendingWinnerLeavesState.HasPendingWin());
	TestEqual(TEXT("No winner is confirmed while multiple players remain"), PendingWinnerLeavesState.WinnerId, DubitoConstants::NoPlayerId);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
