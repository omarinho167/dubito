#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DubitoGameOver.h"
#include "DubitoPostGameView.h"
#include "DubitoPostGameWidget.generated.h"

class ADubitoGameState;
class UTextBlock;
class UVerticalBox;

/**
 * Phase 5.5 post-game panel.
 *
 * Listens for the self-contained public game-over event on ADubitoGameState and presents the
 * terminal result through the pure BuildDubitoPostGameView view-model: who won, why the match
 * ended, and whether the local player won, lost, or watched. It is hidden until the match ends
 * and then persists (the return-to-menu flow arrives with the Phase 5.6 local session flow).
 * It holds no rule authority and only renders the scrubbed public payload.
 */
UCLASS(Blueprintable)
class DUBITO_API UDubitoPostGameWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UDubitoPostGameWidget(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintPure, Category = "Dubito|Post Game")
	const FDubitoPostGameView& GetPostGameView() const { return CachedView; }

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual TSharedRef<SWidget> RebuildWidget() override;

	// Fired after the post-game result is presented so a Blueprint subclass can skin it.
	UFUNCTION(BlueprintImplementableEvent, Category = "Dubito|Post Game")
	void OnPostGamePresented(const FDubitoPostGameView& View);

private:
	void EnsurePostGameTree();
	void BindToGameState();
	void UnbindFromGameState();

	void HandleGameOver(const FDubitoGameOverInfo& GameOver);

	ADubitoGameState* ResolveGameState() const;
	int32 ResolveLocalPlayerId() const;

	void ApplyViewToWidgets();

	FDubitoPostGameView CachedView;

	TWeakObjectPtr<ADubitoGameState> BoundGameState;
	FDelegateHandle GameOverHandle;

	UPROPERTY(Transient)
	TObjectPtr<UVerticalBox> PanelStack = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> HeadlineText = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> WinnerText = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> ReasonText = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> HintText = nullptr;
};
