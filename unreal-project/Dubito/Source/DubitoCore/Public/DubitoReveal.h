#pragma once

#include "CoreMinimal.h"
#include "DubitoCard.h"
#include "DubitoAnnouncement.h"
#include "DubitoConstants.h"
#include "DubitoReveal.generated.h"

// Self-contained public payload describing the outcome of a Doubt. It carries exactly
// what a client needs to render the reveal without guessing hidden state: the claim,
// the now-revealed actual cards of the doubted play, the verdict, the loser who takes
// the pile, and the public claimed stake that moved. The rest of the pile stays hidden.
USTRUCT(BlueprintType)
struct DUBITOCORE_API FDubitoRevealInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Dubito")
	int32 ClaimantId = DubitoConstants::NoPlayerId;

	UPROPERTY(BlueprintReadOnly, Category = "Dubito")
	int32 DoubterId = DubitoConstants::NoPlayerId;

	UPROPERTY(BlueprintReadOnly, Category = "Dubito")
	FDubitoAnnouncement Claim;

	// The doubted play's actual cards, revealed by the Doubt.
	UPROPERTY(BlueprintReadOnly, Category = "Dubito")
	TArray<FDubitoCard> RevealedCards;

	// True when the doubt was correct (the play was a lie).
	UPROPERTY(BlueprintReadOnly, Category = "Dubito")
	bool bWasLie = false;

	// The player who takes the pile as a result.
	UPROPERTY(BlueprintReadOnly, Category = "Dubito")
	int32 LoserId = DubitoConstants::NoPlayerId;

	// Public claimed stake moved onto the loser's public ledger (not the hidden pile size).
	UPROPERTY(BlueprintReadOnly, Category = "Dubito")
	int32 ClaimedStakeTransferred = 0;

	// Set when this Doubt resolved a pending win.
	UPROPERTY(BlueprintReadOnly, Category = "Dubito")
	bool bWinConfirmed = false;

	UPROPERTY(BlueprintReadOnly, Category = "Dubito")
	int32 WinnerId = DubitoConstants::NoPlayerId;
};
