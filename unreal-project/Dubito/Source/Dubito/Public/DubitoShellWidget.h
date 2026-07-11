#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "DubitoShellWidget.generated.h"

class SWidget;
class UWidget;

/**
 * Phase 3 greybox UI surface parent.
 *
 * This owns only placeholder presentation for shell screens. Gameplay authority
 * and rule decisions remain outside widgets.
 */
UCLASS(Blueprintable)
class DUBITO_API UDubitoShellWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	UDubitoShellWidget(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dubito|Shell")
	FText SurfaceTitle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dubito|Shell")
	FText SurfaceSubtitle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dubito|Shell")
	FText PrimaryPlaceholder;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dubito|Shell|Input")
	FText PrimaryActionLabel;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dubito|Shell|Input")
	FText SecondaryActionLabel;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dubito|Shell|Input")
	FText InputHintText;

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativePreConstruct() override;
	virtual UWidget* NativeGetDesiredFocusTarget() const override;

private:
	void EnsurePlaceholderTree();
	void RefreshPlaceholderText() const;
};
