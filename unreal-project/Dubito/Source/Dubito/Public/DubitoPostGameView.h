#pragma once

#include "CoreMinimal.h"
#include "DubitoConstants.h"
#include "DubitoGameOver.h"
#include "DubitoPostGameView.generated.h"

/**
 * Phase 5.5 post-game presentation view-model.
 *
 * Pure derivation of the terminal result from the self-contained public `FDubitoGameOverInfo`
 * plus the local player's identity: who won, why the match ended, and whether the local player
 * won, lost, or only watched. It reads only the scrubbed public payload and holds no rule
 * authority.
 */

// The terminal result from the local player's point of view.
UENUM(BlueprintType)
enum class EDubitoPostGameResult : uint8
{
	None,       // no valid game-over to present
	LocalWin,   // the local player is the confirmed winner
	LocalLoss,  // someone else won
	NoWinner,   // the match ended with no winner (all players gone)
	Spectator   // no local player identity (an observer)
};

USTRUCT(BlueprintType)
struct DUBITO_API FDubitoPostGameView
{
	GENERATED_BODY()

	// False until a real game-over has been presented.
	UPROPERTY(BlueprintReadOnly, Category = "Dubito|Post Game")
	bool bValid = false;

	UPROPERTY(BlueprintReadOnly, Category = "Dubito|Post Game")
	int32 WinnerId = DubitoConstants::NoPlayerId;

	UPROPERTY(BlueprintReadOnly, Category = "Dubito|Post Game")
	bool bHasWinner = false;

	UPROPERTY(BlueprintReadOnly, Category = "Dubito|Post Game")
	EDubitoGameOverReason Reason = EDubitoGameOverReason::Unknown;

	UPROPERTY(BlueprintReadOnly, Category = "Dubito|Post Game")
	EDubitoPostGameResult Result = EDubitoPostGameResult::None;

	UPROPERTY(BlueprintReadOnly, Category = "Dubito|Post Game")
	bool bLocalIsWinner = false;
};

// Pure builder. Deterministic; safe to call anywhere, including automation tests.
DUBITO_API FDubitoPostGameView BuildDubitoPostGameView(const FDubitoGameOverInfo& GameOver, int32 LocalPlayerId);
