#pragma once

#include "CoreMinimal.h"
#include "DubitoConstants.h"
#include "DubitoMatchState.h"
#include "DubitoLobbyView.generated.h"

/**
 * Phase 5.6 local-session lobby view-model.
 *
 * Pure derivation of the pre-match lobby a player sees on the Table map before the deal: who is
 * seated, who is ready, whether the local player is the host, and whether Start is available (with
 * a specific blocked reason so a disabled Start button can explain itself). It mirrors the server
 * start predicate (StartAuthoritativeMatchFromRegisteredPlayers: 2..8 seated players, all ready)
 * from public seat/ready state only.
 *
 * It holds no rule authority; the server still validates the Start and the ready toggles. Seat 0 is
 * the host (the listen server's local player, first to log in on the Table map).
 */

// Why Start is unavailable, in the order the view resolves them.
UENUM(BlueprintType)
enum class EDubitoLobbyStartBlockedReason : uint8
{
	None,               // Start is available
	AlreadyStarted,     // the match already left the lobby
	NotHost,            // only the host (seat 0) may start
	WaitingForPlayers,  // fewer than the minimum (or more than the maximum) seated players
	NotAllReady         // at least one seated player is not ready yet
};

// One seated player as seen from public state.
struct FDubitoLobbySeatInput
{
	int32 PlayerId = DubitoConstants::NoPlayerId;
	int32 SeatIndex = INDEX_NONE;
	bool bReady = false;
	bool bIsLocal = false;
};

struct FDubitoLobbyInputs
{
	EDubitoPhase Phase = EDubitoPhase::Lobby;
	TArray<FDubitoLobbySeatInput> Seats;

	// Whether the local instance is the listen-server host. Derived from network authority, not
	// from a seat index, so it does not depend on join order.
	bool bLocalIsHost = false;
};

USTRUCT(BlueprintType)
struct DUBITO_API FDubitoLobbySeatView
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Dubito|Lobby")
	int32 PlayerId = DubitoConstants::NoPlayerId;

	UPROPERTY(BlueprintReadOnly, Category = "Dubito|Lobby")
	int32 SeatIndex = INDEX_NONE;

	UPROPERTY(BlueprintReadOnly, Category = "Dubito|Lobby")
	bool bReady = false;

	UPROPERTY(BlueprintReadOnly, Category = "Dubito|Lobby")
	bool bIsLocal = false;

	UPROPERTY(BlueprintReadOnly, Category = "Dubito|Lobby")
	bool bIsHost = false;
};

USTRUCT(BlueprintType)
struct DUBITO_API FDubitoLobbyView
{
	GENERATED_BODY()

	// True once a local seat is present; the widget has nothing to show otherwise.
	UPROPERTY(BlueprintReadOnly, Category = "Dubito|Lobby")
	bool bValid = false;

	// True while the match is still in the lobby; drives whether the lobby panel is shown at all.
	UPROPERTY(BlueprintReadOnly, Category = "Dubito|Lobby")
	bool bInLobby = false;

	// Seats sorted by seat index (host first).
	UPROPERTY(BlueprintReadOnly, Category = "Dubito|Lobby")
	TArray<FDubitoLobbySeatView> Seats;

	UPROPERTY(BlueprintReadOnly, Category = "Dubito|Lobby")
	int32 PlayerCount = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Dubito|Lobby")
	int32 ReadyCount = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Dubito|Lobby")
	int32 LocalSeatIndex = INDEX_NONE;

	UPROPERTY(BlueprintReadOnly, Category = "Dubito|Lobby")
	bool bLocalReady = false;

	UPROPERTY(BlueprintReadOnly, Category = "Dubito|Lobby")
	bool bIsLocalHost = false;

	UPROPERTY(BlueprintReadOnly, Category = "Dubito|Lobby")
	bool bCanStart = false;

	UPROPERTY(BlueprintReadOnly, Category = "Dubito|Lobby")
	EDubitoLobbyStartBlockedReason StartBlockedReason = EDubitoLobbyStartBlockedReason::WaitingForPlayers;
};

// Pure builder. Deterministic; safe to call anywhere, including automation tests.
DUBITO_API FDubitoLobbyView BuildDubitoLobbyView(const FDubitoLobbyInputs& Inputs);
