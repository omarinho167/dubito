#include "Misc/AutomationTest.h"
#include "DubitoCard.h"
#include "DubitoDeck.h"
#include "DubitoHand.h"
#include "DubitoConstants.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace
{
	constexpr EAutomationTestFlags DubitoTestFlags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;
}

// --- Deck: build integrity -------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDubitoDeckBuildTest, "Dubito.Core.Deck.Build", DubitoTestFlags)
bool FDubitoDeckBuildTest::RunTest(const FString&)
{
	const TArray<FDubitoCard> Deck = DubitoDeck::BuildStandardDeck();

	TestEqual(TEXT("Standard deck has 52 cards"), Deck.Num(), DubitoConstants::DeckSize);

	// All cards are unique.
	const TSet<FDubitoCard> Unique(Deck);
	TestEqual(TEXT("All 52 cards are unique"), Unique.Num(), DubitoConstants::DeckSize);

	// Every card is valid and each rank appears exactly once per suit.
	bool bAllValid = true;
	for (const FDubitoCard& Card : Deck)
	{
		bAllValid &= Card.IsValid();
	}
	TestTrue(TEXT("Every card has a valid rank"), bAllValid);

	for (int32 SuitIndex = 0; SuitIndex < DubitoConstants::NumSuits; ++SuitIndex)
	{
		int32 CountForSuit = 0;
		for (const FDubitoCard& Card : Deck)
		{
			if (Card.Suit == static_cast<EDubitoSuit>(SuitIndex))
			{
				++CountForSuit;
			}
		}
		TestEqual(TEXT("Each suit has 13 cards"), CountForSuit, DubitoConstants::MaxCardRank);
	}

	return true;
}

// --- Deck: deterministic shuffle ------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDubitoDeckShuffleTest, "Dubito.Core.Deck.Shuffle", DubitoTestFlags)
bool FDubitoDeckShuffleTest::RunTest(const FString&)
{
	TArray<FDubitoCard> DeckA = DubitoDeck::BuildStandardDeck();
	TArray<FDubitoCard> DeckB = DeckA;

	FRandomStream StreamA(12345);
	FRandomStream StreamB(12345);
	DubitoDeck::Shuffle(DeckA, StreamA);
	DubitoDeck::Shuffle(DeckB, StreamB);

	TestTrue(TEXT("Same seed produces the same shuffle order"), DeckA == DeckB);

	// Shuffle is a permutation: the multiset of cards is preserved.
	const TSet<FDubitoCard> Unique(DeckA);
	TestEqual(TEXT("Shuffle preserves all 52 unique cards"), Unique.Num(), DubitoConstants::DeckSize);

	// A different seed produces a different order (robust for a 52-card deck).
	TArray<FDubitoCard> DeckC = DubitoDeck::BuildStandardDeck();
	FRandomStream StreamC(98765);
	DubitoDeck::Shuffle(DeckC, StreamC);
	TestFalse(TEXT("A different seed produces a different order"), DeckC == DeckA);

	return true;
}

// --- Deck: deal even + remove leftovers -----------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDubitoDeckDealTest, "Dubito.Core.Deck.Deal", DubitoTestFlags)
bool FDubitoDeckDealTest::RunTest(const FString&)
{
	const TArray<FDubitoCard> Deck = DubitoDeck::BuildStandardDeck();

	auto CheckDeal = [this, &Deck](int32 NumPlayers, int32 ExpectedPerPlayer, int32 ExpectedRemoved)
	{
		int32 Removed = -1;
		const TArray<FDubitoHand> Hands = DubitoDeck::Deal(Deck, NumPlayers, Removed);

		TestEqual(TEXT("One hand per player"), Hands.Num(), NumPlayers);
		TestEqual(TEXT("Leftover count is correct"), Removed, ExpectedRemoved);

		int32 TotalDealt = 0;
		TSet<FDubitoCard> AllDealt;
		for (const FDubitoHand& Hand : Hands)
		{
			TestEqual(TEXT("Each player gets an even share"), Hand.Num(), ExpectedPerPlayer);
			TotalDealt += Hand.Num();
			AllDealt.Append(Hand.Cards);
		}

		TestEqual(TEXT("Dealt + removed equals deck size"), TotalDealt + Removed, Deck.Num());
		TestEqual(TEXT("No card is dealt to two players"), AllDealt.Num(), TotalDealt);
	};

	CheckDeal(2, 26, 0);
	CheckDeal(3, 17, 1);
	CheckDeal(5, 10, 2);
	CheckDeal(8, 6, 4);

	return true;
}

// --- Hand: add / remove / contains / count --------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDubitoHandMutationTest, "Dubito.Core.Hand.Mutation", DubitoTestFlags)
bool FDubitoHandMutationTest::RunTest(const FString&)
{
	FDubitoHand Hand;
	TestTrue(TEXT("New hand is empty"), Hand.IsEmpty());
	TestEqual(TEXT("New hand has zero cards"), Hand.Num(), 0);

	const FDubitoCard SevenHearts(EDubitoSuit::Hearts, 7);
	const FDubitoCard SevenSpades(EDubitoSuit::Spades, 7);
	const FDubitoCard KingClubs(EDubitoSuit::Clubs, 13);

	Hand.Add(SevenHearts);
	Hand.Add(SevenSpades);
	Hand.Add(KingClubs);

	TestFalse(TEXT("Hand with cards is not empty"), Hand.IsEmpty());
	TestEqual(TEXT("Hand has three cards"), Hand.Num(), 3);
	TestTrue(TEXT("Hand contains an added card"), Hand.Contains(SevenHearts));
	TestFalse(TEXT("Hand does not contain a card never added"), Hand.Contains(FDubitoCard(EDubitoSuit::Diamonds, 2)));

	TestEqual(TEXT("Two cards share rank 7"), Hand.CountOfRank(7), 2);
	TestEqual(TEXT("One card of rank 13"), Hand.CountOfRank(13), 1);

	TestTrue(TEXT("Removing a held card succeeds"), Hand.Remove(SevenHearts));
	TestEqual(TEXT("Hand shrinks after removal"), Hand.Num(), 2);
	TestEqual(TEXT("Only one rank-7 card remains"), Hand.CountOfRank(7), 1);

	TestFalse(TEXT("Removing a card not in hand fails"), Hand.Remove(SevenHearts));
	TestEqual(TEXT("Failed removal does not change the hand"), Hand.Num(), 2);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
