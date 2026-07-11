#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "DubitoConstants.h"
#include "DubitoPlayerState.generated.h"

// Native broadcast fired whenever this player's replicated public state changes, on both
// the authoritative sync and the client OnRep. Lets the table HUD refresh seat ledgers.
DECLARE_MULTICAST_DELEGATE(FDubitoPublicPlayerStateChanged);

/**
 * Replicated public per-player state for Phase 4.2.
 *
 * Exact private hand contents are intentionally not stored here. PublicHandCount
 * is a display/stake ledger and may diverge from the actual server hand.
 */
UCLASS()
class DUBITO_API ADubitoPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	ADubitoPlayerState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void SetPublicIdentity(int32 InDubitoPlayerId, int32 InSeatIndex);
	void SetReady(bool bInReady);
	void SetPublicHandCount(int32 InPublicHandCount);

	UFUNCTION(BlueprintPure, Category = "Dubito|Public Player")
	int32 GetDubitoPlayerId() const { return DubitoPlayerId; }

	UFUNCTION(BlueprintPure, Category = "Dubito|Public Player")
	int32 GetSeatIndex() const { return SeatIndex; }

	UFUNCTION(BlueprintPure, Category = "Dubito|Public Player")
	bool IsReady() const { return bReady; }

	UFUNCTION(BlueprintPure, Category = "Dubito|Public Player")
	int32 GetPublicHandCount() const { return PublicHandCount; }

	// Subscribe to be notified (server sync + client OnRep) when public player state changes.
	FDubitoPublicPlayerStateChanged OnPublicPlayerStateChangedNative;

protected:
	UFUNCTION()
	void OnRep_PublicPlayerState();

	UFUNCTION(BlueprintImplementableEvent, Category = "Dubito|Public Player")
	void OnPublicPlayerStateUpdated();

private:
	void NotifyPublicPlayerStateUpdated();

	UPROPERTY(ReplicatedUsing = OnRep_PublicPlayerState, BlueprintReadOnly, Category = "Dubito|Public Player", meta = (AllowPrivateAccess = "true"))
	int32 DubitoPlayerId = DubitoConstants::NoPlayerId;

	UPROPERTY(ReplicatedUsing = OnRep_PublicPlayerState, BlueprintReadOnly, Category = "Dubito|Public Player", meta = (AllowPrivateAccess = "true"))
	int32 SeatIndex = INDEX_NONE;

	UPROPERTY(ReplicatedUsing = OnRep_PublicPlayerState, BlueprintReadOnly, Category = "Dubito|Public Player", meta = (AllowPrivateAccess = "true"))
	bool bReady = false;

	UPROPERTY(ReplicatedUsing = OnRep_PublicPlayerState, BlueprintReadOnly, Category = "Dubito|Public Player", meta = (AllowPrivateAccess = "true"))
	int32 PublicHandCount = 0;
};
