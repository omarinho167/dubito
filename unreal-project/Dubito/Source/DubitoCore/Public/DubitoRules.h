#pragma once

#include "CoreMinimal.h"
#include "DubitoMatchState.h"
#include "DubitoAnnouncement.h"
#include "DubitoCard.h"

// Pure rule functions operating on FDubitoMatchState. The server (GameMode) owns a
// match state and routes all rule decisions through these functions. Nothing here
// reads UI, actors, or networking.
namespace DubitoRules
{
	// --- Setup & ledgers (Phase 2.1) ---

	// Initialise a match from a turn order and dealt hands: sets public hand ledgers
	// from actual hand sizes, clears the pile/round, and enters the PlayerTurn phase.
	DUBITOCORE_API void SetupMatch(FDubitoMatchState& State, const TArray<int32>& TurnOrder, const TMap<int32, FDubitoHand>& Hands);

	// True if an announcement is well-formed and legal for the current round value:
	// the claimed value is free when opening an empty pile, but locked to RoundValue
	// once a round is open. The claimed count (1..4) is always free to bluff.
	DUBITOCORE_API bool IsAnnouncementValidForRound(const FDubitoAnnouncement& Announcement, int32 RoundValue);

	// Advances the active seat to the next player in turn order (wrapping).
	DUBITOCORE_API void AdvanceTurn(FDubitoMatchState& State);

	// Applies a play to the state, assuming it has already been validated as legal.
	// Updates hidden actual state (hand, actual pile) and public ledgers (claimed pile,
	// public hand count) independently, locks the round value when opening, records the
	// last play for a possible Doubt, flags a pending win if the hand empties, and
	// advances the turn.
	DUBITOCORE_API void ApplyPlay(FDubitoMatchState& State, int32 PlayerId, const TArray<FDubitoCard>& ActualCards, const FDubitoAnnouncement& Announcement);
}
