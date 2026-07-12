#include "Misc/AutomationTest.h"
#include "DubitoAnnouncement.h"
#include "DubitoCard.h"
#include "DubitoConstants.h"
#include "DubitoReveal.h"
#include "DubitoRevealView.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace
{
	constexpr EAutomationTestFlags DubitoRevealFlags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	// A base reveal: player 10 claimed "2 x value 7" and player 20 doubted. Callers set the
	// revealed cards and verdict to shape each scenario.
	FDubitoRevealInfo MakeReveal()
	{
		FDubitoRevealInfo Reveal;
		Reveal.ClaimantId = 10;
		Reveal.DoubterId = 20;
		Reveal.Claim = FDubitoAnnouncement(7, 2);
		Reveal.ClaimedStakeTransferred = 2;
		return Reveal;
	}
}

// --- Default reveal is not presented ---------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDubitoRevealInvalidTest, "Dubito.Unreal.Reveal.Invalid", DubitoRevealFlags)
bool FDubitoRevealInvalidTest::RunTest(const FString&)
{
	FDubitoRevealView View = BuildDubitoRevealView(FDubitoRevealInfo(), 10);
	TestFalse(TEXT("A default reveal is not valid"), View.bValid);
	TestEqual(TEXT("No outcome for a default reveal"), View.Outcome, EDubitoRevealOutcome::None);
	return true;
}

// --- Honest play, wrong doubt ----------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDubitoRevealHonestTest, "Dubito.Unreal.Reveal.HonestPlayWrongDoubt", DubitoRevealFlags)
bool FDubitoRevealHonestTest::RunTest(const FString&)
{
	// Claimed 2 x 7, actually played two 7s: honest. The doubter (20) is wrong and takes the pile.
	FDubitoRevealInfo Reveal = MakeReveal();
	Reveal.RevealedCards = { FDubitoCard(EDubitoSuit::Clubs, 7), FDubitoCard(EDubitoSuit::Hearts, 7) };
	Reveal.bWasLie = false;
	Reveal.LoserId = 20;

	FDubitoRevealView View = BuildDubitoRevealView(Reveal, 20);
	TestTrue(TEXT("Valid reveal"), View.bValid);
	TestEqual(TEXT("Wrong doubt outcome"), View.Outcome, EDubitoRevealOutcome::WrongDoubt);
	TestEqual(TEXT("Honest lie kind"), View.LieKind, EDubitoRevealLieKind::Honest);
	TestFalse(TEXT("No count lie"), View.bCountWasLie);
	TestFalse(TEXT("No value lie"), View.bValueWasLie);
	TestEqual(TEXT("Actual count is two"), View.ActualCount, 2);
	TestEqual(TEXT("Loser is the doubter"), View.LoserId, 20);
	TestTrue(TEXT("Local doubter recognised"), View.bLocalIsDoubter);
	TestTrue(TEXT("Local doubter is the loser here"), View.bLocalIsLoser);
	return true;
}

// --- Count lie, correct doubt ----------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDubitoRevealCountLieTest, "Dubito.Unreal.Reveal.CountLieCorrectDoubt", DubitoRevealFlags)
bool FDubitoRevealCountLieTest::RunTest(const FString&)
{
	// Claimed 2 x 7 but only played one 7: a count lie. Doubt is correct; claimant (10) loses.
	FDubitoRevealInfo Reveal = MakeReveal();
	Reveal.RevealedCards = { FDubitoCard(EDubitoSuit::Clubs, 7) };
	Reveal.bWasLie = true;
	Reveal.LoserId = 10;

	FDubitoRevealView View = BuildDubitoRevealView(Reveal, 10);
	TestEqual(TEXT("Correct doubt outcome"), View.Outcome, EDubitoRevealOutcome::CorrectDoubt);
	TestEqual(TEXT("Count lie kind"), View.LieKind, EDubitoRevealLieKind::CountLie);
	TestTrue(TEXT("Count was a lie"), View.bCountWasLie);
	TestFalse(TEXT("Value was truthful"), View.bValueWasLie);
	TestEqual(TEXT("Actual count is one"), View.ActualCount, 1);
	TestEqual(TEXT("Loser is the claimant"), View.LoserId, 10);
	TestTrue(TEXT("Local claimant recognised"), View.bLocalIsClaimant);
	TestTrue(TEXT("Local claimant is the loser here"), View.bLocalIsLoser);
	return true;
}

