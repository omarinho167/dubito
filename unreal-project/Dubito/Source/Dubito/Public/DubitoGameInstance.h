#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "DubitoGameInstance.generated.h"

/**
 * Phase 5.6 local-session transport.
 *
 * Owns the same-PC loopback host/join flow that connects the Main Menu to a listen-server Table
 * match and back. This is the pre-Steam local path: it uses a direct listen server plus an IP
 * connect (default 127.0.0.1), and deliberately does NOT create an Online Subsystem session.
 * Steam lobbies/sessions are Phase 6.
 *
 * The listen host opens the Table map with ?listen and becomes the authority; a joining client
 * connects to the host by address. Return-to-menu tears the session down through the built-in
 * UGameInstance::ReturnToMainMenu path.
 */
UCLASS()
class DUBITO_API UDubitoGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	// Become a listen server and travel to the Table map so remote clients can join.
	UFUNCTION(BlueprintCallable, Category = "Dubito|Session")
	void HostListenServer();

	// Connect this instance to a host at the given address (host:port or bare IP).
	UFUNCTION(BlueprintCallable, Category = "Dubito|Session")
	void JoinGameAt(const FString& Address);

	// Convenience: join the default loopback address for same-PC testing.
	UFUNCTION(BlueprintCallable, Category = "Dubito|Session")
	void JoinGameDefault();

	// Leave the current session (host or client) and return to the Main Menu map.
	UFUNCTION(BlueprintCallable, Category = "Dubito|Session")
	void ReturnToMenu();

	UFUNCTION(BlueprintPure, Category = "Dubito|Session")
	FString GetDefaultJoinAddress() const { return DefaultJoinAddress; }

private:
	FString BuildHostTravelUrl() const;

	// The Table map to host the match on. The ?listen option is appended at host time.
	UPROPERTY(EditDefaultsOnly, Category = "Dubito|Session")
	FString TableMapPath = TEXT("/Game/Maps/Table");

	// Default same-PC loopback address a second local instance connects to.
	UPROPERTY(EditDefaultsOnly, Category = "Dubito|Session")
	FString DefaultJoinAddress = TEXT("127.0.0.1");
};
