#include "DubitoHUD.h"

#include "Blueprint/UserWidget.h"
#include "DubitoLobbyWidget.h"
#include "DubitoMainMenuWidget.h"
#include "DubitoPlayActionWidget.h"
#include "DubitoPostGameWidget.h"
#include "DubitoRevealWidget.h"
#include "DubitoTableHudWidget.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"

ADubitoHUD::ADubitoHUD()
{
	TableHudWidgetClass = UDubitoTableHudWidget::StaticClass();
	PlayActionWidgetClass = UDubitoPlayActionWidget::StaticClass();
	RevealWidgetClass = UDubitoRevealWidget::StaticClass();
	PostGameWidgetClass = UDubitoPostGameWidget::StaticClass();
	MainMenuWidgetClass = UDubitoMainMenuWidget::StaticClass();
	LobbyWidgetClass = UDubitoLobbyWidget::StaticClass();
}

void ADubitoHUD::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* OwningController = GetOwningPlayerController();
	if (!OwningController || !OwningController->IsLocalController())
	{
		return;
	}

	// Front map: show the greybox Main Menu (Host/Join/Quit). It owns its own UI input mode.
	if (IsMainMenuMap())
	{
		if (MainMenuWidgetClass)
		{
			MainMenuWidget = CreateWidget<UDubitoMainMenuWidget>(OwningController, MainMenuWidgetClass);
			if (MainMenuWidget)
			{
				MainMenuWidget->AddToPlayerScreen();
			}
		}
		return;
	}

	if (!IsTableMap() || !TableHudWidgetClass)
	{
		return;
	}

	// The table mixes a 3D view with clickable lobby/action buttons: show the cursor and accept
	// both game and UI input so the Phase 5.1-5.6 buttons are usable in a standalone game.
	OwningController->bShowMouseCursor = true;
	{
		FInputModeGameAndUI InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		InputMode.SetHideCursorDuringCapture(false);
		OwningController->SetInputMode(InputMode);
	}

	TableHudWidget = CreateWidget<UDubitoTableHudWidget>(OwningController, TableHudWidgetClass);
	if (TableHudWidget)
	{
		TableHudWidget->AddToPlayerScreen();
	}

	if (PlayActionWidgetClass)
	{
		PlayActionWidget = CreateWidget<UDubitoPlayActionWidget>(OwningController, PlayActionWidgetClass);
		if (PlayActionWidget)
		{
			// Above the read-only HUD so its controls are interactive.
			PlayActionWidget->AddToPlayerScreen(10);
		}
	}

	if (RevealWidgetClass)
	{
		RevealWidget = CreateWidget<UDubitoRevealWidget>(OwningController, RevealWidgetClass);
		if (RevealWidget)
		{
			// Above the action bar: the reveal is the one cinematic beat and takes the screen.
			RevealWidget->AddToPlayerScreen(20);
		}
	}

	if (PostGameWidgetClass)
	{
		PostGameWidget = CreateWidget<UDubitoPostGameWidget>(OwningController, PostGameWidgetClass);
		if (PostGameWidget)
		{
			// Top-most: the terminal result screen sits over everything else.
			PostGameWidget->AddToPlayerScreen(30);
		}
	}

	if (LobbyWidgetClass)
	{
		LobbyWidget = CreateWidget<UDubitoLobbyWidget>(OwningController, LobbyWidgetClass);
		if (LobbyWidget)
		{
			// Above the table HUD/action bar while the match is still in the lobby; it hides itself
			// once the deal begins. Below the post-game screen (z 30), which only shows at game over.
			LobbyWidget->AddToPlayerScreen(25);
		}
	}
}

void ADubitoHUD::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (TableHudWidget)
	{
		TableHudWidget->RemoveFromParent();
		TableHudWidget = nullptr;
	}

	if (PlayActionWidget)
	{
		PlayActionWidget->RemoveFromParent();
		PlayActionWidget = nullptr;
	}

	if (RevealWidget)
	{
		RevealWidget->RemoveFromParent();
		RevealWidget = nullptr;
	}

	if (PostGameWidget)
	{
		PostGameWidget->RemoveFromParent();
		PostGameWidget = nullptr;
	}

	if (MainMenuWidget)
	{
		MainMenuWidget->RemoveFromParent();
		MainMenuWidget = nullptr;
	}

	if (LobbyWidget)
	{
		LobbyWidget->RemoveFromParent();
		LobbyWidget = nullptr;
	}

	Super::EndPlay(EndPlayReason);
}

bool ADubitoHUD::IsTableMap() const
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	// World->GetMapName() carries the PIE prefix (e.g. UEDPIE_0_Table); match by suffix.
	return World->GetMapName().EndsWith(TEXT("Table"));
}

bool ADubitoHUD::IsMainMenuMap() const
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	// Same suffix match as IsTableMap, for the front map that hosts the session menu.
	return World->GetMapName().EndsWith(TEXT("MainMenu"));
}