// --- Value lie, correct doubt ----------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDubitoRevealValueLieTest, "Dubito.Unreal.Reveal.ValueLieCorrectDoubt", DubitoRevealFlags)
bool FDubitoRevealValueLieTest::RunTest(const FString&)
{
	// Claimed 2 x 7 and played two cards, but one is a 3: a value lie (count is truthful).
	FDubitoRevealInfo Reveal = MakeReveal();
	Reveal.RevealedCards = { FDubitoCard(EDubitoSuit::Clubs, 7), FDubitoCard(EDubitoSuit::Spades, 3) };
	Reveal.bWasLie = true;
	Reveal.LoserId = 10;

	FDubitoRevealView View = BuildDubitoRevealView(Reveal, 99);
	TestEqual(TEXT("Correct doubt outcome"), View.Outcome, EDubitoRevealOutcome::CorrectDoubt);
	TestEqual(TEXT("Value lie kind"), View.LieKind, EDubitoRevealLieKind::ValueLie);
	TestFalse(TEXT("Count was truthful"), View.bCountWasLie);
	TestTrue(TEXT("Value was a lie"), View.bValueWasLie);
	TestFalse(TEXT("A bystander is not the claimant"), View.bLocalIsClaimant);
	TestFalse(TEXT("A bystander is not the doubter"), View.bLocalIsDoubter);
	TestFalse(TEXT("A bystander is not the loser"), View.bLocalIsLoser);
	return true;
}

// --- Count and value lie ---------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDubitoRevealBothLieTest, "Dubito.Unreal.Reveal.CountAndValueLie", DubitoRevealFlags)
bool FDubitoRevealBothLieTest::RunTest(const FString&)
{
	// Claimed 2 x 7 but played a single 3: wrong count and wrong value.
	FDubitoRevealInfo Reveal = MakeReveal();
	Reveal.RevealedCards = { FDubitoCard(EDubitoSuit::Spades, 3) };
	Reveal.bWasLie = true;
	Reveal.LoserId = 10;

	FDubitoRevealView View = BuildDubitoRevealView(Reveal, 10);
	TestEqual(TEXT("Correct doubt outcome"), View.Outcome, EDubitoRevealOutcome::CorrectDoubt);
	TestEqual(TEXT("Count and value lie kind"), View.LieKind, EDubitoRevealLieKind::CountAndValueLie);
	TestTrue(TEXT("Count was a lie"), View.bCountWasLie);
	TestTrue(TEXT("Value was a lie"), View.bValueWasLie);
	return true;
}

// --- Pending win confirmed by a wrong final doubt --------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDubitoRevealWinTest, "Dubito.Unreal.Reveal.WinConfirmed", DubitoRevealFlags)
bool FDubitoRevealWinTest::RunTest(const FString&)
{
	// The claimant emptied their hand honestly; the final doubt is wrong, confirming the win.
	FDubitoRevealInfo Reveal = MakeReveal();
	Reveal.RevealedCards = { FDubitoCard(EDubitoSuit::Clubs, 7), FDubitoCard(EDubitoSuit::Hearts, 7) };
	Reveal.bWasLie = false;
	Reveal.LoserId = 20;
	Reveal.bWinConfirmed = true;
	Reveal.WinnerId = 10;

	FDubitoRevealView View = BuildDubitoRevealView(Reveal, 10);
	TestEqual(TEXT("Wrong doubt outcome"), View.Outcome, EDubitoRevealOutcome::WrongDoubt);
	TestTrue(TEXT("Win confirmed"), View.bWinConfirmed);
	TestEqual(TEXT("Winner passes through"), View.WinnerId, 10);
	TestTrue(TEXT("Local player is the winner"), View.bLocalIsWinner);

	// A non-winner local player does not read as the winner even on a win-confirmed reveal.
	FDubitoRevealView Other = BuildDubitoRevealView(Reveal, 20);
	TestFalse(TEXT("Non-winner is not flagged winner"), Other.bLocalIsWinner);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
