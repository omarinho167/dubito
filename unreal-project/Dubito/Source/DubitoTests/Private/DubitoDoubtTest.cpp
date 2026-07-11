#include "Misc/AutomationTest.h"
#include "DubitoRules.h"
#include "DubitoMatchState.h"
#include "DubitoReveal.h"
#include "DubitoAnnouncement.h"
#include "DubitoCard.h"
#include "DubitoHand.h"
#include "DubitoConstants.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace
{
	constexpr EAutomationTestFlags DubitoDoubtFlags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	FDubitoCard C(EDubitoSuit Suit, int32 Rank) { return FDubitoCard(Suit, Rank); }

	struct FDoubtOutcome
	{
		FDubitoMatchState State;
		FDubitoRevealInfo Reveal;
	};

	// Player 10 (active) opens with the given actual cards and announcement, then the
	// immediate next player 20 Doubts. Player 20 holds four distinct Spades as filler.
	FDoubtOutcome PlayThenDoubt(const TArray<FDubitoCard>& P10Hand, const TArray<FDubitoCard>& Actual, const FDubitoAnnouncement& Ann)
	{
		TMap<int32, FDubitoHand> Hands;
		FDubitoHand H10; H10.Cards = P10Hand;
		FDubitoHand H20; H20.Cards = { C(EDubitoSuit::Spades, 10), C(EDubitoSuit::Spades, 11), C(EDubitoSuit::Spades, 12), C(EDubitoSuit::Spades, 13) };
		Hands.Add(10, H10);
		Hands.Add(20, H20);

		FDoubtOutcome Out;
		DubitoRules::SetupMatch(Out.State, { 10, 20 }, Hands);
		DubitoRules::ApplyPlay(Out.State, 10, Actual, Ann);
		Out.Reveal = DubitoRules::ResolveDoubt(Out.State, 20);
		return Out;
	}
}

// --- Honest play doubted: wrong doubt, doubter takes pile and loses turn ---
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDubitoDoubtHonestTest, "Dubito.Core.Doubt.HonestWrong", DubitoDoubtFlags)
bool FDubitoDoubtHonestTest::RunTest(const FString&)
{
	// Plays two real 7s claiming (7,2): count and value both truthful.
	FDoubtOutcome O = PlayThenDoubt(
		{ C(EDubitoSuit::Hearts, 7), C(EDubitoSuit::Spades, 7), C(EDubitoSuit::Clubs, 2), C(EDubitoSuit::Diamonds, 2) },
		{ C(EDubitoSuit::Hearts, 7), C(EDubitoSuit::Spades, 7) },
		FDubitoAnnouncement(7, 2));

	TestFalse(TEXT("Honest play: doubt is wrong"), O.Reveal.bWasLie);
	TestEqual(TEXT("Doubter (20) loses and takes the pile"), O.Reveal.LoserId, 20);
	TestEqual(TEXT("Doubter hand grew by the actual pile size (4+2)"), O.State.Hands[20].Num(), 6);
	TestEqual(TEXT("Doubter public ledger grew by the claimed stake (4+2)"), O.State.PublicHandCounts[20], 6);
	TestTrue(TEXT("Pile emptied"), O.State.IsPileEmpty());
	TestFalse(TEXT("Round reset"), O.State.IsRoundOpen());
	TestEqual(TEXT("Claimed pile ledger reset"), O.State.ClaimedPileCount, 0);
	TestEqual(TEXT("Doubter loses the turn: next active is 10"), O.State.ActivePlayerId(), 10);
	return true;
}

// --- Count lie doubted: correct doubt, liar takes pile, doubter plays -------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDubitoDoubtCountLieTest, "Dubito.Core.Doubt.CountLie", DubitoDoubtFlags)
bool FDubitoDoubtCountLieTest::RunTest(const FString&)
{
	// Plays one real 7 but claims count 2: value truthful, count is a lie.
	FDoubtOutcome O = PlayThenDoubt(
		{ C(EDubitoSuit::Hearts, 7), C(EDubitoSuit::Clubs, 2), C(EDubitoSuit::Diamonds, 2), C(EDubitoSuit::Diamonds, 3) },
		{ C(EDubitoSuit::Hearts, 7) },
		FDubitoAnnouncement(7, 2));

	TestTrue(TEXT("Count lie: doubt is correct"), O.Reveal.bWasLie);
	TestEqual(TEXT("Liar (10) takes the pile"), O.Reveal.LoserId, 10);
	TestEqual(TEXT("Liar hand: 3 left + 1 pile card = 4"), O.State.Hands[10].Num(), 4);
	TestEqual(TEXT("Liar public: 2 + claimed stake 2 = 4"), O.State.PublicHandCounts[10], 4);
	TestTrue(TEXT("Pile emptied"), O.State.IsPileEmpty());
	TestFalse(TEXT("Round reset"), O.State.IsRoundOpen());
	TestEqual(TEXT("Doubter (20) plays next"), O.State.ActivePlayerId(), 20);
	return true;
}

// --- Value lie doubted: correct doubt --------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDubitoDoubtValueLieTest, "Dubito.Core.Doubt.ValueLie", DubitoDoubtFlags)
bool FDubitoDoubtValueLieTest::RunTest(const FString&)
{
	// Opens value 7 count 2 but plays two 3s: count truthful, value is a lie.
	FDoubtOutcome O = PlayThenDoubt(
		{ C(EDubitoSuit::Clubs, 3), C(EDubitoSuit::Diamonds, 3), C(EDubitoSuit::Hearts, 2), C(EDubitoSuit::Spades, 2) },
		{ C(EDubitoSuit::Clubs, 3), C(EDubitoSuit::Diamonds, 3) },
		FDubitoAnnouncement(7, 2));

	TestTrue(TEXT("Value lie: doubt is correct"), O.Reveal.bWasLie);
	TestEqual(TEXT("Liar (10) takes the pile"), O.Reveal.LoserId, 10);
	TestEqual(TEXT("Doubter (20) plays next"), O.State.ActivePlayerId(), 20);
	TestFalse(TEXT("Round reset"), O.State.IsRoundOpen());
	return true;
}

// --- Both count and value lie doubted: correct doubt -----------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDubitoDoubtBothLieTest, "Dubito.Core.Doubt.BothLie", DubitoDoubtFlags)
bool FDubitoDoubtBothLieTest::RunTest(const FString&)
{
	// Opens value 7 count 2 but plays a single 3: both count and value are lies.
	FDoubtOutcome O = PlayThenDoubt(
		{ C(EDubitoSuit::Clubs, 3), C(EDubitoSuit::Hearts, 2), C(EDubitoSuit::Spades, 2), C(EDubitoSuit::Diamonds, 4) },
		{ C(EDubitoSuit::Clubs, 3) },
		FDubitoAnnouncement(7, 2));

	TestTrue(TEXT("Both lies: doubt is correct"), O.Reveal.bWasLie);
	TestEqual(TEXT("Liar (10) takes the pile"), O.Reveal.LoserId, 10);
	TestEqual(TEXT("Revealed the single actual card"), O.Reveal.RevealedCards.Num(), 1);
	TestEqual(TEXT("Doubter (20) plays next"), O.State.ActivePlayerId(), 20);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
