#include "DubitoRevealView.h"

FDubitoRevealView BuildDubitoRevealView(const FDubitoRevealInfo& Reveal, int32 LocalPlayerId)
{
	FDubitoRevealView View;

	// A real reveal names both a claimant and a doubter; the default-constructed payload does not.
	View.bValid = Reveal.ClaimantId != DubitoConstants::NoPlayerId && Reveal.DoubterId != DubitoConstants::NoPlayerId;
	if (!View.bValid)
	{
		return View;
	}

	View.ClaimantId = Reveal.ClaimantId;
	View.DoubterId = Reveal.DoubterId;
	View.LoserId = Reveal.LoserId;
	View.Claim = Reveal.Claim;
	View.RevealedCards = Reveal.RevealedCards;
	View.ActualCount = Reveal.RevealedCards.Num();
	View.ClaimedStakeTransferred = Reveal.ClaimedStakeTransferred;
	View.bWasLie = Reveal.bWasLie;
	View.bWinConfirmed = Reveal.bWinConfirmed;
	View.WinnerId = Reveal.WinnerId;

	// Break the outcome into its readable parts: a play can lie about the count, the value, or
	// both. The count lie is a size mismatch; the value lie is any card that is not the claimed
	// (== round) value.
	View.bCountWasLie = View.ActualCount != Reveal.Claim.ClaimedCount;

	bool bAnyOffValue = false;
	for (const FDubitoCard& Card : Reveal.RevealedCards)
	{
		if (Card.Rank != Reveal.Claim.ClaimedValue)
		{
			bAnyOffValue = true;
			break;
		}
	}
	View.bValueWasLie = bAnyOffValue;

	if (View.bCountWasLie && View.bValueWasLie)
	{
		View.LieKind = EDubitoRevealLieKind::CountAndValueLie;
	}
	else if (View.bCountWasLie)
	{
		View.LieKind = EDubitoRevealLieKind::CountLie;
	}
	else if (View.bValueWasLie)
	{
		View.LieKind = EDubitoRevealLieKind::ValueLie;
	}
	else
	{
		View.LieKind = EDubitoRevealLieKind::Honest;
	}

	// The verdict follows the authoritative lie flag: a correct doubt caught a lie, a wrong
	// doubt challenged an honest play.
	View.Outcome = Reveal.bWasLie ? EDubitoRevealOutcome::CorrectDoubt : EDubitoRevealOutcome::WrongDoubt;

	const bool bHasLocal = LocalPlayerId != DubitoConstants::NoPlayerId;
	View.bLocalIsClaimant = bHasLocal && LocalPlayerId == Reveal.ClaimantId;
	View.bLocalIsDoubter = bHasLocal && LocalPlayerId == Reveal.DoubterId;
	View.bLocalIsLoser = bHasLocal && LocalPlayerId == Reveal.LoserId;
	View.bLocalIsWinner = bHasLocal && Reveal.bWinConfirmed && LocalPlayerId == Reveal.WinnerId;

	return View;
}
