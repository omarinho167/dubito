#pragma once

#include "CoreMinimal.h"
#include "CommonButtonBase.h"
#include "DubitoShellButton.generated.h"

class SWidget;

/**
 * CommonUI-compatible placeholder button for Phase 3 input validation.
 */
UCLASS(Blueprintable)
class DUBITO_API UDubitoShellButton : public UCommonButtonBase
{
	GENERATED_BODY()

public:
	UDubitoShellButton(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dubito|Shell")
	FText ButtonText;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dubito|Shell")
	FLinearColor ButtonTint;

	UFUNCTION(BlueprintCallable, Category = "Dubito|Shell")
	void SetButtonText(const FText& InText);

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativePreConstruct() override;

private:
	void EnsureButtonTree();
	void RefreshButtonText() const;
};
