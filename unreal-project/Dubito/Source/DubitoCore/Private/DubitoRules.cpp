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
}
