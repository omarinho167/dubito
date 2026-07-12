#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "DubitoHUD.generated.h"

class UDubitoTableHudWidget;
class UDubitoPlayActionWidget;
class UDubitoRevealWidget;
class UDubitoPostGameWidget;
class UDubitoMainMenuWidget;
class UDubitoLobbyWidget;

/**
 * Phase 5.0 HUD owner.
 *
 * The HUD exists per client. On the Table map it creates and shows the table HUD widget
 * for the owning local player. Other maps (menu, waiting room, post game) get their flow
 * UI in later Phase 5 sub-phases, so the table HUD is intentionally gated to the table.
 *
 * Phase 5.1 adds the interactive Play action bar alongside the read-only table HUD, also
 * gated to the Table map and the owning local player.
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

	// Class of the interactive Play action bar. Defaults to the C++ widget; a Blueprint
	// subclass can be set to skin it without changing intent/submit logic.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dubito|HUD")
	TSubclassOf<UDubitoPlayActionWidget> PlayActionWidgetClass;

	// Class of the Doubt reveal panel. Defaults to the C++ widget; a Blueprint subclass can
	// skin the reveal beat without changing which public event drives it.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dubito|HUD")
	TSubclassOf<UDubitoRevealWidget> RevealWidgetClass;

	// Class of the terminal post-game panel. Defaults to the C++ widget; a Blueprint subclass
	// can skin the result screen without changing which public event drives it.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dubito|HUD")
	TSubclassOf<UDubitoPostGameWidget> PostGameWidgetClass;

	// Class of the greybox Main Menu shown on the front map (Phase 5.6 local session flow).
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dubito|HUD")
	TSubclassOf<UDubitoMainMenuWidget> MainMenuWidgetClass;

	// Class of the pre-match lobby shown on the Table map before the deal (Phase 5.6).
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dubito|HUD")
	TSubclassOf<UDubitoLobbyWidget> LobbyWidgetClass;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	bool IsTableMap() const;
	bool IsMainMenuMap() const;

	UPROPERTY(Transient)
	TObjectPtr<UDubitoTableHudWidget> TableHudWidget = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UDubitoPlayActionWidget> PlayActionWidget = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UDubitoRevealWidget> RevealWidget = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UDubitoPostGameWidget> PostGameWidget = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UDubitoMainMenuWidget> MainMenuWidget = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UDubitoLobbyWidget> LobbyWidget = nullptr;
};
