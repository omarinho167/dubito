#include "DubitoGameState.h"

#include "DubitoConstants.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"

ADubitoGameState::ADubitoGameState()
{
	SetReplicates(true);
}

void ADubitoGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ADubitoGameState, PublicPhase);
	DOREPLIFETIME(ADubitoGameState, ActivePlayerId);
	DOREPLIFETIME(ADubitoGameState, RoundValue);
	DOREPLIFETIME(ADubitoGameState, bHasPreviousPublicClaim);
	DOREPLIFETIME(ADubitoGameState, PreviousClaimantId);
	DOREPLIFETIME(ADubitoGameState, PreviousPublicAnnouncement);
	DOREPLIFETIME(ADubitoGameState, ClaimedPileCount);
	DOREPLIFETIME(ADubitoGameState, bHasPendingWin);
	DOREPLIFETIME(ADubitoGameState, TurnDeadlineServerTimeSeconds);
}

void ADubitoGameState::SyncFromAuthoritativeState(const FDubitoMatchState& State, double InTurnDeadlineServerTimeSeconds)
{
	PublicPhase = State.Phase;
	ActivePlayerId = State.ActivePlayerId();
	RoundValue = State.RoundValue;
	ClaimedPileCount = State.ClaimedPileCount;
	bHasPendingWin = State.HasPendingWin();
	TurnDeadlineServerTimeSeconds = InTurnDeadlineServerTimeSeconds;

	bHasPreviousPublicClaim = State.LastClaimantId != DubitoConstants::NoPlayerId && State.LastAnnouncement.IsWellFormed();
	PreviousClaimantId = bHasPreviousPublicClaim ? State.LastClaimantId : DubitoConstants::NoPlayerId;
	PreviousPublicAnnouncement = bHasPreviousPublicClaim ? State.LastAnnouncement : FDubitoAnnouncement();

	OnPublicMatchStateUpdated();
}

void ADubitoGameState::PublishPublicReveal(const FDubitoRevealInfo& Reveal)
{
	const UWorld* World = GetWorld();
	if (World && World->GetNetDriver())
	{
		MulticastPublicReveal(Reveal);
	}
	else
	{
		MulticastPublicReveal_Implementation(Reveal);
	}
}

void ADubitoGameState::PublishGameOver(const FDubitoGameOverInfo& GameOver)
{
	const UWorld* World = GetWorld();
	if (World && World->GetNetDriver())
	{
		MulticastGameOver(GameOver);
	}
	else
	{
		MulticastGameOver_Implementation(GameOver);
	}
}

void ADubitoGameState::MulticastPublicReveal_Implementation(const FDubitoRevealInfo& Reveal)
{
	LastPublicReveal = Reveal;
	++PublicRevealEventCount;
	OnPublicReveal(Reveal);
}

void ADubitoGameState::MulticastGameOver_Implementation(const FDubitoGameOverInfo& GameOver)
{
	LastGameOver = GameOver;
	++GameOverEventCount;
	OnGameOver(GameOver);
}

void ADubitoGameState::OnRep_PublicMatchState()
{
	OnPublicMatchStateUpdated();
}
