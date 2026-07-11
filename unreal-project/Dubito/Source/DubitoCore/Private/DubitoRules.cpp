#include "DubitoRules.h"

namespace DubitoRules
{
	void SetupMatch(FDubitoMatchState& State, const TArray<int32>& TurnOrder, const TMap<int32, FDubitoHand>& Hands)
	{
		State = FDubitoMatchState();
		State.Phase = EDubitoPhase::PlayerTurn;
		State.TurnOrder = TurnOrder;
		State.ActiveSeatIndex = 0;
		State.RoundValue = DubitoConstants::NoRoundValue;
		State.Hands = Hands;

		// Public hand ledgers start equal to the actual dealt sizes; they diverge from
		// the actual hands only once hidden plays begin.
		for (const int32 PlayerId : TurnOrder)
		{
			const FDubitoHand* Hand = Hands.Find(PlayerId);
			State.PublicHandCounts.Add(PlayerId, Hand ? Hand->Num() : 0);
			State.ConsecutiveTimeouts.Add(PlayerId, 0);
		}
	}

	bool IsAnnouncementValidForRound(const FDubitoAnnouncement& Announcement, int32 RoundValue)
	{
		if (!Announcement.IsWellFormed())
		{
			return false;
		}

		if (RoundValue != DubitoConstants::NoRoundValue)
		{
			// Round open: the claimed value is locked to the round value.
			return Announcement.ClaimedValue == RoundValue;
		}

		// Opening an empty pile: any well-formed value chooses the round.
		return true;
	}

	void AdvanceTurn(FDubitoMatchState& State)
	{
		if (State.TurnOrder.Num() == 0)
		{
			return;
		}
		State.ActiveSeatIndex = (State.ActiveSeatIndex + 1) % State.TurnOrder.Num();
	}

	void ApplyPlay(FDubitoMatchState& State, int32 PlayerId, const TArray<FDubitoCard>& ActualCards, const FDubitoAnnouncement& Announcement)
	{
		// If a win is pending, the responder choosing to Play (rather than Doubt) forfeits
		// the doubt window and confirms the pending winner; the play itself does not happen.
		if (State.HasPendingWin())
		{
			ConfirmWin(State, State.PendingWinnerId);
			return;
		}

		// Opening the round on an empty pile locks the round value to the claimed value.
		if (!State.IsRoundOpen())
		{
			State.RoundValue = Announcement.ClaimedValue;
		}

		// Remove the actual cards from the hidden hand (actual count).
		if (FDubitoHand* Hand = State.Hands.Find(PlayerId))
		{
			for (const FDubitoCard& Card : ActualCards)
			{
				Hand->Remove(Card);
			}
		}

		// Public hand ledger decreases by the CLAIMED count, clamped at zero: it never
		// exposes the actual number of cards played.
		int32& PublicCount = State.PublicHandCounts.FindOrAdd(PlayerId);
		PublicCount = FMath::Max(0, PublicCount - Announcement.ClaimedCount);

		// Hidden actual pile grows by the actual count; public claimed pile grows by the
		// claimed count. The two intentionally diverge.
		State.ActualPile.Append(ActualCards);
		State.ClaimedPileCount += Announcement.ClaimedCount;

		// Record the last play so the immediate next player may Doubt it.
		State.LastClaimantId = PlayerId;
		State.LastActualCards = ActualCards;
		State.LastAnnouncement = Announcement;
		State.bLastPlayDoubtable = true;

		// Emptying the actual hand creates a pending win, not an immediate win.
		const FDubitoHand* HandAfter = State.Hands.Find(PlayerId);
		if (HandAfter && HandAfter->IsEmpty())
		{
			State.PendingWinnerId = PlayerId;
		}

		AdvanceTurn(State);
	}

	bool CanPlay(const FDubitoMatchState& State, int32 PlayerId)
	{
		return State.Phase == EDubitoPhase::PlayerTurn && State.ActivePlayerId() == PlayerId;
	}

	bool CanDoubt(const FDubitoMatchState& State, int32 PlayerId)
	{
		return State.Phase == EDubitoPhase::PlayerTurn
			&& State.ActivePlayerId() == PlayerId
			&& State.bLastPlayDoubtable
			&& State.LastClaimantId != DubitoConstants::NoPlayerId;
	}

	bool CanDiscard(const FDubitoMatchState& State, int32 PlayerId)
	{
		return State.Phase == EDubitoPhase::PlayerTurn
			&& State.ActivePlayerId() == PlayerId
			&& !State.IsPileEmpty()
			&& !State.HasPendingWin();
	}

