#pragma once

#include "CoreMinimal.h"
#include "DubitoCard.h"
#include "DubitoHand.h"
#include "DubitoConstants.h"
#include "DubitoAnnouncement.h"
#include "DubitoMatchState.generated.h"

// Replicated public phases (see docs/ARCHITECTURE.md state machine).
UENUM(BlueprintType)
enum class EDubitoPhase : uint8
{
	Lobby,
	Dealing,
	PlayerTurn,
	DoubtReveal,
	GameOver
};

// Authoritative, server-side match model held by DubitoCore. It carries BOTH the
// hidden actual state (hands, actual pile, actual last-played cards) and the public
// ledgers (claimed pile count, public hand counts). The network layer decides what
// each client may observe; the rules model itself keeps everything so it can resolve
// authoritatively. This is a plain struct on purpose: it never replicates as one blob.
struct FDubitoMatchState
{
	EDubitoPhase Phase = EDubitoPhase::Lobby;

	// Turn order as player ids; ActiveSeatIndex indexes into it.
	TArray<int32> TurnOrder;
	int32 ActiveSeatIndex = 0;

	// The locked round value, or NoRoundValue when the pile is empty.
	int32 RoundValue = DubitoConstants::NoRoundValue;

	// --- Hidden / server-only actual state ---
	TMap<int32, FDubitoHand> Hands;
	TArray<FDubitoCard> ActualPile;
	int32 LastClaimantId = DubitoConstants::NoPlayerId;
	TArray<FDubitoCard> LastActualCards;
	FDubitoAnnouncement LastAnnouncement;
	bool bLastPlayDoubtable = false;

	// --- Public ledgers (display/stake, not proof of hidden counts) ---
	int32 ClaimedPileCount = 0;
	TMap<int32, int32> PublicHandCounts;

	// --- Pending win ---
	int32 PendingWinnerId = DubitoConstants::NoPlayerId;

	// --- Anti-AFK ---
	TMap<int32, int32> ConsecutiveTimeouts;

	int32 ActivePlayerId() const
	{
		return TurnOrder.IsValidIndex(ActiveSeatIndex) ? TurnOrder[ActiveSeatIndex] : DubitoConstants::NoPlayerId;
	}

	bool IsRoundOpen() const { return RoundValue != DubitoConstants::NoRoundValue; }
	bool IsPileEmpty() const { return ActualPile.Num() == 0; }
	bool HasPendingWin() const { return PendingWinnerId != DubitoConstants::NoPlayerId; }
};
