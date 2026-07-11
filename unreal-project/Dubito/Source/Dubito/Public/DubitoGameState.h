#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "DubitoAnnouncement.h"
#include "DubitoMatchState.h"
#include "DubitoGameState.generated.h"

/**
 * Replicated public match state for Phase 4.1.
 *
 * This class intentionally carries only public, persistent state. Hidden hands,
 * actual played cards, and actual pile contents stay in ADubitoGameMode.
 */
UCLASS()
class DUBITO_API ADubitoGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	ADubitoGameState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void SyncFromAuthoritativeState(const FDubitoMatchState& State, double InTurnDeadlineServerTimeSeconds);

	UFUNCTION(BlueprintPure, Category = "Dubito|Public State")
	EDubitoPhase GetPublicPhase() const { return PublicPhase; }

	UFUNCTION(BlueprintPure, Category = "Dubito|Public State")
	int32 GetActivePlayerId() const { return ActivePlayerId; }

	UFUNCTION(BlueprintPure, Category = "Dubito|Public State")
	int32 GetRoundValue() const { return RoundValue; }

	UFUNCTION(BlueprintPure, Category = "Dubito|Public State")
	bool HasPreviousPublicClaim() const { return bHasPreviousPublicClaim; }

	UFUNCTION(BlueprintPure, Category = "Dubito|Public State")
	int32 GetPreviousClaimantId() const { return PreviousClaimantId; }

	UFUNCTION(BlueprintPure, Category = "Dubito|Public State")
	FDubitoAnnouncement GetPreviousPublicAnnouncement() const { return PreviousPublicAnnouncement; }

	UFUNCTION(BlueprintPure, Category = "Dubito|Public State")
	int32 GetClaimedPileCount() const { return ClaimedPileCount; }

	UFUNCTION(BlueprintPure, Category = "Dubito|Public State")
	bool HasPendingWin() const { return bHasPendingWin; }

	UFUNCTION(BlueprintPure, Category = "Dubito|Public State")
	double GetTurnDeadlineServerTimeSeconds() const { return TurnDeadlineServerTimeSeconds; }

protected:
	UFUNCTION()
	void OnRep_PublicMatchState();

	UFUNCTION(BlueprintImplementableEvent, Category = "Dubito|Public State")
	void OnPublicMatchStateUpdated();

private:
	UPROPERTY(ReplicatedUsing = OnRep_PublicMatchState, BlueprintReadOnly, Category = "Dubito|Public State", meta = (AllowPrivateAccess = "true"))
	EDubitoPhase PublicPhase = EDubitoPhase::Lobby;

	UPROPERTY(ReplicatedUsing = OnRep_PublicMatchState, BlueprintReadOnly, Category = "Dubito|Public State", meta = (AllowPrivateAccess = "true"))
	int32 ActivePlayerId = DubitoConstants::NoPlayerId;

	UPROPERTY(ReplicatedUsing = OnRep_PublicMatchState, BlueprintReadOnly, Category = "Dubito|Public State", meta = (AllowPrivateAccess = "true"))
	int32 RoundValue = DubitoConstants::NoRoundValue;

	UPROPERTY(ReplicatedUsing = OnRep_PublicMatchState, BlueprintReadOnly, Category = "Dubito|Public State", meta = (AllowPrivateAccess = "true"))
	bool bHasPreviousPublicClaim = false;

	UPROPERTY(ReplicatedUsing = OnRep_PublicMatchState, BlueprintReadOnly, Category = "Dubito|Public State", meta = (AllowPrivateAccess = "true"))
	int32 PreviousClaimantId = DubitoConstants::NoPlayerId;

	UPROPERTY(ReplicatedUsing = OnRep_PublicMatchState, BlueprintReadOnly, Category = "Dubito|Public State", meta = (AllowPrivateAccess = "true"))
	FDubitoAnnouncement PreviousPublicAnnouncement;

	UPROPERTY(ReplicatedUsing = OnRep_PublicMatchState, BlueprintReadOnly, Category = "Dubito|Public State", meta = (AllowPrivateAccess = "true"))
	int32 ClaimedPileCount = 0;

	UPROPERTY(ReplicatedUsing = OnRep_PublicMatchState, BlueprintReadOnly, Category = "Dubito|Public State", meta = (AllowPrivateAccess = "true"))
	bool bHasPendingWin = false;

	UPROPERTY(ReplicatedUsing = OnRep_PublicMatchState, BlueprintReadOnly, Category = "Dubito|Public State", meta = (AllowPrivateAccess = "true"))
	double TurnDeadlineServerTimeSeconds = 0.0;
};
