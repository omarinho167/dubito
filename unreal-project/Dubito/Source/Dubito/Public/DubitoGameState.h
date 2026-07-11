#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "DubitoAnnouncement.h"
#include "DubitoMatchState.h"
#include "DubitoReveal.h"
#include "DubitoGameState.generated.h"

UENUM(BlueprintType)
enum class EDubitoGameOverReason : uint8
{
	Unknown,
	PendingWinDoubtFailed,
	PendingWinDeclined,
	PendingWinTimeout,
	PendingWinResponderDisconnected,
	LastPlayerStanding,
	NoPlayersRemaining
};

USTRUCT(BlueprintType)
struct DUBITO_API FDubitoGameOverInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Dubito|Public Events")
	int32 WinnerId = DubitoConstants::NoPlayerId;

	UPROPERTY(BlueprintReadOnly, Category = "Dubito|Public Events")
	EDubitoGameOverReason Reason = EDubitoGameOverReason::Unknown;
};

// Native broadcast fired whenever the replicated public match snapshot changes, on both
// the authoritative sync and the client OnRep. Lets C++ HUD widgets refresh event-driven
// from replicated state without a Blueprint hop (Documentation/ARCHITECTURE.md UI rule).
DECLARE_MULTICAST_DELEGATE(FDubitoPublicMatchStateChanged);

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
	void PublishPublicReveal(const FDubitoRevealInfo& Reveal);
	void PublishGameOver(const FDubitoGameOverInfo& GameOver);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastPublicReveal(const FDubitoRevealInfo& Reveal);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastGameOver(const FDubitoGameOverInfo& GameOver);

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
	bool IsPreviousClaimDoubtable() const { return bPreviousClaimDoubtable; }

	UFUNCTION(BlueprintPure, Category = "Dubito|Public State")
	FDubitoAnnouncement GetPreviousPublicAnnouncement() const { return PreviousPublicAnnouncement; }

	UFUNCTION(BlueprintPure, Category = "Dubito|Public State")
	int32 GetClaimedPileCount() const { return ClaimedPileCount; }

	UFUNCTION(BlueprintPure, Category = "Dubito|Public State")
	bool HasPendingWin() const { return bHasPendingWin; }

	UFUNCTION(BlueprintPure, Category = "Dubito|Public State")
	double GetTurnDeadlineServerTimeSeconds() const { return TurnDeadlineServerTimeSeconds; }

	UFUNCTION(BlueprintPure, Category = "Dubito|Public Events")
	FDubitoRevealInfo GetLastPublicReveal() const { return LastPublicReveal; }

	UFUNCTION(BlueprintPure, Category = "Dubito|Public Events")
	int32 GetPublicRevealEventCount() const { return PublicRevealEventCount; }

	UFUNCTION(BlueprintPure, Category = "Dubito|Public Events")
	FDubitoGameOverInfo GetLastGameOver() const { return LastGameOver; }

	UFUNCTION(BlueprintPure, Category = "Dubito|Public Events")
	int32 GetGameOverEventCount() const { return GameOverEventCount; }

	// Subscribe to be notified (server sync + client OnRep) when public match state changes.
	FDubitoPublicMatchStateChanged OnPublicMatchStateChanged;

protected:
	UFUNCTION()
	void OnRep_PublicMatchState();

	UFUNCTION(BlueprintImplementableEvent, Category = "Dubito|Public State")
	void OnPublicMatchStateUpdated();

	UFUNCTION(BlueprintImplementableEvent, Category = "Dubito|Public Events")
	void OnPublicReveal(const FDubitoRevealInfo& Reveal);

	UFUNCTION(BlueprintImplementableEvent, Category = "Dubito|Public Events")
	void OnGameOver(const FDubitoGameOverInfo& GameOver);

private:
	void NotifyPublicMatchStateUpdated();

	FDubitoRevealInfo LastPublicReveal;
	int32 PublicRevealEventCount = 0;
	FDubitoGameOverInfo LastGameOver;
	int32 GameOverEventCount = 0;

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

	// Whether the previous public claim can still be doubted by the active player. A stale
	// claim left by a disconnected player stays visible but is not doubtable.
	UPROPERTY(ReplicatedUsing = OnRep_PublicMatchState, BlueprintReadOnly, Category = "Dubito|Public State", meta = (AllowPrivateAccess = "true"))
	bool bPreviousClaimDoubtable = false;

	UPROPERTY(ReplicatedUsing = OnRep_PublicMatchState, BlueprintReadOnly, Category = "Dubito|Public State", meta = (AllowPrivateAccess = "true"))
	FDubitoAnnouncement PreviousPublicAnnouncement;

	UPROPERTY(ReplicatedUsing = OnRep_PublicMatchState, BlueprintReadOnly, Category = "Dubito|Public State", meta = (AllowPrivateAccess = "true"))
	int32 ClaimedPileCount = 0;

	UPROPERTY(ReplicatedUsing = OnRep_PublicMatchState, BlueprintReadOnly, Category = "Dubito|Public State", meta = (AllowPrivateAccess = "true"))
	bool bHasPendingWin = false;

	UPROPERTY(ReplicatedUsing = OnRep_PublicMatchState, BlueprintReadOnly, Category = "Dubito|Public State", meta = (AllowPrivateAccess = "true"))
	double TurnDeadlineServerTimeSeconds = 0.0;
};
