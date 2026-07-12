#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DubitoDiscardView.h"
#include "DubitoPlaySelectionView.h"
#include "DubitoPlayActionWidget.generated.h"

class ADubitoGameState;
class ADubitoPlayerController;
class UButton;
class UHorizontalBox;
class UTextBlock;
class UVerticalBox;

/**
 * Phase 5.1 Play interaction widget.
 *
 * The interactive action bar for the local player's own turn: it presents the exact local
 * hand, lets the player select 1..4 actual cards, choose the claimed value (only while
 * opening a round) and the bluffable claimed count, and confirm. Confirming sends the
 * composed payload through ADubitoPlayerController::RequestPlayCards; the server validates,
 * applies the hand/ledger update, and advances the turn, and the change flows back as
 * replicated state.
 *
 * It holds no rule authority: every enable/submittable decision comes from the pure
 * BuildDubitoPlaySelectionView view-model, which mirrors the server predicates. Controls are
 * driven by discrete actions (cursor + toggle, value/count steppers, confirm) so the bar is
 * fully usable with keyboard and gamepad focus, not mouse only. It only ever reads the local
 * exact hand, so it cannot leak an opponent's actual count.
 */
UCLASS(Blueprintable)
class DUBITO_API UDubitoPlayActionWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UDubitoPlayActionWidget(const FObjectInitializer& ObjectInitializer);

	// The latest composed view, for Blueprint subclasses that want to skin the bar.
	UFUNCTION(BlueprintPure, Category = "Dubito|Play")
	const FDubitoPlaySelectionView& GetPlaySelectionView() const { return CachedView; }

	// Rebuilds the composed view from current replicated state + local intent and redraws.
	UFUNCTION(BlueprintCallable, Category = "Dubito|Play")
	void RefreshFromReplicatedState();

	// Intent mutators, exposed so a Blueprint skin or a mouse click can drive them directly.
	UFUNCTION(BlueprintCallable, Category = "Dubito|Play")
	void ToggleCardSelected(int32 HandIndex);

	UFUNCTION(BlueprintCallable, Category = "Dubito|Play")
	void SetChosenValue(int32 Value);

	UFUNCTION(BlueprintCallable, Category = "Dubito|Play")
	void StepClaimedCount(int32 Delta);

	// Sends the composed Play to the server if it is currently submittable. Returns true if a
	// request was sent. Clears local intent on success.
	UFUNCTION(BlueprintCallable, Category = "Dubito|Play")
	bool ConfirmPlay();

	// Discards the in-progress selection/claim without sending anything.
	UFUNCTION(BlueprintCallable, Category = "Dubito|Play")
	void ClearIntent();

	// True when the local active player may Doubt the previous claim right now (mirrors the
	// server Doubt predicate from public state).
	UFUNCTION(BlueprintPure, Category = "Dubito|Doubt")
	bool IsDoubtAvailable() const;

	// Hold-to-confirm Doubt: a press begins the hold, a release before the threshold cancels,
	// and holding past the threshold sends the Doubt. Exposed so a Blueprint skin can drive it.
	UFUNCTION(BlueprintCallable, Category = "Dubito|Doubt")
	void BeginDoubtHold();

	UFUNCTION(BlueprintCallable, Category = "Dubito|Doubt")
	void CancelDoubtHold();

	// Current Discard availability + blocked reason from public state (mirrors CanDiscard).
	UFUNCTION(BlueprintPure, Category = "Dubito|Discard")
	FDubitoDiscardView GetDiscardView() const;

	// Light confirmation: the first request arms Discard, a second within the window confirms
	// and sends it, and the arm lapses on a short timer. Exposed for a Blueprint skin.
	UFUNCTION(BlueprintCallable, Category = "Dubito|Discard")
	void RequestDiscardConfirm();

	UFUNCTION(BlueprintCallable, Category = "Dubito|Discard")
	void CancelDiscardArm();

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual TSharedRef<SWidget> RebuildWidget() override;

	// Fired after each refresh so a Blueprint subclass can drive richer presentation.
	UFUNCTION(BlueprintImplementableEvent, Category = "Dubito|Play")
	void OnPlaySelectionViewUpdated(const FDubitoPlaySelectionView& View);

private:
	// Greybox control handlers (bound to the native buttons).
	UFUNCTION()
	void HandleCursorLeft();
	UFUNCTION()
	void HandleCursorRight();
	UFUNCTION()
	void HandleToggleAtCursor();
	UFUNCTION()
	void HandleValueDown();
	UFUNCTION()
	void HandleValueUp();
	UFUNCTION()
	void HandleCountDown();
	UFUNCTION()
	void HandleCountUp();
	UFUNCTION()
	void HandleConfirm();
	UFUNCTION()
	void HandleClear();
	UFUNCTION()
	void HandleDoubtPressed();
	UFUNCTION()
	void HandleDoubtReleased();
	void CompleteDoubtHold();
	UFUNCTION()
	void HandleDiscard();
	void DisarmDiscard();

	void EnsureActionTree();
	void BindToGameState();
	void UnbindFromGameState();
	void HandlePublicMatchStateChanged();

	ADubitoGameState* ResolveGameState() const;
	ADubitoPlayerController* ResolveDubitoController() const;
	int32 ResolveLocalPlayerId() const;

	FDubitoPlaySelectionInputs GatherInputs() const;
	void StepChosenValue(int32 Delta);
	void ApplyViewToWidgets();

	// One-pending-request guard: block a second send until the server confirms via replicated
	// state (or a short safety window lapses), so spammed input cannot queue duplicate actions.
	void BeginPendingRequest();
	void ClearPendingRequest();

	// Seconds the Doubt button must be held before the Doubt is sent (hold-to-confirm).
	static constexpr float DoubtHoldSeconds = 0.6f;

	// Seconds an armed Discard stays armed before it lapses back to a first press.
	static constexpr float DiscardArmSeconds = 3.0f;

	// Safety window after a send before the pending-request guard clears itself, in case a
	// rejected action produces no replicated state change to clear it.
	static constexpr float PendingRequestSafetySeconds = 2.0f;

	// In-progress local intent (never authoritative).
	TArray<int32> SelectedIndices;
	int32 CursorIndex = 0;
	int32 ChosenValue = DubitoConstants::NoRoundValue;
	int32 ClaimedCount = DubitoConstants::MinCardsPerPlay;
	bool bDoubtHoldActive = false;
	bool bDiscardArmed = false;
	bool bAwaitingServerResponse = false;

	FDubitoPlaySelectionView CachedView;

	TWeakObjectPtr<ADubitoGameState> BoundGameState;
	FDelegateHandle PublicStateChangedHandle;
	FTimerHandle DoubtHoldTimerHandle;
	FTimerHandle DiscardArmTimerHandle;
	FTimerHandle PendingRequestTimerHandle;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> TitleText = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UHorizontalBox> HandBox = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> ClaimText = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> StatusText = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> DoubtText = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> DiscardText = nullptr;
};
