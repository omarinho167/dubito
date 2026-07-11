#pragma once

#include "CoreMinimal.h"
#include "DubitoMatchState.h"
#include "DubitoAnnouncement.h"
#include "DubitoCard.h"
#include "DubitoReveal.h"

// Why a proposed Play is or is not valid. Ordinary illegal gameplay uses these
// reasons for controlled rejection; it is not treated as an abusive payload.
enum class EDubitoPlayValidity : uint8
{
	Valid,
	WrongPhase,     // not in an active player turn
	NotYourTurn,    // caller is not the active player
	BadActualCount, // number of actual cards is outside 1..4
	BadAnnouncement,// claimed count/value is not well-formed
	ValueLocked,    // claimed value does not match the locked round value
	DontOwnCards    // caller does not hold all of the actual cards
};

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

	// --- Action legality / availability (Phase 2.2) ---
	// These predicates answer the Action Matrix in docs/DESIGN.md: whether a given
	// action is currently available to a player, independent of the action's contents.

	// Play is available whenever it is the caller's active turn (including the empty
	// pile that opens a round, and the pending-win window).
	DUBITOCORE_API bool CanPlay(const FDubitoMatchState& State, int32 PlayerId);

	// Doubt is available only to the immediate next player (the active player) and only
	// while there is a doubtable previous claim.
	DUBITOCORE_API bool CanDoubt(const FDubitoMatchState& State, int32 PlayerId);

	// Discard is available only to the active player, only on a non-empty pile, and
	// never while a win is pending.
	DUBITOCORE_API bool CanDiscard(const FDubitoMatchState& State, int32 PlayerId);

	// Full validation of a proposed Play's contents (turn, actual count 1..4, announcement
	// legal for the round, and ownership of every actual card). Returns Valid or the reason.
	DUBITOCORE_API EDubitoPlayValidity ValidatePlay(const FDubitoMatchState& State, int32 PlayerId, const TArray<FDubitoCard>& ActualCards, const FDubitoAnnouncement& Announcement);

	// --- Doubt resolution & win confirmation (Phase 2.3) ---

	// True if the recorded last play is a lie: its actual count differs from the claimed
	// count, or any actual card's rank is not the locked round value.
	DUBITOCORE_API bool WasLastPlayALie(const FDubitoMatchState& State);

	// Resolves a Doubt by the active player against the recorded last play. Transfers the
	// whole pile to the loser (actual cards to the hidden hand; public ledger by the
	// claimed stake), empties the pile, resets the round, and sets the next turn: on a
	// correct doubt the doubter plays next; on a wrong doubt the doubter loses the turn.
	// Resolves any pending win (correct doubt cancels it, wrong doubt confirms it).
	// Returns a self-contained reveal payload.
	DUBITOCORE_API FDubitoRevealInfo ResolveDoubt(FDubitoMatchState& State, int32 DoubterId);

	// Confirms WinnerId as the winner and moves the match to GameOver.
	DUBITOCORE_API void ConfirmWin(FDubitoMatchState& State, int32 WinnerId);
}
