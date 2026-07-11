#include "DubitoPlayerState.h"

#include "Net/UnrealNetwork.h"

ADubitoPlayerState::ADubitoPlayerState()
{
	SetReplicates(true);
}

void ADubitoPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ADubitoPlayerState, DubitoPlayerId);
	DOREPLIFETIME(ADubitoPlayerState, SeatIndex);
	DOREPLIFETIME(ADubitoPlayerState, bReady);
	DOREPLIFETIME(ADubitoPlayerState, PublicHandCount);
}

void ADubitoPlayerState::SetPublicIdentity(int32 InDubitoPlayerId, int32 InSeatIndex)
{
	DubitoPlayerId = InDubitoPlayerId;
	SeatIndex = InSeatIndex;
	NotifyPublicPlayerStateUpdated();
}

void ADubitoPlayerState::SetReady(bool bInReady)
{
	bReady = bInReady;
	NotifyPublicPlayerStateUpdated();
}

void ADubitoPlayerState::SetPublicHandCount(int32 InPublicHandCount)
{
	PublicHandCount = FMath::Max(0, InPublicHandCount);
	NotifyPublicPlayerStateUpdated();
}

void ADubitoPlayerState::OnRep_PublicPlayerState()
{
	NotifyPublicPlayerStateUpdated();
}

void ADubitoPlayerState::NotifyPublicPlayerStateUpdated()
{
	OnPublicPlayerStateChangedNative.Broadcast();
	OnPublicPlayerStateUpdated();
}