	EDubitoPlayValidity ValidatePlay(const FDubitoMatchState& State, int32 PlayerId, const TArray<FDubitoCard>& ActualCards, const FDubitoAnnouncement& Announcement)
	{
		if (State.Phase != EDubitoPhase::PlayerTurn)
		{
			return EDubitoPlayValidity::WrongPhase;
		}
		if (State.ActivePlayerId() != PlayerId)
		{
			return EDubitoPlayValidity::NotYourTurn;
		}
		if (ActualCards.Num() < DubitoConstants::MinCardsPerPlay || ActualCards.Num() > DubitoConstants::MaxCardsPerPlay)
		{
			return EDubitoPlayValidity::BadActualCount;
		}
		if (!Announcement.IsWellFormed())
		{
			return EDubitoPlayValidity::BadAnnouncement;
		}
		if (!IsAnnouncementValidForRound(Announcement, State.RoundValue))
		{
			// Well-formed but the value is not the locked round value.
			return EDubitoPlayValidity::ValueLocked;
		}

		// Ownership: the caller must hold every actual card. Check against a copy so
		// duplicates would be consumed correctly.
		if (const FDubitoHand* Hand = State.Hands.Find(PlayerId))
		{
			FDubitoHand Working = *Hand;
			for (const FDubitoCard& Card : ActualCards)
			{
				if (!Working.Remove(Card))
				{
					return EDubitoPlayValidity::DontOwnCards;
				}
			}
		}
		else
		{
			return EDubitoPlayValidity::DontOwnCards;
		}

		return EDubitoPlayValidity::Valid;
	}

	void ConfirmWin(FDubitoMatchState& State, int32 WinnerId)
	{
		State.WinnerId = WinnerId;
		State.PendingWinnerId = DubitoConstants::NoPlayerId;
		State.Phase = EDubitoPhase::GameOver;
	}

	bool WasLastPlayALie(const FDubitoMatchState& State)
	{
		// A lie if the actual count differs from the claimed count...
		if (State.LastActualCards.Num() != State.LastAnnouncement.ClaimedCount)
		{
			return true;
		}
		// ...or if any actual card is not the locked round value.
		for (const FDubitoCard& Card : State.LastActualCards)
		{
			if (Card.Rank != State.RoundValue)
			{
				return true;
			}
		}
		return false;
	}

	FDubitoRevealInfo ResolveDoubt(FDubitoMatchState& State, int32 DoubterId)
	{
		FDubitoRevealInfo Reveal;
		Reveal.ClaimantId = State.LastClaimantId;
		Reveal.DoubterId = DoubterId;
		Reveal.Claim = State.LastAnnouncement;
		Reveal.RevealedCards = State.LastActualCards;

		const bool bWasLie = WasLastPlayALie(State);
		Reveal.bWasLie = bWasLie;

		// The liar (on a correct doubt) or the doubter (on a wrong doubt) takes the pile.
		const int32 LoserId = bWasLie ? State.LastClaimantId : DoubterId;
		Reveal.LoserId = LoserId;
		Reveal.ClaimedStakeTransferred = State.ClaimedPileCount;

		// Move the whole hidden pile into the loser's actual hand.
		if (FDubitoHand* LoserHand = State.Hands.Find(LoserId))
		{
			LoserHand->Cards.Append(State.ActualPile);
		}
		// The loser's public ledger rises by the claimed stake, not the hidden pile size.
		int32& LoserPublic = State.PublicHandCounts.FindOrAdd(LoserId);
		LoserPublic += State.ClaimedPileCount;

		// Empty the pile and reset the round: a new round opens on the now-empty pile.
		State.ActualPile.Reset();
		State.ClaimedPileCount = 0;
		State.RoundValue = DubitoConstants::NoRoundValue;
		State.bLastPlayDoubtable = false;
		State.LastClaimantId = DubitoConstants::NoPlayerId;
		State.LastActualCards.Reset();
		State.LastAnnouncement = FDubitoAnnouncement();

		if (State.HasPendingWin())
		{
			if (bWasLie)
			{
				// Correct doubt cancels the win: the would-be winner took cards back.
				State.PendingWinnerId = DubitoConstants::NoPlayerId;
				// Doubter plays next; the active seat is already the doubter.
			}
			else
			{
				// Wrong doubt confirms the pending win.
				Reveal.bWinConfirmed = true;
				Reveal.WinnerId = State.PendingWinnerId;
				ConfirmWin(State, State.PendingWinnerId);
			}
			return Reveal;
		}

		if (bWasLie)
		{
			// Correct doubt: the liar took the pile; the doubter plays next (active unchanged).
		}
		else
		{
			// Wrong doubt: the doubter took the pile and loses the turn.
			AdvanceTurn(State);
		}

		return Reveal;
	}

	void ApplyDiscard(FDubitoMatchState& State, int32 PlayerId)
	{
		// Remove the pile from the game and clear the round; hands are not affected.
		State.ActualPile.Reset();
		State.ClaimedPileCount = 0;
		State.RoundValue = DubitoConstants::NoRoundValue;
		State.bLastPlayDoubtable = false;
		State.LastClaimantId = DubitoConstants::NoPlayerId;
		State.LastActualCards.Reset();
		State.LastAnnouncement = FDubitoAnnouncement();

		// The active player skips their turn.
		AdvanceTurn(State);
	}

