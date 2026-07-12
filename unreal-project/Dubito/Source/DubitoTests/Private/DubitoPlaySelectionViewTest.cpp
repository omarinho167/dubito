#include "Misc/AutomationTest.h"
#include "DubitoAnnouncement.h"
#include "DubitoCard.h"
#include "DubitoConstants.h"
#include "DubitoMatchState.h"
#include "DubitoPlaySelectionView.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace
{
	constexpr EAutomationTestFlags DubitoPlaySelectionFlags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	// A five-card hand of distinct ranks in a single suit, so selections are unambiguous.
	TArray<FDubitoCard> MakeHand()
	{
		return {
			FDubitoCard(EDubitoSuit::Clubs, 3),
			FDubitoCard(EDubitoSuit::Clubs, 7),
			FDubitoCard(EDubitoSuit::Hearts, 7),
			FDubitoCard(EDubitoSuit::Spades, 10),
			FDubitoCard(EDubitoSuit::Diamonds, 13)
		};
	}

	// The local player's active turn on an empty pile (opening a round), with a full hand.
	FDubitoPlaySelectionInputs MakeOpeningInputs()
	{
		FDubitoPlaySelectionInputs In;
		In.Phase = EDubitoPhase::PlayerTurn;
		In.bIsLocalPlayerActive = true;
		In.RoundValue = DubitoConstants::NoRoundValue;
		In.LocalHand = MakeHand();
		return In;
	}
}

// --- Gating: only the active player on their turn may compose --------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDubitoPlaySelectionGateTest, "Dubito.Unreal.PlaySelection.Gate", DubitoPlaySelectionFlags)
bool FDubitoPlaySelectionGateTest::RunTest(const FString&)
{
	FDubitoPlaySelectionInputs In = MakeOpeningInputs();

	// Not the local player's turn.
	FDubitoPlaySelectionInputs NotMine = In;
	NotMine.bIsLocalPlayerActive = false;
	FDubitoPlaySelectionView NotMineView = BuildDubitoPlaySelectionView(NotMine);
	TestFalse(TEXT("Cannot compose when it is not my turn"), NotMineView.bCanCompose);
	TestFalse(TEXT("Nothing is submittable when it is not my turn"), NotMineView.bIsSubmittable);
	TestEqual(TEXT("Not-my-turn error"), NotMineView.Error, EDubitoPlayComposeError::NotYourTurn);

	// My turn but wrong phase (a doubt reveal is not a compose window).
	FDubitoPlaySelectionInputs Reveal = In;
	Reveal.Phase = EDubitoPhase::DoubtReveal;
	TestFalse(TEXT("Cannot compose during reveal"), BuildDubitoPlaySelectionView(Reveal).bCanCompose);

	// My turn but an empty hand.
	FDubitoPlaySelectionInputs EmptyHand = In;
	EmptyHand.LocalHand.Empty();
	TestFalse(TEXT("Cannot compose with an empty hand"), BuildDubitoPlaySelectionView(EmptyHand).bCanCompose);

	// The happy path can compose.
	TestTrue(TEXT("Can compose on my active turn"), BuildDubitoPlaySelectionView(In).bCanCompose);
	return true;
}

// --- Opening a round exposes the value control ----------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDubitoPlaySelectionOpeningTest, "Dubito.Unreal.PlaySelection.OpeningRound", DubitoPlaySelectionFlags)
bool FDubitoPlaySelectionOpeningTest::RunTest(const FString&)
{
	FDubitoPlaySelectionInputs In = MakeOpeningInputs();
	In.SelectedHandIndices = { 0 };       // one actual card
	In.ChosenValue = 9;                   // free choice while opening
	In.ClaimedCount = 2;                  // bluff: claim two while playing one

	FDubitoPlaySelectionView View = BuildDubitoPlaySelectionView(In);
	TestTrue(TEXT("Opening round is flagged"), View.bIsOpeningRound);
	TestTrue(TEXT("Value control is exposed while opening"), View.bValueControlEnabled);
	TestEqual(TEXT("Effective value is the chosen value while opening"), View.EffectiveValue, 9);
	TestEqual(TEXT("Selected count is the picked cards"), View.SelectedCount, 1);
	TestTrue(TEXT("Selection of one card is valid"), View.bSelectionValid);
	TestTrue(TEXT("Announcement is valid for an opening round"), View.bAnnouncementValid);
	TestTrue(TEXT("A one-card play claiming two is submittable (bluff allowed)"), View.bIsSubmittable);
	TestEqual(TEXT("Submit error is None"), View.Error, EDubitoPlayComposeError::None);

	// Opening without a chosen value cannot submit.
	FDubitoPlaySelectionInputs NoValue = In;
	NoValue.ChosenValue = DubitoConstants::NoRoundValue;
	FDubitoPlaySelectionView NoValueView = BuildDubitoPlaySelectionView(NoValue);
	TestFalse(TEXT("Opening without a value is not submittable"), NoValueView.bIsSubmittable);
	TestEqual(TEXT("Missing value error"), NoValueView.Error, EDubitoPlayComposeError::ValueNotChosen);
	return true;
}

