#include "DubitoPlayerController.h"

#include "Net/UnrealNetwork.h"

ADubitoPlayerController::ADubitoPlayerController()
{
	SetReplicates(true);
}

void ADubitoPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ADubitoPlayerController, PrivateHand, PrivateHandReplicationCondition);
}

void ADubitoPlayerController::SetPrivateHand(const FDubitoHand& InPrivateHand)
{
	PrivateHand = InPrivateHand;
	OnPrivateHandUpdated();
}

void ADubitoPlayerController::ClearPrivateHand()
{
	PrivateHand = FDubitoHand();
	OnPrivateHandUpdated();
}

void ADubitoPlayerController::OnRep_PrivateHand()
{
	OnPrivateHandUpdated();
}
