#pragma once

#include "CoreMinimal.h"
#include "DubitoConstants.h"
#include "DubitoGameOver.generated.h"

// Why a match ended. Carried by the self-contained public game-over event so a client can
// present the terminal result without guessing hidden state.
UENUM(BlueprintType)
enum class EDubitoGameOverReason : uint8
{
	Unknown,
	PendingWinDoubtFailed,
	PendingWinDeclined,
	PendingWinTimeout,
	PendingWinResponderDisconnected,
	LastPlayerStanding,
	NoPlayersRemaining
};

// Self-contained public game-over payload: the confirmed winner (or NoPlayerId when none
// remain) and the reason the match ended.
USTRUCT(BlueprintType)
struct DUBITO_API FDubitoGameOverInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Dubito|Public Events")
	int32 WinnerId = DubitoConstants::NoPlayerId;

	UPROPERTY(BlueprintReadOnly, Category = "Dubito|Public Events")
	EDubitoGameOverReason Reason = EDubitoGameOverReason::Unknown;
};