// --- Following a round locks the value ------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDubitoPlaySelectionLockedValueTest, "Dubito.Unreal.PlaySelection.LockedValue", DubitoPlaySelectionFlags)
bool FDubitoPlaySelectionLockedValueTest::RunTest(const FString&)
{
	FDubitoPlaySelectionInputs In = MakeOpeningInputs();
	In.RoundValue = 7;                    // round is open, value locked to 7
	In.SelectedHandIndices = { 0, 3 };    // play a 3 and a 10 -- value bluff on both
	In.ChosenValue = 2;                   // ignored while the round is open
	In.ClaimedCount = 2;

	FDubitoPlaySelectionView View = BuildDubitoPlaySelectionView(In);
	TestFalse(TEXT("Not an opening round"), View.bIsOpeningRound);
	TestFalse(TEXT("Value control is disabled while the round is open"), View.bValueControlEnabled);
	TestEqual(TEXT("Effective value is forced to the locked round value"), View.EffectiveValue, 7);
	TestEqual(TEXT("Announcement claimed value is the round value"), View.Announcement.ClaimedValue, 7);
	TestTrue(TEXT("Announcement is valid for the locked round"), View.bAnnouncementValid);
	TestTrue(TEXT("Playing off-value cards is still submittable (value bluff)"), View.bIsSubmittable);
	return true;
}

// --- Selection bounds ------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDubitoPlaySelectionBoundsTest, "Dubito.Unreal.PlaySelection.SelectionBounds", DubitoPlaySelectionFlags)
bool FDubitoPlaySelectionBoundsTest::RunTest(const FString&)
{
	FDubitoPlaySelectionInputs In = MakeOpeningInputs();
	In.ChosenValue = 5;
	In.ClaimedCount = 1;

	// No cards picked yet.
	FDubitoPlaySelectionView NoneView = BuildDubitoPlaySelectionView(In);
	TestFalse(TEXT("No cards: not submittable"), NoneView.bIsSubmittable);
	TestEqual(TEXT("No cards error"), NoneView.Error, EDubitoPlayComposeError::NoCardsSelected);

	// Duplicate index is malformed.
	FDubitoPlaySelectionInputs Dup = In;
	Dup.SelectedHandIndices = { 1, 1 };
	FDubitoPlaySelectionView DupView = BuildDubitoPlaySelectionView(Dup);
	TestFalse(TEXT("Duplicate pick: not submittable"), DupView.bIsSubmittable);
	TestEqual(TEXT("Duplicate pick error"), DupView.Error, EDubitoPlayComposeError::InvalidSelection);

	// Out-of-range index is malformed.
	FDubitoPlaySelectionInputs OutOfRange = In;
	OutOfRange.SelectedHandIndices = { 42 };
	FDubitoPlaySelectionView OutView = BuildDubitoPlaySelectionView(OutOfRange);
	TestFalse(TEXT("Out-of-range pick: not submittable"), OutView.bIsSubmittable);
	TestEqual(TEXT("Out-of-range pick error"), OutView.Error, EDubitoPlayComposeError::InvalidSelection);

	// Five cards exceeds MaxCardsPerPlay.
	FDubitoPlaySelectionInputs TooMany = In;
	TooMany.SelectedHandIndices = { 0, 1, 2, 3, 4 };
	FDubitoPlaySelectionView TooManyView = BuildDubitoPlaySelectionView(TooMany);
	TestEqual(TEXT("Five picks counted"), TooManyView.SelectedCount, 5);
	TestFalse(TEXT("Five cards: selection invalid"), TooManyView.bSelectionValid);
	TestFalse(TEXT("Five cards: not submittable"), TooManyView.bIsSubmittable);
	TestEqual(TEXT("Too-many error"), TooManyView.Error, EDubitoPlayComposeError::TooManyCardsSelected);

	// Exactly four cards is the maximum legal actual set.
	FDubitoPlaySelectionInputs FourCards = In;
	FourCards.SelectedHandIndices = { 0, 1, 2, 3 };
	FourCards.ClaimedCount = 4;
	FDubitoPlaySelectionView FourView = BuildDubitoPlaySelectionView(FourCards);
	TestTrue(TEXT("Four cards: submittable"), FourView.bIsSubmittable);
	return true;
}

