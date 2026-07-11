#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "UObject/CoreNetTypes.h"
#include "DubitoAnnouncement.h"
#include "DubitoConstants.h"
#include "DubitoHand.h"
#include "DubitoPlayerController.generated.h"

class ADubitoGameMode;

UENUM(BlueprintType)
enum class EDubitoServerAction : uint8
{
	None,
	Ready,
	StartMatch,
	Play,
	Doubt,
	Discard
};

UENUM(BlueprintType)
enum class EDubitoActionRejectionReason : uint8
{
	None,
	MissingAuthority,
	MissingPlayer,
	StartInvalidPlayers,
	StartNotAllReady,
	PlayWrongPhase,
	PlayNotYourTurn,
	PlayBadActualCount,
	PlayBadAnnouncement,
	PlayValueLocked,
	PlayDontOwnCards,
	DoubtUnavailable,
	DiscardUnavailable
};

/**
 * Owning-client replicated private state for Phase 4.3.
 *
 * PlayerController exists on the server and the owning client. The exact hand is
 * also marked COND_OwnerOnly so non-owners never receive it through replication.
 */
UCLASS()
class DUBITO_API ADubitoPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ADubitoPlayerController();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	static constexpr ELifetimeCondition PrivateHandReplicationCondition = COND_OwnerOnly;
	static constexpr ELifetimeCondition GetPrivateHandReplicationCondition() { return PrivateHandReplicationCondition; }

	void SetAuthorityPlayerId(int32 InPlayerId);
	int32 GetAuthorityPlayerId() const { return AuthorityPlayerId; }
	int32 ResolveAuthorityPlayerId() const;

	UFUNCTION(BlueprintCallable, Category = "Dubito|Actions")
	void RequestReady(bool bReady);

	UFUNCTION(BlueprintCallable, Category = "Dubito|Actions")
	void RequestStartMatch(int32 ShuffleSeed);

	UFUNCTION(BlueprintCallable, Category = "Dubito|Actions")
	void RequestPlayCards(const TArray<FDubitoCard>& ActualCards, const FDubitoAnnouncement& Announcement);

	UFUNCTION(BlueprintCallable, Category = "Dubito|Actions")
	void RequestDoubt();

	UFUNCTION(BlueprintCallable, Category = "Dubito|Actions")
	void RequestDiscard();

	UFUNCTION(Server, Reliable)
	void ServerSetReady(bool bReady);

	UFUNCTION(Server, Reliable)
	void ServerStartMatch(int32 ShuffleSeed);

	UFUNCTION(Server, Reliable)
	void ServerPlayCards(const TArray<FDubitoCard>& ActualCards, const FDubitoAnnouncement& Announcement);

	UFUNCTION(Server, Reliable)
	void ServerDoubt();

	UFUNCTION(Server, Reliable)
	void ServerDiscard();

	UFUNCTION(Client, Reliable)
	void ClientReceiveActionRejected(EDubitoServerAction Action, EDubitoActionRejectionReason Reason, bool bRequestedResync);

	void SetPrivateHand(const FDubitoHand& InPrivateHand);
	void ClearPrivateHand();

	const FDubitoHand& GetPrivateHand() const { return PrivateHand; }

	UFUNCTION(BlueprintPure, Category = "Dubito|Private Hand")
	TArray<FDubitoCard> GetPrivateHandCards() const { return PrivateHand.Cards; }

	UFUNCTION(BlueprintPure, Category = "Dubito|Private Hand")
	int32 GetPrivateHandCount() const { return PrivateHand.Num(); }

	UFUNCTION(BlueprintPure, Category = "Dubito|Actions")
	EDubitoServerAction GetLastRejectedAction() const { return LastRejectedAction; }

	UFUNCTION(BlueprintPure, Category = "Dubito|Actions")
	EDubitoActionRejectionReason GetLastRejectionReason() const { return LastRejectionReason; }

	UFUNCTION(BlueprintPure, Category = "Dubito|Actions")
	int32 GetActionRejectionCount() const { return ActionRejectionCount; }

	UFUNCTION(BlueprintPure, Category = "Dubito|Actions")
	bool DidLastRejectionRequestResync() const { return bLastRejectionRequestedResync; }

protected:
	UFUNCTION()
	void OnRep_PrivateHand();

	UFUNCTION(BlueprintImplementableEvent, Category = "Dubito|Private Hand")
	void OnPrivateHandUpdated();

	UFUNCTION(BlueprintImplementableEvent, Category = "Dubito|Actions")
	void OnServerActionRejected(EDubitoServerAction Action, EDubitoActionRejectionReason Reason);

private:
	ADubitoGameMode* GetAuthorityGameMode() const;
	bool ExecuteReadyAction(bool bReady);
	bool ExecuteStartMatchAction(int32 ShuffleSeed);
	bool ExecutePlayAction(const TArray<FDubitoCard>& ActualCards, const FDubitoAnnouncement& Announcement);
	bool ExecuteDoubtAction();
	bool ExecuteDiscardAction();
	void RejectAction(ADubitoGameMode* GameMode, EDubitoServerAction Action, EDubitoActionRejectionReason Reason);
	void RecordActionRejection(EDubitoServerAction Action, EDubitoActionRejectionReason Reason, bool bRequestedResync);

	int32 AuthorityPlayerId = DubitoConstants::NoPlayerId;
	EDubitoServerAction LastRejectedAction = EDubitoServerAction::None;
	EDubitoActionRejectionReason LastRejectionReason = EDubitoActionRejectionReason::None;
	int32 ActionRejectionCount = 0;
	bool bLastRejectionRequestedResync = false;

	UPROPERTY(ReplicatedUsing = OnRep_PrivateHand, BlueprintReadOnly, Category = "Dubito|Private Hand", meta = (AllowPrivateAccess = "true"))
	FDubitoHand PrivateHand;
};
