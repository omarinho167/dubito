#include "DubitoTableHudWidget.h"

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
#include "TimerManager.h"

namespace
{
	const FName HudRootName(TEXT("TableHudRoot"));
	const FName HudStackName(TEXT("TableHudStack"));
	const FName HudPhaseName(TEXT("TableHudPhaseText"));
	const FName HudTurnName(TEXT("TableHudTurnText"));
	const FName HudTimerName(TEXT("TableHudTimerText"));
	const FName HudRoundName(TEXT("TableHudRoundText"));
	const FName HudClaimName(TEXT("TableHudClaimText"));
	const FName HudStakeName(TEXT("TableHudStakeText"));
	const FName HudActionsName(TEXT("TableHudActionsText"));
	const FName HudSeatsName(TEXT("TableHudSeatsBox"));

	constexpr float CountdownIntervalSeconds = 0.5f;

	FString PhaseToString(EDubitoPhase Phase)
	{
		switch (Phase)
		{
		case EDubitoPhase::Lobby: return TEXT("Lobby");
		case EDubitoPhase::Dealing: return TEXT("Dealing");
		case EDubitoPhase::PlayerTurn: return TEXT("Player Turn");
		case EDubitoPhase::DoubtReveal: return TEXT("Doubt Reveal");
		case EDubitoPhase::GameOver: return TEXT("Game Over");
		default: return TEXT("Unknown");
		}
	}

	FString AvailabilityTag(bool bAvailable)
	{
		return bAvailable ? TEXT("available") : TEXT("--");
	}

	UTextBlock* BuildHudTextBlock(UWidgetTree& Tree, const FName Name, const FLinearColor Color)
	{
		UTextBlock* TextBlock = Tree.ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), Name);
		TextBlock->SetColorAndOpacity(FSlateColor(Color));
		TextBlock->SetAutoWrapText(true);
		return TextBlock;
	}

	void AddStackChild(UVerticalBox& Stack, UWidget& Child, const FMargin Padding)
	{
		UVerticalBoxSlot* Slot = Stack.AddChildToVerticalBox(&Child);
		Slot->SetHorizontalAlignment(HAlign_Fill);
		Slot->SetPadding(Padding);
	}
}

UDubitoTableHudWidget::UDubitoTableHudWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetVisibility(ESlateVisibility::HitTestInvisible);
}

TSharedRef<SWidget> UDubitoTableHudWidget::RebuildWidget()
{
	EnsureHudTree();
	return Super::RebuildWidget();
}

void UDubitoTableHudWidget::EnsureHudTree()
{
	if (!WidgetTree || (WidgetTree->RootWidget && WidgetTree->RootWidget->GetFName() == HudRootName))
	{
		return;
	}

	UBorder* Root = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), HudRootName);
	Root->SetBrushColor(FLinearColor(0.02f, 0.024f, 0.03f, 0.82f));
	Root->SetPadding(FMargin(20.0f, 16.0f));
	Root->SetHorizontalAlignment(HAlign_Left);
	Root->SetVerticalAlignment(VAlign_Top);

	UVerticalBox* Stack = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), HudStackName);
	Root->SetContent(Stack);

	PhaseText = BuildHudTextBlock(*WidgetTree, HudPhaseName, FLinearColor(0.72f, 0.77f, 0.80f, 1.0f));
	TurnText = BuildHudTextBlock(*WidgetTree, HudTurnName, FLinearColor(0.96f, 0.92f, 0.84f, 1.0f));
	TimerText = BuildHudTextBlock(*WidgetTree, HudTimerName, FLinearColor(0.90f, 0.80f, 0.55f, 1.0f));
	RoundText = BuildHudTextBlock(*WidgetTree, HudRoundName, FLinearColor(0.86f, 0.89f, 0.88f, 1.0f));
	ClaimText = BuildHudTextBlock(*WidgetTree, HudClaimName, FLinearColor(0.86f, 0.89f, 0.88f, 1.0f));
	StakeText = BuildHudTextBlock(*WidgetTree, HudStakeName, FLinearColor(0.86f, 0.89f, 0.88f, 1.0f));
	ActionsText = BuildHudTextBlock(*WidgetTree, HudActionsName, FLinearColor(0.58f, 0.82f, 0.74f, 1.0f));
	SeatsBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), HudSeatsName);

	AddStackChild(*Stack, *PhaseText, FMargin(0.0f, 0.0f, 0.0f, 4.0f));
	AddStackChild(*Stack, *TurnText, FMargin(0.0f, 0.0f, 0.0f, 4.0f));
	AddStackChild(*Stack, *TimerText, FMargin(0.0f, 0.0f, 0.0f, 12.0f));
	AddStackChild(*Stack, *RoundText, FMargin(0.0f, 0.0f, 0.0f, 4.0f));
	AddStackChild(*Stack, *ClaimText, FMargin(0.0f, 0.0f, 0.0f, 4.0f));
	AddStackChild(*Stack, *StakeText, FMargin(0.0f, 0.0f, 0.0f, 12.0f));
	AddStackChild(*Stack, *ActionsText, FMargin(0.0f, 0.0f, 0.0f, 12.0f));
	AddStackChild(*Stack, *SeatsBox, FMargin(0.0f));

	WidgetTree->RootWidget = Root;
}