	void NoteVoluntaryAction(FDubitoMatchState& State, int32 PlayerId)
	{
		if (int32* Streak = State.ConsecutiveTimeouts.Find(PlayerId))
		{
			*Streak = 0;
		}
	}

	// Picks the single card and announcement a timeout auto-plays: truthful when possible,
	// a forced minimal bluff (claiming the locked value) when no truthful option exists.
	static void SelectTimeoutPlay(const FDubitoMatchState& State, int32 PlayerId, TArray<FDubitoCard>& OutCards, FDubitoAnnouncement& OutAnnouncement)
	{
		OutCards.Reset();

		const FDubitoHand* Hand = State.Hands.Find(PlayerId);
		if (!Hand || Hand->IsEmpty())
		{
			return;
		}

		if (!State.IsRoundOpen())
		{
			// Opening: play the first card truthfully at its own value, claim count 1.
			const FDubitoCard& Card = Hand->Cards[0];
			OutCards = { Card };
			OutAnnouncement = FDubitoAnnouncement(Card.Rank, 1);
			return;
		}

		// Round open: prefer a truthful card matching the locked round value.
		for (const FDubitoCard& Card : Hand->Cards)
		{
			if (Card.Rank == State.RoundValue)
			{
				OutCards = { Card };
				OutAnnouncement = FDubitoAnnouncement(State.RoundValue, 1);
				return;
			}
		}

		// No truthful option: forced minimal bluff with the first card, claiming the locked value.
		const FDubitoCard& Card = Hand->Cards[0];
		OutCards = { Card };
		OutAnnouncement = FDubitoAnnouncement(State.RoundValue, 1);
	}

	void ResolveTimeout(FDubitoMatchState& State, int32 PlayerId)
	{
		// The timer is a turn timer: reveal/game-over phases and stale timer callbacks
		// must not mutate the authoritative rule state.
		if (State.Phase != EDubitoPhase::PlayerTurn || State.ActivePlayerId() != PlayerId)
		{
			return;
		}

		// A timeout during a pending-win response confirms the pending winner.
		if (State.HasPendingWin())
		{
			ConfirmWin(State, State.PendingWinnerId);
			return;
		}

		TArray<FDubitoCard> Cards;
		FDubitoAnnouncement Announcement;
		SelectTimeoutPlay(State, PlayerId, Cards, Announcement);
		if (Cards.Num() > 0)
		{
			ApplyPlay(State, PlayerId, Cards, Announcement);
		}

		// Count the timeout; three in a row is treated as a disconnect.
		int32& Streak = State.ConsecutiveTimeouts.FindOrAdd(PlayerId);
		++Streak;
		if (Streak >= DubitoConstants::MaxConsecutiveTimeouts)
		{
			HandleDisconnect(State, PlayerId);
		}
	}

	void HandleDisconnect(FDubitoMatchState& State, int32 PlayerId)
	{
		const int32 RemoveIndex = State.TurnOrder.IndexOfByKey(PlayerId);
		if (RemoveIndex == INDEX_NONE)
		{
			return; // not part of the match
		}

		const bool bWasActive = (State.ActivePlayerId() == PlayerId);
		const bool bWasResponder = State.HasPendingWin() && bWasActive;

		// A disconnected previous claimant's claim is no longer doubtable.
		if (State.LastClaimantId == PlayerId)
		{
			State.bLastPlayDoubtable = false;
			State.LastClaimantId = DubitoConstants::NoPlayerId;
		}

		// If the pending winner themselves leaves, the win is no longer pending.
		if (State.PendingWinnerId == PlayerId)
		{
			State.PendingWinnerId = DubitoConstants::NoPlayerId;
		}

		// Remove the player's hand and ledgers; the pile remains in play.
		State.Hands.Remove(PlayerId);
		State.PublicHandCounts.Remove(PlayerId);
		State.ConsecutiveTimeouts.Remove(PlayerId);
		State.TurnOrder.RemoveAt(RemoveIndex);

		if (State.TurnOrder.Num() == 0)
		{
			State.ActiveSeatIndex = 0;
			State.Phase = EDubitoPhase::GameOver;
			return;
		}

		// Fix the active seat: removing a seat before the active one shifts it down;
		// removing the active seat itself makes the next player active (advance turn).
		if (RemoveIndex < State.ActiveSeatIndex)
		{
			--State.ActiveSeatIndex;
		}
		State.ActiveSeatIndex %= State.TurnOrder.Num();

		// Disconnecting as the pending-win responder confirms the pending winner.
		if (bWasResponder)
		{
			ConfirmWin(State, State.PendingWinnerId);
			return;
		}

		// Last player standing wins.
		if (State.TurnOrder.Num() == 1)
		{
			ConfirmWin(State, State.TurnOrder[0]);
		}
	}
}
