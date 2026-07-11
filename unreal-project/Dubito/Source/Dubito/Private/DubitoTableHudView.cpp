#include "DubitoTableHudView.h"

#include "Math/UnrealMathUtility.h"

namespace
{
	bool IsActiveTurnPhase(EDubitoPhase Phase)
	{
		return Phase == EDubitoPhase::PlayerTurn;
	}

	bool IsMatchInProgress(EDubitoPhase Phase)
	{
		return Phase == EDubitoPhase::PlayerTurn || Phase == EDubitoPhase::DoubtReveal;
	}
}

FDubitoTableHudSnapshot BuildDubitoTableHudSnapshot(const FDubitoTableHudInputs& Inputs)
{
	FDubitoTableHudSnapshot Snapshot;

	Snapshot.Phase = Inputs.Phase;
	Snapshot.bMatchInProgress = IsMatchInProgress(Inputs.Phase);
	Snapshot.ActivePlayerId = Inputs.ActivePlayerId;

	const bool bHasLocalPlayer = Inputs.LocalPlayerId != DubitoConstants::NoPlayerId;
	const bool bIsLocalActive = bHasLocalPlayer && Inputs.ActivePlayerId == Inputs.LocalPlayerId;
	Snapshot.bIsLocalPlayerActive = bIsLocalActive;

	Snapshot.RoundValue = Inputs.RoundValue;
	Snapshot.bHasRoundValue = Inputs.RoundValue != DubitoConstants::NoRoundValue;

	Snapshot.bHasPublicClaim = Inputs.bHasPreviousPublicClaim && Inputs.PreviousClaimantId != DubitoConstants::NoPlayerId;
	Snapshot.bClaimIsDoubtable = Snapshot.bHasPublicClaim && Inputs.bPreviousClaimDoubtable;
	Snapshot.ClaimantId = Snapshot.bHasPublicClaim ? Inputs.PreviousClaimantId : DubitoConstants::NoPlayerId;
	Snapshot.PublicClaim = Snapshot.bHasPublicClaim ? Inputs.PreviousPublicAnnouncement : FDubitoAnnouncement();

	Snapshot.ClaimedPileCount = Inputs.ClaimedPileCount;
	Snapshot.bHasPendingWin = Inputs.bHasPendingWin;
	Snapshot.LocalHandCount = Inputs.LocalHandCount;

	// Action availability mirrors the server's DubitoRules predicates using only the
	// public state a client actually holds:
	//   Play    : active player's turn, with cards in hand (allowed during pending win).
	//   Doubt   : active player, only while the previous claim is doubtable.
	//   Discard : active player, only on a non-empty pile (ClaimedPileCount>0 <=> pile
	//             non-empty), and never while a win is pending.
	const bool bActiveTurn = IsActiveTurnPhase(Inputs.Phase);
	Snapshot.bCanAttemptPlay = bIsLocalActive && bActiveTurn && Inputs.LocalHandCount > 0;
	Snapshot.bCanAttemptDoubt = bIsLocalActive && bActiveTurn && Snapshot.bClaimIsDoubtable;
	Snapshot.bCanAttemptDiscard = bIsLocalActive && bActiveTurn && Inputs.ClaimedPileCount > 0 && !Inputs.bHasPendingWin;

	// Turn timer: only meaningful during an active turn with a real deadline set.
	const bool bHasDeadline = bActiveTurn && Inputs.TurnDeadlineServerTimeSeconds > 0.0;
	if (bHasDeadline)
	{
		const double Remaining = Inputs.TurnDeadlineServerTimeSeconds - Inputs.NowServerTimeSeconds;
		Snapshot.TurnSecondsRemaining = FMath::Max(0, FMath::CeilToInt(Remaining));
		Snapshot.bHasTurnTimer = true;
	}
	else
	{
		Snapshot.TurnSecondsRemaining = 0;
		Snapshot.bHasTurnTimer = false;
	}

	Snapshot.Seats.Reserve(Inputs.Seats.Num());
	for (const FDubitoSeatLedgerInput& SeatInput : Inputs.Seats)
	{
		FDubitoSeatLedgerView SeatView;
		SeatView.PlayerId = SeatInput.PlayerId;
		SeatView.SeatIndex = SeatInput.SeatIndex;
		SeatView.DisplayName = FText::FromString(SeatInput.DisplayName);
		SeatView.PublicHandCount = SeatInput.PublicHandCount;
		SeatView.bReady = SeatInput.bReady;
		SeatView.bIsActive = SeatInput.PlayerId != DubitoConstants::NoPlayerId && SeatInput.PlayerId == Inputs.ActivePlayerId;
		SeatView.bIsLocal = bHasLocalPlayer && SeatInput.PlayerId == Inputs.LocalPlayerId;
		Snapshot.Seats.Add(MoveTemp(SeatView));
	}

	Snapshot.Seats.Sort([](const FDubitoSeatLedgerView& A, const FDubitoSeatLedgerView& B)
	{
		return A.SeatIndex < B.SeatIndex;
	});

	return Snapshot;
}