void UDubitoTableHudWidget::NativeConstruct()
{
	Super::NativeConstruct();

	BindToGameState();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(CountdownTimerHandle, this, &UDubitoTableHudWidget::HandleCountdownTick, CountdownIntervalSeconds, true);
	}

	RefreshFromReplicatedState();
}

void UDubitoTableHudWidget::NativeDestruct()
{
	UnbindFromGameState();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(CountdownTimerHandle);
	}

	Super::NativeDestruct();
}

ADubitoGameState* UDubitoTableHudWidget::ResolveGameState() const
{
	const UWorld* World = GetWorld();
	return World ? World->GetGameState<ADubitoGameState>() : nullptr;
}

void UDubitoTableHudWidget::BindToGameState()
{
	ADubitoGameState* GameState = ResolveGameState();
	if (!GameState || BoundGameState.Get() == GameState)
	{
		return;
	}

	UnbindFromGameState();
	PublicStateChangedHandle = GameState->OnPublicMatchStateChanged.AddUObject(this, &UDubitoTableHudWidget::HandlePublicMatchStateChanged);
	BoundGameState = GameState;
}

void UDubitoTableHudWidget::UnbindFromGameState()
{
	if (ADubitoGameState* GameState = BoundGameState.Get())
	{
		GameState->OnPublicMatchStateChanged.Remove(PublicStateChangedHandle);
	}
	PublicStateChangedHandle.Reset();
	BoundGameState = nullptr;
}

void UDubitoTableHudWidget::HandlePublicMatchStateChanged()
{
	RefreshFromReplicatedState();
}

void UDubitoTableHudWidget::HandleCountdownTick()
{
	// The GameState may not have existed when the widget first constructed (fast travel);
	// keep trying to bind, then refresh so the countdown display advances.
	BindToGameState();
	RefreshFromReplicatedState();
}

int32 UDubitoTableHudWidget::ResolveLocalPlayerId() const
{
	APlayerController* OwningController = GetOwningPlayer();
	if (const ADubitoPlayerController* DubitoController = Cast<ADubitoPlayerController>(OwningController))
	{
		return DubitoController->ResolveAuthorityPlayerId();
	}

	if (OwningController)
	{
		if (const ADubitoPlayerState* DubitoPlayerState = OwningController->GetPlayerState<ADubitoPlayerState>())
		{
			return DubitoPlayerState->GetDubitoPlayerId();
		}
	}

	return DubitoConstants::NoPlayerId;
}

int32 UDubitoTableHudWidget::ResolveLocalHandCount() const
{
	if (const ADubitoPlayerController* DubitoController = Cast<ADubitoPlayerController>(GetOwningPlayer()))
	{
		return DubitoController->GetPrivateHandCount();
	}

	return 0;
}

void UDubitoTableHudWidget::RefreshFromReplicatedState()
{
	FDubitoTableHudInputs Inputs;

	ADubitoGameState* GameState = ResolveGameState();
	if (GameState)
	{
		Inputs.Phase = GameState->GetPublicPhase();
		Inputs.ActivePlayerId = GameState->GetActivePlayerId();
		Inputs.RoundValue = GameState->GetRoundValue();
		Inputs.bHasPreviousPublicClaim = GameState->HasPreviousPublicClaim();
		Inputs.bPreviousClaimDoubtable = GameState->IsPreviousClaimDoubtable();
		Inputs.PreviousClaimantId = GameState->GetPreviousClaimantId();
		Inputs.PreviousPublicAnnouncement = GameState->GetPreviousPublicAnnouncement();
		Inputs.ClaimedPileCount = GameState->GetClaimedPileCount();
		Inputs.bHasPendingWin = GameState->HasPendingWin();
		Inputs.TurnDeadlineServerTimeSeconds = GameState->GetTurnDeadlineServerTimeSeconds();
		Inputs.NowServerTimeSeconds = GameState->GetServerWorldTimeSeconds();

		for (APlayerState* PlayerState : GameState->PlayerArray)
		{
			const ADubitoPlayerState* DubitoPlayerState = Cast<ADubitoPlayerState>(PlayerState);
			if (!DubitoPlayerState || DubitoPlayerState->GetDubitoPlayerId() == DubitoConstants::NoPlayerId)
			{
				continue;
			}

			FDubitoSeatLedgerInput SeatInput;
			SeatInput.PlayerId = DubitoPlayerState->GetDubitoPlayerId();
			SeatInput.SeatIndex = DubitoPlayerState->GetSeatIndex();
			SeatInput.DisplayName = DubitoPlayerState->GetPlayerName();
			SeatInput.PublicHandCount = DubitoPlayerState->GetPublicHandCount();
			SeatInput.bReady = DubitoPlayerState->IsReady();
			Inputs.Seats.Add(MoveTemp(SeatInput));
		}
	}

	Inputs.LocalPlayerId = ResolveLocalPlayerId();
	Inputs.LocalHandCount = ResolveLocalHandCount();

	CachedSnapshot = BuildDubitoTableHudSnapshot(Inputs);

	ApplySnapshotToWidgets();
	OnTableHudSnapshotUpdated(CachedSnapshot);
}

