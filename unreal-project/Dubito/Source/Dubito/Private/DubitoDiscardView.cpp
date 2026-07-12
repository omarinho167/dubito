#include "DubitoDiscardView.h"

FDubitoDiscardView BuildDubitoDiscardView(const FDubitoDiscardInputs& Inputs)
{
	FDubitoDiscardView View;

	const bool bActiveTurn = Inputs.bIsLocalPlayerActive && Inputs.Phase == EDubitoPhase::PlayerTurn;

	// Resolve the most fundamental blocker first so the explanation is stable.
	if (!bActiveTurn)
	{
		View.bCanDiscard = false;
		View.BlockedReason = EDubitoDiscardBlockedReason::NotYourTurn;
	}
	else if (Inputs.bHasPendingWin)
	{
		// A win is pending: the active responder may Play or make a final Doubt, never Discard.
		View.bCanDiscard = false;
		View.BlockedReason = EDubitoDiscardBlockedReason::PendingWin;
	}
	else if (Inputs.ClaimedPileCount <= 0)
	{
		View.bCanDiscard = false;
		View.BlockedReason = EDubitoDiscardBlockedReason::EmptyPile;
	}
	else
	{
		View.bCanDiscard = true;
		View.BlockedReason = EDubitoDiscardBlockedReason::None;
	}

	return View;
}
