#include "DubitoHUD.h"

#include "Blueprint/UserWidget.h"
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
}

void ADubitoHUD::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* OwningController = GetOwningPlayerController();
	if (!OwningController || !OwningController->IsLocalController())
	{
		return;
	}

	if (!IsTableMap() || !TableHudWidgetClass)
	{
		return;
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
