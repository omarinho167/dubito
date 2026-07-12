#pragma once

#include "CoreMinimal.h"
#include "DubitoConstants.h"
#include "DubitoMatchState.h"
#include "DubitoDiscardView.generated.h"

/**
 * Phase 5.3 Discard availability view-model.
 *
 * Pure derivation of whether the local player may Discard right now, plus a specific blocked
 * reason so a disabled Discard control can explain itself (Documentation/DESIGN.md: disabled
 * actions must explain why). It mirrors DubitoRules::CanDiscard from public state only: the
 * active player, on a player turn, on a non-empty pile, and never while a win is pending.
 *
 * It holds no rule authority; the server still validates the Discard and resyncs on a stale
 * request.
 */

// Why Discard is unavailable, in the order the view resolves them.
UENUM(BlueprintType)
enum class EDubitoDiscardBlockedReason : uint8
{
	None,        // Discard is available
	NotYourTurn, // not the local player's active turn
	PendingWin,  // a win is pending, so the pile cannot be discarded
	EmptyPile    // the pile is empty, so there is nothing to discard
};

struct FDubitoDiscardInputs
{
	EDubitoPhase Phase = EDubitoPhase::Lobby;
	bool bIsLocalPlayerActive = false;
	int32 ClaimedPileCount = 0;
	bool bHasPendingWin = false;
};

USTRUCT(BlueprintType)
struct DUBITO_API FDubitoDiscardView
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Dubito|Discard")
	bool bCanDiscard = false;

	UPROPERTY(BlueprintReadOnly, Category = "Dubito|Discard")
	EDubitoDiscardBlockedReason BlockedReason = EDubitoDiscardBlockedReason::NotYourTurn;
};

// Pure builder. Deterministic; safe to call anywhere, including automation tests.
DUBITO_API FDubitoDiscardView BuildDubitoDiscardView(const FDubitoDiscardInputs& Inputs);
