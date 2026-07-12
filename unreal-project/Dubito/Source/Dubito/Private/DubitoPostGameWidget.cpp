#include "DubitoPostGameWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "DubitoGameState.h"
#include "DubitoPlayerController.h"
#include "DubitoPlayerState.h"
#include "Engine/World.h"
#include "GameFramework/PlayerState.h"

namespace
{
	const FName PostGameRootName(TEXT("PostGameRoot"));

	FString ReasonToString(EDubitoGameOverReason Reason)
	{
		switch (Reason)
		{
		case EDubitoGameOverReason::PendingWinDoubtFailed: return TEXT("the final doubt was wrong");
		case EDubitoGameOverReason::PendingWinDeclined: return TEXT("the next player did not doubt the last card");
		case EDubitoGameOverReason::PendingWinTimeout: return TEXT("the final doubt window timed out");
		case EDubitoGameOverReason::PendingWinResponderDisconnected: return TEXT("the responder disconnected");
		case EDubitoGameOverReason::LastPlayerStanding: return TEXT("last player standing");
		case EDubitoGameOverReason::NoPlayersRemaining: return TEXT("no players remaining");
		default: return TEXT("the match ended");
		}
	}
}

UDubitoPostGameWidget::UDubitoPostGameWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetVisibility(ESlateVisibility::Collapsed);
}

TSharedRef<SWidget> UDubitoPostGameWidget::RebuildWidget()
{
	EnsurePostGameTree();
	return Super::RebuildWidget();
}

void UDubitoPostGameWidget::EnsurePostGameTree()
{
	if (!WidgetTree || (WidgetTree->RootWidget && WidgetTree->RootWidget->GetFName() == PostGameRootName))
	{
		return;
	}

	UBorder* Root = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), PostGameRootName);
	Root->SetBrushColor(FLinearColor(0.02f, 0.03f, 0.05f, 0.96f));
	Root->SetPadding(FMargin(36.0f, 30.0f));
	Root->SetHorizontalAlignment(HAlign_Center);
	Root->SetVerticalAlignment(VAlign_Center);

	PanelStack = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
	Root->SetContent(PanelStack);

	auto MakeLine = [this](const FLinearColor Color) -> UTextBlock*
	{
		UTextBlock* Text = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		Text->SetColorAndOpacity(FSlateColor(Color));
		Text->SetJustification(ETextJustify::Center);
		Text->SetAutoWrapText(true);
		UVerticalBoxSlot* Slot = PanelStack->AddChildToVerticalBox(Text);
		Slot->SetHorizontalAlignment(HAlign_Center);
		Slot->SetPadding(FMargin(0.0f, 6.0f));
		return Text;
	};

	HeadlineText = MakeLine(FLinearColor(0.98f, 0.86f, 0.42f, 1.0f));
	WinnerText = MakeLine(FLinearColor(0.92f, 0.94f, 0.96f, 1.0f));
	ReasonText = MakeLine(FLinearColor(0.74f, 0.80f, 0.84f, 1.0f));
	HintText = MakeLine(FLinearColor(0.56f, 0.62f, 0.68f, 1.0f));

	WidgetTree->RootWidget = Root;
}

void UDubitoPostGameWidget::NativeConstruct()
{
	Super::NativeConstruct();
	BindToGameState();
}

void UDubitoPostGameWidget::NativeDestruct()
{
	UnbindFromGameState();
	Super::NativeDestruct();
}

ADubitoGameState* UDubitoPostGameWidget::ResolveGameState() const
{
	const UWorld* World = GetWorld();
	return World ? World->GetGameState<ADubitoGameState>() : nullptr;
}

int32 UDubitoPostGameWidget::ResolveLocalPlayerId() const
{
	if (const ADubitoPlayerController* Controller = Cast<ADubitoPlayerController>(GetOwningPlayer()))
	{
		return Controller->ResolveAuthorityPlayerId();
	}
	if (const APlayerController* Owner = GetOwningPlayer())
	{
		if (const ADubitoPlayerState* DubitoPlayerState = Owner->GetPlayerState<ADubitoPlayerState>())
		{
			return DubitoPlayerState->GetDubitoPlayerId();
		}
	}
	return DubitoConstants::NoPlayerId;
}

void UDubitoPostGameWidget::BindToGameState()
{
	ADubitoGameState* GameState = ResolveGameState();
	if (!GameState || BoundGameState.Get() == GameState)
	{
		return;
	}

	UnbindFromGameState();
	GameOverHandle = GameState->OnPublicGameOverEvent.AddUObject(this, &UDubitoPostGameWidget::HandleGameOver);
	BoundGameState = GameState;
}

void UDubitoPostGameWidget::UnbindFromGameState()
{
	if (ADubitoGameState* GameState = BoundGameState.Get())
	{
		GameState->OnPublicGameOverEvent.Remove(GameOverHandle);
	}
	GameOverHandle.Reset();
	BoundGameState = nullptr;
}

void UDubitoPostGameWidget::HandleGameOver(const FDubitoGameOverInfo& GameOver)
{
	CachedView = BuildDubitoPostGameView(GameOver, ResolveLocalPlayerId());
	if (!CachedView.bValid)
	{
		return;
	}

	ApplyViewToWidgets();
	SetVisibility(ESlateVisibility::HitTestInvisible);
	OnPostGamePresented(CachedView);
}

void UDubitoPostGameWidget::ApplyViewToWidgets()
{
	if (HeadlineText)
	{
		FString Headline;
		switch (CachedView.Result)
		{
		case EDubitoPostGameResult::LocalWin: Headline = TEXT("YOU WIN"); break;
		case EDubitoPostGameResult::LocalLoss: Headline = TEXT("DEFEAT"); break;
		case EDubitoPostGameResult::NoWinner: Headline = TEXT("MATCH OVER"); break;
		default: Headline = TEXT("MATCH OVER"); break;
		}
		HeadlineText->SetText(FText::FromString(Headline));
	}

	if (WinnerText)
	{
		const FString WinnerLine = CachedView.bHasWinner
			? FString::Printf(TEXT("Winner: Player %d"), CachedView.WinnerId)
			: TEXT("No winner - all players left the match");
		WinnerText->SetText(FText::FromString(WinnerLine));
	}

	if (ReasonText)
	{
		ReasonText->SetText(FText::FromString(FString::Printf(TEXT("Ended by %s"), *ReasonToString(CachedView.Reason))));
	}

	if (HintText)
	{
		HintText->SetText(FText::FromString(TEXT("Return to the menu (session flow arrives in Phase 5.6)")));
	}
}
