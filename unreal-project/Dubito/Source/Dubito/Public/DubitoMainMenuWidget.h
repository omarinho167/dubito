#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DubitoMainMenuWidget.generated.h"

class UTextBlock;

/**
 * Phase 5.6 greybox Main Menu.
 *
 * A programmatic C++ menu (same approach as the Phase 5.0-5.5 table widgets) that drives the local
 * session flow through UDubitoGameInstance: Host a same-PC listen server, Join the default loopback
 * address, or Quit. It carries no gameplay authority; it only starts transport.
 */
UCLASS()
class DUBITO_API UDubitoMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UDubitoMainMenuWidget(const FObjectInitializer& ObjectInitializer);

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeConstruct() override;

	UFUNCTION()
	void HandleHost();

	UFUNCTION()
	void HandleJoin();

	UFUNCTION()
	void HandleQuit();

private:
	void EnsureMenuTree();
	class UDubitoGameInstance* ResolveGameInstance() const;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> StatusText = nullptr;
};