// --- Claimed count bounds --------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDubitoPlaySelectionCountTest, "Dubito.Unreal.PlaySelection.ClaimedCount", DubitoPlaySelectionFlags)
bool FDubitoPlaySelectionCountTest::RunTest(const FString&)
{
	FDubitoPlaySelectionInputs In = MakeOpeningInputs();
	In.ChosenValue = 5;
	In.SelectedHandIndices = { 0 };

	// Zero claimed count is unset / illegal.
	FDubitoPlaySelectionInputs Zero = In;
	Zero.ClaimedCount = 0;
	FDubitoPlaySelectionView ZeroView = BuildDubitoPlaySelectionView(Zero);
	TestFalse(TEXT("Zero claimed count: not submittable"), ZeroView.bIsSubmittable);
	TestEqual(TEXT("Bad count error"), ZeroView.Error, EDubitoPlayComposeError::BadClaimedCount);

	// Five exceeds the max claim.
	FDubitoPlaySelectionInputs Five = In;
	Five.ClaimedCount = 5;
	FDubitoPlaySelectionView FiveView = BuildDubitoPlaySelectionView(Five);
	TestEqual(TEXT("Claimed count clamps to max for display"), FiveView.ClaimedCount, DubitoConstants::MaxCardsPerPlay);
	// Clamp lands on a legal value, so a claim of 5 presents as 4 and is submittable.
	TestTrue(TEXT("Over-max claim clamps to a legal count"), FiveView.bIsSubmittable);

	// A claim that legally exceeds the actual count is a valid bluff.
	FDubitoPlaySelectionInputs Bluff = In;
	Bluff.ClaimedCount = 4; // claim four while playing one
	FDubitoPlaySelectionView BluffView = BuildDubitoPlaySelectionView(Bluff);
	TestEqual(TEXT("Selected count stays truthful in the view"), BluffView.SelectedCount, 1);
	TestEqual(TEXT("Claimed count carries the bluff"), BluffView.ClaimedCount, 4);
	TestTrue(TEXT("Count bluff is submittable"), BluffView.bIsSubmittable);
	return true;
}

// --- Payload determinism ---------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDubitoPlaySelectionPayloadTest, "Dubito.Unreal.PlaySelection.Payload", DubitoPlaySelectionFlags)
bool FDubitoPlaySelectionPayloadTest::RunTest(const FString&)
{
	FDubitoPlaySelectionInputs In = MakeOpeningInputs();
	In.ChosenValue = 11;
	In.ClaimedCount = 2;
	// Pick indices out of order; the payload must come out in stable hand order.
	In.SelectedHandIndices = { 3, 0 };

	FDubitoPlaySelectionView View = BuildDubitoPlaySelectionView(In);
	if (!TestTrue(TEXT("Submittable payload"), View.bIsSubmittable))
	{
		return false;
	}

	if (!TestEqual(TEXT("Payload has two actual cards"), View.ActualCards.Num(), 2))
	{
		return false;
	}
	TestEqual(TEXT("First actual card is hand index 0"), View.ActualCards[0], FDubitoCard(EDubitoSuit::Clubs, 3));
	TestEqual(TEXT("Second actual card is hand index 3"), View.ActualCards[1], FDubitoCard(EDubitoSuit::Spades, 10));
	TestEqual(TEXT("Announcement value"), View.Announcement.ClaimedValue, 11);
	TestEqual(TEXT("Announcement count"), View.Announcement.ClaimedCount, 2);

	// The per-card render flags mark exactly the picked indices.
	if (!TestEqual(TEXT("All hand cards rendered"), View.Cards.Num(), 5))
	{
		return false;
	}
	TestTrue(TEXT("Index 0 selected"), View.Cards[0].bSelected);
	TestFalse(TEXT("Index 1 not selected"), View.Cards[1].bSelected);
	TestTrue(TEXT("Index 3 selected"), View.Cards[3].bSelected);

	// A non-submittable intent emits no payload.
	FDubitoPlaySelectionInputs Blocked = In;
	Blocked.SelectedHandIndices.Empty();
	FDubitoPlaySelectionView BlockedView = BuildDubitoPlaySelectionView(Blocked);
	TestEqual(TEXT("Blocked intent emits no payload"), BlockedView.ActualCards.Num(), 0);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