void UDubitoTableHudWidget::ApplySnapshotToWidgets()
{
	if (PhaseText)
	{
		PhaseText->SetText(FText::FromString(FString::Printf(TEXT("Phase: %s"), *PhaseToString(CachedSnapshot.Phase))));
	}

	if (TurnText)
	{
		FString TurnLine;
		if (!CachedSnapshot.bMatchInProgress)
		{
			TurnLine = TEXT("Waiting for match to start");
		}
		else if (CachedSnapshot.ActivePlayerId == DubitoConstants::NoPlayerId)
		{
			TurnLine = TEXT("Turn: --");
		}
		else
		{
			TurnLine = FString::Printf(TEXT("Turn: Player %d%s"),
				CachedSnapshot.ActivePlayerId,
				CachedSnapshot.bIsLocalPlayerActive ? TEXT("  (YOU)") : TEXT(""));
		}
		TurnText->SetText(FText::FromString(TurnLine));
	}

	if (TimerText)
	{
		const FString TimerLine = CachedSnapshot.bHasTurnTimer
			? FString::Printf(TEXT("Time left: %ds"), CachedSnapshot.TurnSecondsRemaining)
			: TEXT("Time left: --");
		TimerText->SetText(FText::FromString(TimerLine));
	}

	if (RoundText)
	{
		const FString RoundLine = CachedSnapshot.bHasRoundValue
			? FString::Printf(TEXT("Round value: %d"), CachedSnapshot.RoundValue)
			: TEXT("Round value: open (empty pile)");
		RoundText->SetText(FText::FromString(RoundLine));
	}

	if (ClaimText)
	{
		FString ClaimLine;
		if (!CachedSnapshot.bHasPublicClaim)
		{
			ClaimLine = TEXT("Claim to judge: none");
		}
		else
		{
			ClaimLine = FString::Printf(TEXT("Claim to judge: Player %d claims %d x value %d%s"),
				CachedSnapshot.ClaimantId,
				CachedSnapshot.PublicClaim.ClaimedCount,
				CachedSnapshot.PublicClaim.ClaimedValue,
				CachedSnapshot.bClaimIsDoubtable ? TEXT("  [doubtable]") : TEXT("  [stale]"));
		}
		ClaimText->SetText(FText::FromString(ClaimLine));
	}

	if (StakeText)
	{
		FString StakeLine = FString::Printf(TEXT("Pile stake (claimed): %d"), CachedSnapshot.ClaimedPileCount);
		if (CachedSnapshot.bHasPendingWin)
		{
			StakeLine += TEXT("   |   WIN PENDING");
		}
		StakeText->SetText(FText::FromString(StakeLine));
	}

	if (ActionsText)
	{
		const FString ActionsLine = FString::Printf(TEXT("Play: %s   Doubt: %s   Discard: %s"),
			*AvailabilityTag(CachedSnapshot.bCanAttemptPlay),
			*AvailabilityTag(CachedSnapshot.bCanAttemptDoubt),
			*AvailabilityTag(CachedSnapshot.bCanAttemptDiscard));
		ActionsText->SetText(FText::FromString(ActionsLine));
	}

	if (SeatsBox)
	{
		SeatsBox->ClearChildren();
		for (const FDubitoSeatLedgerView& Seat : CachedSnapshot.Seats)
		{
			UTextBlock* SeatLine = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
			const FLinearColor SeatColor = Seat.bIsActive
				? FLinearColor(0.96f, 0.86f, 0.55f, 1.0f)
				: FLinearColor(0.78f, 0.82f, 0.84f, 1.0f);
			SeatLine->SetColorAndOpacity(FSlateColor(SeatColor));

			FString SeatName = Seat.DisplayName.ToString();
			if (SeatName.IsEmpty())
			{
				SeatName = FString::Printf(TEXT("Player %d"), Seat.PlayerId);
			}

			const FString SeatText = FString::Printf(TEXT("Seat %d: %s - %d cards%s%s"),
				Seat.SeatIndex,
				*SeatName,
				Seat.PublicHandCount,
				Seat.bIsActive ? TEXT("  <turn>") : TEXT(""),
				Seat.bIsLocal ? TEXT("  (you)") : TEXT(""));
			SeatLine->SetText(FText::FromString(SeatText));

			UVerticalBoxSlot* SeatSlot = SeatsBox->AddChildToVerticalBox(SeatLine);
			SeatSlot->SetPadding(FMargin(0.0f, 1.0f));
		}
	}
}
