#pragma once

#include "CoreMinimal.h"
#include "DubitoAnnouncement.h"
#include "DubitoConstants.h"
#include "DubitoMatchState.h"
#include "DubitoTableHudView.generated.h"

/**
 * Phase 5.0 table HUD presentation view-model.
 *
 * This is pure presentation derivation: it turns the replicated PUBLIC match state
 * plus the local player's own identity and hand size into an answer to the four
 * questions the table HUD must always answer -- whose turn is it, what claim can be
 * judged, what actions can the local player attempt, and what public stake is shown.
 *
 * It never reads hidden state and it is not rule authority: the action-availability
 * flags mirror the server's legality predicates as a best-effort client gate. The
 * server still validates every action and issues a controlled rejection/resync when a
 * client attempts something that is stale by the time it arrives.
 */

// One seat's public ledger, as fed in from an ADubitoPlayerState.
struct FDubitoSeatLedgerInput
{
	int32 PlayerId = DubitoConstants::NoPlayerId;
	int32 SeatIndex = INDEX_NONE;
	FString DisplayName;
	int32 PublicHandCount = 0;
	bool bReady = false;
};

// Everything the builder needs, expressed as plain values so it is testable without
// actors, a world, or networking.
struct FDubitoTableHudInputs
{
	EDubitoPhase Phase = EDubitoPhase::Lobby;
	int32 ActivePlayerId = DubitoConstants::NoPlayerId;
	int32 RoundValue = DubitoConstants::NoRoundValue;
	bool bHasPreviousPublicClaim = false;
	bool bPreviousClaimDoubtable = false;
	int32 PreviousClaimantId = DubitoConstants::NoPlayerId;
	FDubitoAnnouncement PreviousPublicAnnouncement;
	int32 ClaimedPileCount = 0;
	bool bHasPendingWin = false;
	int32 LocalPlayerId = DubitoConstants::NoPlayerId;
	int32 LocalHandCount = 0;
	double TurnDeadlineServerTimeSeconds = 0.0;
	double NowServerTimeSeconds = 0.0;
	TArray<FDubitoSeatLedgerInput> Seats;
};

// One seat as the HUD should present it: the public ledger plus who is active / local.
USTRUCT(BlueprintType)
struct DUBITO_API FDubitoSeatLedgerView
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Dubito|Table HUD")
	int32 PlayerId = DubitoConstants::NoPlayerId;

	UPROPERTY(BlueprintReadOnly, Category = "Dubito|Table HUD")
	int32 SeatIndex = INDEX_NONE;

	UPROPERTY(BlueprintReadOnly, Category = "Dubito|Table HUD")
	FText DisplayName;

	UPROPERTY(BlueprintReadOnly, Category = "Dubito|Table HUD")
	int32 PublicHandCount = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Dubito|Table HUD")
	bool bReady = false;

	UPROPERTY(BlueprintReadOnly, Category = "Dubito|Table HUD")
	bool bIsActive = false;

	UPROPERTY(BlueprintReadOnly, Category = "Dubito|Table HUD")
	bool bIsLocal = false;
};

// The complete table HUD snapshot for one frame of one client.
USTRUCT(BlueprintType)
struct DUBITO_API FDubitoTableHudSnapshot
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Dubito|Table HUD")
	EDubitoPhase Phase = EDubitoPhase::Lobby;

	// True while the public phase is an active turn or a doubt reveal.
	UPROPERTY(BlueprintReadOnly, Category = "Dubito|Table HUD")
	bool bMatchInProgress = false;

	// Whose turn it is, and whether that is the local player.
	UPROPERTY(BlueprintReadOnly, Category = "Dubito|Table HUD")
	int32 ActivePlayerId = DubitoConstants::NoPlayerId;

	UPROPERTY(BlueprintReadOnly, Category = "Dubito|Table HUD")
	bool bIsLocalPlayerActive = false;

	// The locked round value, if a round is open.
	UPROPERTY(BlueprintReadOnly, Category = "Dubito|Table HUD")
	int32 RoundValue = DubitoConstants::NoRoundValue;

	UPROPERTY(BlueprintReadOnly, Category = "Dubito|Table HUD")
	bool bHasRoundValue = false;

	// The previous public claim -- the thing the next player may judge.
	UPROPERTY(BlueprintReadOnly, Category = "Dubito|Table HUD")
	bool bHasPublicClaim = false;

	// Whether that claim can actually be doubted right now (false for a stale claim).
	UPROPERTY(BlueprintReadOnly, Category = "Dubito|Table HUD")
	bool bClaimIsDoubtable = false;

	UPROPERTY(BlueprintReadOnly, Category = "Dubito|Table HUD")
	int32 ClaimantId = DubitoConstants::NoPlayerId;

	UPROPERTY(BlueprintReadOnly, Category = "Dubito|Table HUD")
	FDubitoAnnouncement PublicClaim;

	// The public stake on the pile (claimed count ledger, not hidden pile size).
	UPROPERTY(BlueprintReadOnly, Category = "Dubito|Table HUD")
	int32 ClaimedPileCount = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Dubito|Table HUD")
	bool bHasPendingWin = false;

	// The local player's own exact hand size.
	UPROPERTY(BlueprintReadOnly, Category = "Dubito|Table HUD")
	int32 LocalHandCount = 0;

	// Best-effort client gate for which actions the local player can attempt.
	UPROPERTY(BlueprintReadOnly, Category = "Dubito|Table HUD")
	bool bCanAttemptPlay = false;

	UPROPERTY(BlueprintReadOnly, Category = "Dubito|Table HUD")
	bool bCanAttemptDoubt = false;

	UPROPERTY(BlueprintReadOnly, Category = "Dubito|Table HUD")
	bool bCanAttemptDiscard = false;

	// Whole seconds left on the active turn, and whether a turn timer is running.
	UPROPERTY(BlueprintReadOnly, Category = "Dubito|Table HUD")
	int32 TurnSecondsRemaining = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Dubito|Table HUD")
	bool bHasTurnTimer = false;

	// Public seat ledgers, sorted by seat index.
	UPROPERTY(BlueprintReadOnly, Category = "Dubito|Table HUD")
	TArray<FDubitoSeatLedgerView> Seats;
};

// Pure builder. Deterministic; safe to call anywhere, including automation tests.
DUBITO_API FDubitoTableHudSnapshot BuildDubitoTableHudSnapshot(const FDubitoTableHudInputs& Inputs);
