#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "UObject/CoreNetTypes.h"
#include "DubitoHand.h"
#include "DubitoPlayerController.generated.h"

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

	void SetPrivateHand(const FDubitoHand& InPrivateHand);
	void ClearPrivateHand();

	const FDubitoHand& GetPrivateHand() const { return PrivateHand; }

	UFUNCTION(BlueprintPure, Category = "Dubito|Private Hand")
	TArray<FDubitoCard> GetPrivateHandCards() const { return PrivateHand.Cards; }

	UFUNCTION(BlueprintPure, Category = "Dubito|Private Hand")
	int32 GetPrivateHandCount() const { return PrivateHand.Num(); }

protected:
	UFUNCTION()
	void OnRep_PrivateHand();

	UFUNCTION(BlueprintImplementableEvent, Category = "Dubito|Private Hand")
	void OnPrivateHandUpdated();

private:
	UPROPERTY(ReplicatedUsing = OnRep_PrivateHand, BlueprintReadOnly, Category = "Dubito|Private Hand", meta = (AllowPrivateAccess = "true"))
	FDubitoHand PrivateHand;
};
