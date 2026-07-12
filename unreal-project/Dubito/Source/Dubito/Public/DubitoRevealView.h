#pragma once

#include "CoreMinimal.h"
#include "DubitoAnnouncement.h"
#include "DubitoCard.h"
#include "DubitoConstants.h"
#include "DubitoReveal.h"
#include "DubitoRevealView.generated.h"

/**
 * Phase 5.2 Doubt reveal presentation view-model.
 *
 * This is pure presentation derivation for the public reveal that resolves a Doubt. It turns
 * the self-contained FDubitoRevealInfo (plus the local player's identity) into the readable
 * facts a reveal panel must show so every outcome is legible: was the doubt correct or wrong,
 * what exactly was the lie (count, value, both, or an honest play the doubter wrongly
 * challenged), the claim vs the now-revealed actual cards, who takes the pile, and whether the
 * doubt confirmed or cancelled a pending win.
 *
 * It reads only the self-contained reveal payload, which the server already scrubbed to public
 * facts (the doubted play's actual cards are revealed by the Doubt; the rest of the pile stays
 * hidden). It holds no rule authority and never guesses hidden state.
 */

// The headline outcome of the reveal, from the table's point of view.
UENUM(BlueprintType)
enum class EDubitoRevealOutcome : uint8
{
	None,           // no valid reveal to present
	CorrectDoubt,   // the doubt was right: the play was a lie
	WrongDoubt      // the doubt was wrong: the play was honest
};

// What made the doubted play a lie. A play can lie about the count, the value, or both; an
// honest play lies about neither (so a doubt against it is a WrongDoubt).
UENUM(BlueprintType)
enum class EDubitoRevealLieKind : uint8
{
	Honest,        // actual count == claimed count and every card is the claimed value
	CountLie,      // actual count differs from the claimed count only
	ValueLie,      // at least one card is not the claimed value only
	CountAndValueLie
};

// The complete reveal snapshot for one client's reveal panel.
USTRUCT(BlueprintType)
struct DUBITO_API FDubitoRevealView
{
	GENERATED_BODY()

	// False until a real reveal has been presented (initial/cleared state).
	UPROPERTY(BlueprintReadOnly, Category = "Dubito|Reveal")
	bool bValid = false;

	UPROPERTY(BlueprintReadOnly, Category = "Dubito|Reveal")
	EDubitoRevealOutcome Outcome = EDubitoRevealOutcome::None;

	UPROPERTY(BlueprintReadOnly, Category = "Dubito|Reveal")
	EDubitoRevealLieKind LieKind = EDubitoRevealLieKind::Honest;

	// True when the doubt was correct (the play was a lie). Mirrors FDubitoRevealInfo::bWasLie.
	UPROPERTY(BlueprintReadOnly, Category = "Dubito|Reveal")
	bool bWasLie = false;

	UPROPERTY(BlueprintReadOnly, Category = "Dubito|Reveal")
	int32 ClaimantId = DubitoConstants::NoPlayerId;

	UPROPERTY(BlueprintReadOnly, Category = "Dubito|Reveal")
	int32 DoubterId = DubitoConstants::NoPlayerId;

	// The player who takes the pile as a result of the reveal.
	UPROPERTY(BlueprintReadOnly, Category = "Dubito|Reveal")
	int32 LoserId = DubitoConstants::NoPlayerId;

	// The public claim that was judged.
	UPROPERTY(BlueprintReadOnly, Category = "Dubito|Reveal")
	FDubitoAnnouncement Claim;

	// The doubted play's actual cards, revealed by the Doubt.
	UPROPERTY(BlueprintReadOnly, Category = "Dubito|Reveal")
	TArray<FDubitoCard> RevealedCards;

	// The actual number of cards that were played (RevealedCards.Num()), for the count compare.
	UPROPERTY(BlueprintReadOnly, Category = "Dubito|Reveal")
	int32 ActualCount = 0;

	// True when the revealed count differs from the claimed count.
	UPROPERTY(BlueprintReadOnly, Category = "Dubito|Reveal")
	bool bCountWasLie = false;

	// True when at least one revealed card is not the claimed value.
	UPROPERTY(BlueprintReadOnly, Category = "Dubito|Reveal")
	bool bValueWasLie = false;

	// Public claimed stake moved onto the loser's public ledger (not the hidden pile size).
	UPROPERTY(BlueprintReadOnly, Category = "Dubito|Reveal")
	int32 ClaimedStakeTransferred = 0;

	// Set when this reveal resolved a pending win.
	UPROPERTY(BlueprintReadOnly, Category = "Dubito|Reveal")
	bool bWinConfirmed = false;

	UPROPERTY(BlueprintReadOnly, Category = "Dubito|Reveal")
	int32 WinnerId = DubitoConstants::NoPlayerId;

	// Local-perspective flags so the panel can address the local player directly.
	UPROPERTY(BlueprintReadOnly, Category = "Dubito|Reveal")
	bool bLocalIsClaimant = false;

	UPROPERTY(BlueprintReadOnly, Category = "Dubito|Reveal")
	bool bLocalIsDoubter = false;

	UPROPERTY(BlueprintReadOnly, Category = "Dubito|Reveal")
	bool bLocalIsLoser = false;

	UPROPERTY(BlueprintReadOnly, Category = "Dubito|Reveal")
	bool bLocalIsWinner = false;
};

// Pure builder. Deterministic; safe to call anywhere, including automation tests.
DUBITO_API FDubitoRevealView BuildDubitoRevealView(const FDubitoRevealInfo& Reveal, int32 LocalPlayerId);
