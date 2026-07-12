#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DubitoRevealView.h"
#include "DubitoGameState.h"
#include "DubitoRevealWidget.generated.h"

class UTextBlock;
class UVerticalBox;

/**
 * Phase 5.2 Doubt reveal panel.
 *
 * The only cinematic beat in V1 (Documentation/DESIGN.md motion rules). It listens for the
 * self-contained public reveal event on ADubitoGameState and presents the resolved Doubt
 * readably through the pure BuildDubitoRevealView view-model: the verdict, the exact lie
 * (count, value, both, or an honest play), the claim vs the revealed cards, who takes the pile,
 * and any confirmed win. It holds no rule authority and only renders the scrubbed public
 * payload, so it cannot leak hidden state. The panel is hidden until a reveal arrives and
 * auto-hides after a short beat so the following turn stays readable. The terminal game-over
 * screen is owned separately by UDubitoPostGameWidget.
 */
UCLASS(Blueprintable)
class DUBITO_API UDubitoRevealWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UDubitoRevealWidget(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintPure, Category = "Dubito|Reveal")
	const FDubitoRevealView& GetRevealView() const { return CachedView; }

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual TSharedRef<SWidget> RebuildWidget() override;

	// Fired after a reveal is presented so a Blueprint subclass can drive richer presentation.
	UFUNCTION(BlueprintImplementableEvent, Category = "Dubito|Reveal")
	void OnRevealPresented(const FDubitoRevealView& View);

private:
	void EnsureRevealTree();
	void BindToGameState();
	void UnbindFromGameState();

	void HandlePublicReveal(const FDubitoRevealInfo& Reveal);
	void HidePanel();

	ADubitoGameState* ResolveGameState() const;
	int32 ResolveLocalPlayerId() const;

	void ApplyViewToWidgets();

	static constexpr float RevealVisibleSeconds = 6.0f;

	FDubitoRevealView CachedView;

	TWeakObjectPtr<ADubitoGameState> BoundGameState;
	FDelegateHandle RevealHandle;
	FTimerHandle HideTimerHandle;

	UPROPERTY(Transient)
	TObjectPtr<UVerticalBox> PanelStack = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> VerdictText = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> ClaimText = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> ActualText = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> OutcomeText = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> WinText = nullptr;
};
