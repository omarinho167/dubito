#include "DubitoGameInstance.h"

#include "Engine/World.h"
#include "GameFramework/PlayerController.h"

FString UDubitoGameInstance::BuildHostTravelUrl() const
{
	// ?listen makes the opened map a listen server that accepts remote connections.
	return TableMapPath + TEXT("?listen");
}

void UDubitoGameInstance::HostListenServer()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// ServerTravel from a standalone game opens the Table map as a listen server. The Table map's
	// GameMode (ADubitoGameMode) then seats the host and any joining clients in the lobby.
	World->ServerTravel(BuildHostTravelUrl());
}

void UDubitoGameInstance::JoinGameAt(const FString& Address)
{
	if (Address.IsEmpty())
	{
		return;
	}

	if (APlayerController* LocalController = GetFirstLocalPlayerController())
	{
		// Absolute client travel connects this instance to the host and loads its current map.
		LocalController->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
	}
}

void UDubitoGameInstance::JoinGameDefault()
{
	JoinGameAt(DefaultJoinAddress);
}

void UDubitoGameInstance::ReturnToMenu()
{
	// Built-in teardown: disconnects a client or ends the listen server and travels the local
	// player back to the default (Main Menu) map.
	ReturnToMainMenu();
}
