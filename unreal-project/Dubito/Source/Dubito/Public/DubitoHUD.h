#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "DubitoHUD.generated.h"

class UDubitoTableHudWidget;

/**
 * Phase 5.0 HUD owner.
 *
 * The HUD exists per client. On the Table map it creates and shows the table HUD widget
 * for the owning local player. Other maps (menu, waiting room, post game) get their flow
 * UI in later Phase 5 sub-phases, so the table HUD is intentionally gated to the table.
 */
UCLASS()
class DUBITO_API ADubitoHUD : public AHUD
{
	GENERATED_BODY()

public:
	ADubitoHUD();

	// Class of table HUD widget to create. Defaults to the C++ table HUD; a Blueprint
	// subclass can be set to skin it without changing binding logic.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dubito|HUD")
	TSubclassOf<UDubitoTableHudWidget> TableHudWidgetClass;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	bool IsTableMap() const;

	UPROPERTY(Transient)
	TObjectPtr<UDubitoTableHudWidget> TableHudWidget = nullptr;
};
