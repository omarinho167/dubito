#pragma once

#include "CoreMinimal.h"
#include "DubitoAnnouncement.h"
#include "DubitoCard.h"
#include "DubitoConstants.h"
#include "DubitoMatchState.h"
#include "DubitoPlaySelectionView.generated.h"

/**
 * Phase 5.1 Play composition view-model.
 *
 * This is pure presentation/intent derivation for the local player's own Play. It turns
 * the local player's EXACT hand plus the replicated public round context plus the local
 * player's in-progress selection intent (which cards are picked, which value/count is
 * claimed) into a submittable Play payload and the flags an action bar needs to render
 * selection and claim controls.
 *
 * It is not rule authority. Submittability mirrors the server's DubitoRules predicates as
 * a best-effort client gate, and it reuses DubitoCore's IsAnnouncementValidForRound so the
 * value-lock rule is never duplicated. The server still validates every Play and issues a
 * controlled rejection/resync if the intent is stale by the time it arrives.
 *
 * Only the owning client ever builds this: it reads the local exact hand and never any
 * hidden opponent state, so it cannot leak another player's actual count.
 */

// Why the current selection cannot be submitted, in the order the view resolves them.
UENUM(BlueprintType)
enum class EDubitoPlayComposeError : uint8
{
	None,
	NotYourTurn,          // not the local player's active turn (or empty hand / wrong phase)
	NoCardsSelected,      // nothing picked yet
	InvalidSelection,     // a picked index is out of range or duplicated
	TooManyCardsSelected, // more than MaxCardsPerPlay picked
	ValueNotChosen,       // opening a round without a valid claimed value
	BadClaimedCount       // claimed count is outside 1..4
};

// One hand card as the action bar should present it: the card plus whether it is picked.
USTRUCT(BlueprintType)
struct DUBITO_API FDubitoPlayCardView
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Dubito|Play")
	FDubitoCard Card;

	// Index into the local hand, so the widget can map a click back to a selection toggle.
	UPROPERTY(BlueprintReadOnly, Category = "Dubito|Play")
	int32 HandIndex = INDEX_NONE;

	UPROPERTY(BlueprintReadOnly, Category = "Dubito|Play")
	bool bSelected = false;
};

// The local player's in-progress Play intent, expressed as plain values so the view is
// testable without actors, a world, or networking.
struct FDubitoPlaySelectionInputs
{
	// Replicated public round context.
	EDubitoPhase Phase = EDubitoPhase::Lobby;
	bool bIsLocalPlayerActive = false;
	int32 RoundValue = DubitoConstants::NoRoundValue; // NoRoundValue => opening an empty pile

	// The local player's EXACT hand, in display order.
	TArray<FDubitoCard> LocalHand;

	// User intent: indices into LocalHand that are picked as the actual cards to play.
	TArray<int32> SelectedHandIndices;

	// Claimed value the player is announcing. Only meaningful while opening a round; when a
	// round is open the value is locked and this input is ignored.
	int32 ChosenValue = DubitoConstants::NoRoundValue;

	// Claimed count (1..4) the player is announcing. May legally differ from the number of
	// selected cards (that is the bluff).
	int32 ClaimedCount = 0;
};

// The complete Play composition snapshot for one frame of the owning client.
USTRUCT(BlueprintType)
struct DUBITO_API FDubitoPlaySelectionView
{
	GENERATED_BODY()

	// True while the local player may compose a Play at all (their active turn, cards in hand).
	UPROPERTY(BlueprintReadOnly, Category = "Dubito|Play")
	bool bCanCompose = false;

	// True when this play opens the round, so the value control must be exposed. When false
	// the claimed value is locked to the round value and only the count control is exposed.
	UPROPERTY(BlueprintReadOnly, Category = "Dubito|Play")
	bool bIsOpeningRound = false;

	// Convenience: the value control is only usable while composing an opening play.
	UPROPERTY(BlueprintReadOnly, Category = "Dubito|Play")
	bool bValueControlEnabled = false;

	// The value that will actually be announced: the chosen value when opening, otherwise
	// the locked round value.
	UPROPERTY(BlueprintReadOnly, Category = "Dubito|Play")
	int32 EffectiveValue = DubitoConstants::NoRoundValue;

	// How many actual cards are currently picked.
	UPROPERTY(BlueprintReadOnly, Category = "Dubito|Play")
	int32 SelectedCount = 0;

	// The claimed count echoed back, clamped into the presentable range for the count control.
	UPROPERTY(BlueprintReadOnly, Category = "Dubito|Play")
	int32 ClaimedCount = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Dubito|Play")
	int32 MinClaimedCount = DubitoConstants::MinCardsPerPlay;

	UPROPERTY(BlueprintReadOnly, Category = "Dubito|Play")
	int32 MaxClaimedCount = DubitoConstants::MaxCardsPerPlay;

	// The picked cards form a legal actual set (1..MaxCardsPerPlay, valid unique indices).
	UPROPERTY(BlueprintReadOnly, Category = "Dubito|Play")
	bool bSelectionValid = false;

	// The announcement (EffectiveValue + ClaimedCount) is well-formed and legal for the round.
	UPROPERTY(BlueprintReadOnly, Category = "Dubito|Play")
	bool bAnnouncementValid = false;

	// True only when a valid Play payload is ready to send to the server.
	UPROPERTY(BlueprintReadOnly, Category = "Dubito|Play")
	bool bIsSubmittable = false;

	UPROPERTY(BlueprintReadOnly, Category = "Dubito|Play")
	EDubitoPlayComposeError Error = EDubitoPlayComposeError::NotYourTurn;

	// Every hand card with its picked flag, for rendering the selectable hand.
	UPROPERTY(BlueprintReadOnly, Category = "Dubito|Play")
	TArray<FDubitoPlayCardView> Cards;

	// The Play payload. Populated (in hand order) only when bIsSubmittable is true.
	UPROPERTY(BlueprintReadOnly, Category = "Dubito|Play")
	TArray<FDubitoCard> ActualCards;

	UPROPERTY(BlueprintReadOnly, Category = "Dubito|Play")
	FDubitoAnnouncement Announcement;
};

// Pure builder. Deterministic; safe to call anywhere, including automation tests.
DUBITO_API FDubitoPlaySelectionView BuildDubitoPlaySelectionView(const FDubitoPlaySelectionInputs& Inputs);
