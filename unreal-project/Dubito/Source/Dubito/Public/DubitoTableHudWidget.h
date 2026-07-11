#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DubitoTableHudView.h"
#include "DubitoTableHudWidget.generated.h"

class ADubitoGameState;
class UVerticalBox;
class UTextBlock;

/**
 * Phase 5.0 table HUD.
 *
 * Renders confirmed replicated PUBLIC state so a player can always answer: whose turn is
 * it, what claim can be judged, what actions are legal, and what public stake is shown.
 * It derives everything through the pure BuildDubitoTableHudSnapshot view-model and holds
 * no rule authority. Refresh is event-driven off the GameState public-state delegate, with
 * a light repeating tick only to advance the turn countdown display.
 */
UCLASS(Blueprintable)
class DUBITO_API UDubitoTableHudWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UDubitoTableHudWidget(const FObjectInitializer& ObjectInitializer);

	// The latest derived snapshot, for Blueprint subclasses that want to skin the HUD.
	UFUNCTION(BlueprintPure, Category = "Dubito|Table HUD")
	const FDubitoTableHudSnapshot& GetTableHudSnapshot() const { return CachedSnapshot; }

	// Rebuilds the snapshot from current replicated state and updates the display.
	UFUNCTION(BlueprintCallable, Category = "Dubito|Table HUD")
	void RefreshFromReplicatedState();

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual TSharedRef<SWidget> RebuildWidget() override;

	// Fired after each refresh so a Blueprint subclass can drive richer presentation.
	UFUNCTION(BlueprintImplementableEvent, Category = "Dubito|Table HUD")
	void OnTableHudSnapshotUpdated(const FDubitoTableHudSnapshot& Snapshot);

private:
	void EnsureHudTree();
	void BindToGameState();
	void UnbindFromGameState();
	void HandlePublicMatchStateChanged();
	void HandleCountdownTick();

	ADubitoGameState* ResolveGameState() const;
	int32 ResolveLocalPlayerId() const;
	int32 ResolveLocalHandCount() const;

	void ApplySnapshotToWidgets();

	FDubitoTableHudSnapshot CachedSnapshot;

	TWeakObjectPtr<ADubitoGameState> BoundGameState;
	FDelegateHandle PublicStateChangedHandle;
	FTimerHandle CountdownTimerHandle;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> PhaseText = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> TurnText = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> TimerText = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> RoundText = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> ClaimText = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> StakeText = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> ActionsText = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UVerticalBox> SeatsBox = nullptr;
};
