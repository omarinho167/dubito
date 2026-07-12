#include "DubitoPlaySelectionView.h"

#include "DubitoRules.h"
#include "Math/UnrealMathUtility.h"

namespace
{
	bool IsActiveTurnPhase(EDubitoPhase Phase)
	{
		return Phase == EDubitoPhase::PlayerTurn;
	}
}

FDubitoPlaySelectionView BuildDubitoPlaySelectionView(const FDubitoPlaySelectionInputs& Inputs)
{
	FDubitoPlaySelectionView View;

	const int32 HandNum = Inputs.LocalHand.Num();

	// Composing is gated exactly like the HUD's Play availability: it must be the local
	// player's active turn (Play is allowed on the empty pile that opens a round and during
	// the pending-win window, both of which are still the PlayerTurn phase) with cards in hand.
	View.bCanCompose = Inputs.bIsLocalPlayerActive && IsActiveTurnPhase(Inputs.Phase) && HandNum > 0;

	// Opening a round exposes the value control; a locked round only exposes the count.
	View.bIsOpeningRound = Inputs.RoundValue == DubitoConstants::NoRoundValue;
	View.bValueControlEnabled = View.bCanCompose && View.bIsOpeningRound;

	View.EffectiveValue = View.bIsOpeningRound ? Inputs.ChosenValue : Inputs.RoundValue;

	View.MinClaimedCount = DubitoConstants::MinCardsPerPlay;
	View.MaxClaimedCount = DubitoConstants::MaxCardsPerPlay;
	View.ClaimedCount = FMath::Clamp(Inputs.ClaimedCount, 0, DubitoConstants::MaxCardsPerPlay);

	// Resolve the selection: build the per-card picked flags and count valid unique picks.
	// A picked index that is out of range or repeated makes the whole selection malformed.
	bool bSelectionMalformed = false;
	TSet<int32> PickedIndices;
	PickedIndices.Reserve(Inputs.SelectedHandIndices.Num());
	for (const int32 Index : Inputs.SelectedHandIndices)
	{
		if (Index < 0 || Index >= HandNum)
		{
			bSelectionMalformed = true;
			continue;
		}

		bool bAlreadyPicked = false;
		PickedIndices.Add(Index, &bAlreadyPicked);
		if (bAlreadyPicked)
		{
			bSelectionMalformed = true;
		}
	}

	View.SelectedCount = PickedIndices.Num();

	View.Cards.Reserve(HandNum);
	for (int32 Index = 0; Index < HandNum; ++Index)
	{
		FDubitoPlayCardView CardView;
		CardView.Card = Inputs.LocalHand[Index];
		CardView.HandIndex = Index;
		CardView.bSelected = PickedIndices.Contains(Index);
		View.Cards.Add(MoveTemp(CardView));
	}

	// A legal actual set is 1..MaxCardsPerPlay unique, in-range cards.
	View.bSelectionValid = !bSelectionMalformed
		&& View.SelectedCount >= DubitoConstants::MinCardsPerPlay
		&& View.SelectedCount <= DubitoConstants::MaxCardsPerPlay;

	// The announcement reuses DubitoCore's value-lock rule so it is never duplicated here.
	View.Announcement = FDubitoAnnouncement(View.EffectiveValue, View.ClaimedCount);
	View.bAnnouncementValid = DubitoRules::IsAnnouncementValidForRound(View.Announcement, Inputs.RoundValue);

	// Resolve the first blocking reason, most fundamental first.
	if (!View.bCanCompose)
	{
		View.Error = EDubitoPlayComposeError::NotYourTurn;
	}
	else if (View.SelectedCount == 0 && !bSelectionMalformed)
	{
		View.Error = EDubitoPlayComposeError::NoCardsSelected;
	}
	else if (bSelectionMalformed)
	{
		View.Error = EDubitoPlayComposeError::InvalidSelection;
	}
	else if (View.SelectedCount > DubitoConstants::MaxCardsPerPlay)
	{
		View.Error = EDubitoPlayComposeError::TooManyCardsSelected;
	}
	else if (View.bIsOpeningRound && !View.Announcement.HasValidValue())
	{
		View.Error = EDubitoPlayComposeError::ValueNotChosen;
	}
	else if (!View.Announcement.HasValidCount())
	{
		View.Error = EDubitoPlayComposeError::BadClaimedCount;
	}
	else
	{
		View.Error = EDubitoPlayComposeError::None;
	}

	View.bIsSubmittable = View.bCanCompose && View.bSelectionValid && View.bAnnouncementValid
		&& View.Error == EDubitoPlayComposeError::None;

	// The payload is only meaningful when the whole intent is submittable. Emit the picked
	// cards in stable hand order so the actual set is deterministic.
	if (View.bIsSubmittable)
	{
		View.ActualCards.Reserve(View.SelectedCount);
		for (int32 Index = 0; Index < HandNum; ++Index)
		{
			if (PickedIndices.Contains(Index))
			{
				View.ActualCards.Add(Inputs.LocalHand[Index]);
			}
		}
	}

	return View;
}
