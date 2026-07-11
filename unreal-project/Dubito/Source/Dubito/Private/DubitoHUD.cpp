#include "DubitoHUD.h"

#include "Blueprint/UserWidget.h"
#include "DubitoTableHudWidget.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"

ADubitoHUD::ADubitoHUD()
{
	TableHudWidgetClass = UDubitoTableHudWidget::StaticClass();
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
}

void ADubitoHUD::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (TableHudWidget)
	{
		TableHudWidget->RemoveFromParent();
		TableHudWidget = nullptr;
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
