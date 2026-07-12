#include "DubitoRevealWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "DubitoPlayerController.h"
#include "DubitoPlayerState.h"
#include "Engine/World.h"
#include "GameFramework/PlayerState.h"
#include "TimerManager.h"

namespace
{
	const FName RevealRootName(TEXT("RevealRoot"));

	FString RankToString(int32 Rank)
	{
		switch (Rank)
		{
		case 1: return TEXT("A");
		case 11: return TEXT("J");
		case 12: return TEXT("Q");
		case 13: return TEXT("K");
		default: return FString::FromInt(Rank);
		}
	}

	FString SuitToString(EDubitoSuit Suit)
	{
		switch (Suit)
		{
		case EDubitoSuit::Clubs: return TEXT("C");
		case EDubitoSuit::Diamonds: return TEXT("D");
		case EDubitoSuit::Hearts: return TEXT("H");
		case EDubitoSuit::Spades: return TEXT("S");
		default: return TEXT("?");
		}
	}

	FString CardsToString(const TArray<FDubitoCard>& Cards)
	{
		if (Cards.Num() == 0)
		{
			return TEXT("(none)");
		}

		TArray<FString> Parts;
		Parts.Reserve(Cards.Num());
		for (const FDubitoCard& Card : Cards)
		{
			Parts.Add(RankToString(Card.Rank) + SuitToString(Card.Suit));
		}
		return FString::Join(Parts, TEXT("  "));
	}

	FString LieKindToString(EDubitoRevealLieKind Kind)
	{
		switch (Kind)
		{
		case EDubitoRevealLieKind::Honest: return TEXT("Honest play - count and value both true");
		case EDubitoRevealLieKind::CountLie: return TEXT("Count lie - wrong number of cards");
		case EDubitoRevealLieKind::ValueLie: return TEXT("Value lie - an off-value card");
		case EDubitoRevealLieKind::CountAndValueLie: return TEXT("Count and value lie");
		default: return TEXT("");
		}
	}

}

UDubitoRevealWidget::UDubitoRevealWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetVisibility(ESlateVisibility::Collapsed);
}

TSharedRef<SWidget> UDubitoRevealWidget::RebuildWidget()
{
	EnsureRevealTree();
	return Super::RebuildWidget();
}

void UDubitoRevealWidget::EnsureRevealTree()
{
	if (!WidgetTree || (WidgetTree->RootWidget && WidgetTree->RootWidget->GetFName() == RevealRootName))
	{
		return;
	}

	UBorder* Root = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), RevealRootName);
	Root->SetBrushColor(FLinearColor(0.06f, 0.02f, 0.02f, 0.92f));
	Root->SetPadding(FMargin(28.0f, 22.0f));
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
		Slot->SetPadding(FMargin(0.0f, 4.0f));
		return Text;
	};

	VerdictText = MakeLine(FLinearColor(0.98f, 0.80f, 0.35f, 1.0f));
	ClaimText = MakeLine(FLinearColor(0.86f, 0.89f, 0.88f, 1.0f));
	ActualText = MakeLine(FLinearColor(0.96f, 0.92f, 0.84f, 1.0f));
	OutcomeText = MakeLine(FLinearColor(0.72f, 0.84f, 0.80f, 1.0f));
	WinText = MakeLine(FLinearColor(0.60f, 0.92f, 0.66f, 1.0f));

	WidgetTree->RootWidget = Root;
}

void UDubitoRevealWidget::NativeConstruct()
{
	Super::NativeConstruct();
	BindToGameState();
}

void UDubitoRevealWidget::NativeDestruct()
{
	UnbindFromGameState();
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(HideTimerHandle);
	}
	Super::NativeDestruct();
}

ADubitoGameState* UDubitoRevealWidget::ResolveGameState() const
{
	const UWorld* World = GetWorld();
	return World ? World->GetGameState<ADubitoGameState>() : nullptr;
}

int32 UDubitoRevealWidget::ResolveLocalPlayerId() const
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

void UDubitoRevealWidget::BindToGameState()
{
	ADubitoGameState* GameState = ResolveGameState();
	if (!GameState || BoundGameState.Get() == GameState)
	{
		return;
	}

	UnbindFromGameState();
	RevealHandle = GameState->OnPublicRevealEvent.AddUObject(this, &UDubitoRevealWidget::HandlePublicReveal);
	BoundGameState = GameState;
}

void UDubitoRevealWidget::UnbindFromGameState()
{
	if (ADubitoGameState* GameState = BoundGameState.Get())
	{
		GameState->OnPublicRevealEvent.Remove(RevealHandle);
	}
	RevealHandle.Reset();
	BoundGameState = nullptr;
}

void UDubitoRevealWidget::HandlePublicReveal(const FDubitoRevealInfo& Reveal)
{
	CachedView = BuildDubitoRevealView(Reveal, ResolveLocalPlayerId());
	if (!CachedView.bValid)
	{
		return;
	}

	ApplyViewToWidgets();
	SetVisibility(ESlateVisibility::HitTestInvisible);
	OnRevealPresented(CachedView);

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(HideTimerHandle, this, &UDubitoRevealWidget::HidePanel, RevealVisibleSeconds, false);
	}
}

void UDubitoRevealWidget::HidePanel()
{
	SetVisibility(ESlateVisibility::Collapsed);
}

void UDubitoRevealWidget::ApplyViewToWidgets()
{
	if (VerdictText)
	{
		FString Verdict = CachedView.Outcome == EDubitoRevealOutcome::CorrectDoubt
			? FString::Printf(TEXT("CORRECT DOUBT - Player %d caught Player %d"), CachedView.DoubterId, CachedView.ClaimantId)
			: FString::Printf(TEXT("WRONG DOUBT - Player %d was honest"), CachedView.ClaimantId);
		VerdictText->SetText(FText::FromString(Verdict));
	}

	if (ClaimText)
	{
		ClaimText->SetText(FText::FromString(FString::Printf(TEXT("Claimed: %d x value %s"),
			CachedView.Claim.ClaimedCount, *RankToString(CachedView.Claim.ClaimedValue))));
	}

	if (ActualText)
	{
		ActualText->SetText(FText::FromString(FString::Printf(TEXT("Actually played (%d): %s"),
			CachedView.ActualCount, *CardsToString(CachedView.RevealedCards))));
	}

	if (OutcomeText)
	{
		FString Line = LieKindToString(CachedView.LieKind);
		Line += FString::Printf(TEXT("   |   Player %d takes the pile (+%d claimed)"),
			CachedView.LoserId, CachedView.ClaimedStakeTransferred);
		OutcomeText->SetText(FText::FromString(Line));
	}

	if (WinText)
	{
		if (CachedView.bWinConfirmed)
		{
			WinText->SetText(FText::FromString(FString::Printf(TEXT("WIN CONFIRMED - Player %d%s"),
				CachedView.WinnerId, CachedView.bLocalIsWinner ? TEXT("  - YOU WIN") : TEXT(""))));
		}
		else
		{
			WinText->SetText(FText::GetEmpty());
		}
	}
}
