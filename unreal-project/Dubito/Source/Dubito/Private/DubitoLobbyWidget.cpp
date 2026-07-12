#include "DubitoLobbyWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "DubitoGameState.h"
#include "DubitoPlayerController.h"
#include "DubitoPlayerState.h"
#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "Math/UnrealMathUtility.h"
#include "TimerManager.h"

namespace
{
	const FName LobbyRootName(TEXT("LobbyRoot"));
	constexpr float LobbyRefreshSeconds = 0.5f;

	FString StartBlockedToString(EDubitoLobbyStartBlockedReason Reason)
	{
		switch (Reason)
		{
		case EDubitoLobbyStartBlockedReason::None: return TEXT("START MATCH");
		case EDubitoLobbyStartBlockedReason::AlreadyStarted: return TEXT("Match in progress");
		case EDubitoLobbyStartBlockedReason::NotHost: return TEXT("Waiting for the host to start");
		case EDubitoLobbyStartBlockedReason::WaitingForPlayers: return TEXT("Waiting for players (2-8)");
		case EDubitoLobbyStartBlockedReason::NotAllReady: return TEXT("Waiting for all players to ready up");
		default: return TEXT("Start unavailable");
		}
	}

	UButton* MakeLabeledButton(UWidgetTree& Tree, const FName Name, UObject* Handler, FName HandlerFn, TObjectPtr<UTextBlock>& OutLabel)
	{
		UButton* Button = Tree.ConstructWidget<UButton>(UButton::StaticClass(), Name);

		OutLabel = Tree.ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		OutLabel->SetJustification(ETextJustify::Center);
		OutLabel->SetColorAndOpacity(FSlateColor(FLinearColor(0.95f, 0.97f, 0.94f, 1.0f)));
		Button->AddChild(OutLabel);

		FScriptDelegate Delegate;
		Delegate.BindUFunction(Handler, HandlerFn);
		Button->OnClicked.Add(Delegate);
		return Button;
	}
}

UDubitoLobbyWidget::UDubitoLobbyWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Hidden until the first refresh confirms we are in the lobby with a local seat.
	SetVisibility(ESlateVisibility::Collapsed);
}

TSharedRef<SWidget> UDubitoLobbyWidget::RebuildWidget()
{
	EnsureLobbyTree();
	return Super::RebuildWidget();
}

void UDubitoLobbyWidget::EnsureLobbyTree()
{
	if (!WidgetTree || (WidgetTree->RootWidget && WidgetTree->RootWidget->GetFName() == LobbyRootName))
	{
		return;
	}

	UBorder* Root = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), LobbyRootName);
	Root->SetBrushColor(FLinearColor(0.03f, 0.04f, 0.06f, 0.94f));
	Root->SetPadding(FMargin(34.0f, 28.0f));
	Root->SetHorizontalAlignment(HAlign_Center);
	Root->SetVerticalAlignment(VAlign_Center);

	UVerticalBox* Stack = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
	Root->SetContent(Stack);

	TitleText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	TitleText->SetJustification(ETextJustify::Center);
	TitleText->SetColorAndOpacity(FSlateColor(FLinearColor(0.98f, 0.86f, 0.42f, 1.0f)));
	TitleText->SetText(FText::FromString(TEXT("WAITING ROOM")));
	{
		UVerticalBoxSlot* BoxSlot =Stack->AddChildToVerticalBox(TitleText);
		BoxSlot->SetHorizontalAlignment(HAlign_Center);
		BoxSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 12.0f));
	}

	SeatBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
	{
		UVerticalBoxSlot* BoxSlot =Stack->AddChildToVerticalBox(SeatBox);
		BoxSlot->SetHorizontalAlignment(HAlign_Center);
		BoxSlot->SetPadding(FMargin(0.0f, 4.0f));
	}

	ReadyButton = MakeLabeledButton(*WidgetTree, TEXT("ReadyBtn"), this,
		GET_FUNCTION_NAME_CHECKED(UDubitoLobbyWidget, HandleReadyToggle), ReadyLabel);
	{
		UVerticalBoxSlot* BoxSlot =Stack->AddChildToVerticalBox(ReadyButton);
		BoxSlot->SetHorizontalAlignment(HAlign_Fill);
		BoxSlot->SetPadding(FMargin(0.0f, 10.0f, 0.0f, 4.0f));
	}

	StartButton = MakeLabeledButton(*WidgetTree, TEXT("StartBtn"), this,
		GET_FUNCTION_NAME_CHECKED(UDubitoLobbyWidget, HandleStart), StartLabel);
	{
		UVerticalBoxSlot* BoxSlot =Stack->AddChildToVerticalBox(StartButton);
		BoxSlot->SetHorizontalAlignment(HAlign_Fill);
		BoxSlot->SetPadding(FMargin(0.0f, 4.0f));
	}

	StatusText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	StatusText->SetJustification(ETextJustify::Center);
	StatusText->SetColorAndOpacity(FSlateColor(FLinearColor(0.58f, 0.82f, 0.74f, 1.0f)));
	{
		UVerticalBoxSlot* BoxSlot =Stack->AddChildToVerticalBox(StatusText);
		BoxSlot->SetHorizontalAlignment(HAlign_Center);
		BoxSlot->SetPadding(FMargin(0.0f, 10.0f, 0.0f, 0.0f));
	}

	WidgetTree->RootWidget = Root;
}

void UDubitoLobbyWidget::NativeConstruct()
{
	Super::NativeConstruct();

	BindToGameState();

	// New joiners and remote ready toggles arrive as replicated PlayerState changes; a light poll
	// keeps the seat list current without subscribing to every player's state delegate.
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(RefreshTimerHandle, this, &UDubitoLobbyWidget::Refresh, LobbyRefreshSeconds, true);
	}

	Refresh();
}

void UDubitoLobbyWidget::NativeDestruct()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(RefreshTimerHandle);
	}
	UnbindFromGameState();
	Super::NativeDestruct();
}

ADubitoGameState* UDubitoLobbyWidget::ResolveGameState() const
{
	const UWorld* World = GetWorld();
	return World ? World->GetGameState<ADubitoGameState>() : nullptr;
}

ADubitoPlayerController* UDubitoLobbyWidget::ResolveDubitoController() const
{
	return Cast<ADubitoPlayerController>(GetOwningPlayer());
}

int32 UDubitoLobbyWidget::ResolveLocalPlayerId() const
{
	if (const ADubitoPlayerController* Controller = ResolveDubitoController())
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

void UDubitoLobbyWidget::BindToGameState()
{
	ADubitoGameState* GameState = ResolveGameState();
	if (!GameState || BoundGameState.Get() == GameState)
	{
		return;
	}

	UnbindFromGameState();
	PublicStateChangedHandle = GameState->OnPublicMatchStateChanged.AddUObject(this, &UDubitoLobbyWidget::HandlePublicMatchStateChanged);
	BoundGameState = GameState;
}

void UDubitoLobbyWidget::UnbindFromGameState()
{
	if (ADubitoGameState* GameState = BoundGameState.Get())
	{
		GameState->OnPublicMatchStateChanged.Remove(PublicStateChangedHandle);
	}
	PublicStateChangedHandle.Reset();
	BoundGameState = nullptr;
}

void UDubitoLobbyWidget::HandlePublicMatchStateChanged()
{
	// Refresh immediately so the lobby hides the instant the deal moves us out of the lobby phase.
	Refresh();
}

FDubitoLobbyInputs UDubitoLobbyWidget::GatherInputs() const
{
	FDubitoLobbyInputs Inputs;

	const ADubitoGameState* GameState = ResolveGameState();
	if (!GameState)
	{
		return Inputs;
	}

	Inputs.Phase = GameState->GetPublicPhase();

	// The host is the listen-server instance: its local controller holds network authority.
	if (const APlayerController* Owner = GetOwningPlayer())
	{
		Inputs.bLocalIsHost = Owner->HasAuthority();
	}

	const int32 LocalId = ResolveLocalPlayerId();
	for (APlayerState* PlayerState : GameState->PlayerArray)
	{
		const ADubitoPlayerState* DubitoPlayerState = Cast<ADubitoPlayerState>(PlayerState);
		if (!DubitoPlayerState || DubitoPlayerState->GetDubitoPlayerId() == DubitoConstants::NoPlayerId || DubitoPlayerState->GetSeatIndex() < 0)
		{
			continue;
		}

		FDubitoLobbySeatInput Seat;
		Seat.PlayerId = DubitoPlayerState->GetDubitoPlayerId();
		Seat.SeatIndex = DubitoPlayerState->GetSeatIndex();
		Seat.bReady = DubitoPlayerState->IsReady();
		Seat.bIsLocal = LocalId != DubitoConstants::NoPlayerId && Seat.PlayerId == LocalId;
		Inputs.Seats.Add(Seat);
	}

	return Inputs;
}

void UDubitoLobbyWidget::Refresh()
{
	CachedView = BuildDubitoLobbyView(GatherInputs());

	if (!CachedView.bValid || !CachedView.bInLobby)
	{
		SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	SetVisibility(ESlateVisibility::Visible);
	ApplyViewToWidgets();
}

void UDubitoLobbyWidget::ApplyViewToWidgets()
{
	if (SeatBox)
	{
		SeatBox->ClearChildren();
		for (const FDubitoLobbySeatView& Seat : CachedView.Seats)
		{
			UTextBlock* Row = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
			Row->SetJustification(ETextJustify::Center);

			FString Line = FString::Printf(TEXT("Seat %d - Player %d"), Seat.SeatIndex, Seat.PlayerId);
			if (Seat.bIsHost)
			{
				Line += TEXT("  [HOST]");
			}
			if (Seat.bIsLocal)
			{
				Line += TEXT("  [YOU]");
			}
			Line += Seat.bReady ? TEXT("  -  READY") : TEXT("  -  not ready");
			Row->SetText(FText::FromString(Line));

			const FLinearColor RowColor = Seat.bReady
				? FLinearColor(0.60f, 0.92f, 0.66f, 1.0f)
				: FLinearColor(0.82f, 0.80f, 0.74f, 1.0f);
			Row->SetColorAndOpacity(FSlateColor(RowColor));

			UVerticalBoxSlot* BoxSlot =SeatBox->AddChildToVerticalBox(Row);
			BoxSlot->SetHorizontalAlignment(HAlign_Center);
			BoxSlot->SetPadding(FMargin(0.0f, 2.0f));
		}
	}

	if (ReadyLabel)
	{
		ReadyLabel->SetText(FText::FromString(CachedView.bLocalReady ? TEXT("Cancel Ready") : TEXT("Ready up")));
	}

	if (StartLabel)
	{
		StartLabel->SetText(FText::FromString(StartBlockedToString(CachedView.StartBlockedReason)));
	}

	if (StartButton)
	{
		// Only the host ever sees an enabled Start; everyone else sees the waiting label disabled.
		StartButton->SetIsEnabled(CachedView.bCanStart);
		StartButton->SetVisibility(CachedView.bIsLocalHost ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}

	if (StatusText)
	{
		const FString Status = FString::Printf(TEXT("%d player(s) seated, %d ready"), CachedView.PlayerCount, CachedView.ReadyCount);
		StatusText->SetText(FText::FromString(Status));
	}
}

void UDubitoLobbyWidget::HandleReadyToggle()
{
	if (ADubitoPlayerController* Controller = ResolveDubitoController())
	{
		Controller->RequestReady(!CachedView.bLocalReady);
	}
	Refresh();
}

void UDubitoLobbyWidget::HandleStart()
{
	Refresh();
	if (!CachedView.bCanStart)
	{
		return;
	}

	if (ADubitoPlayerController* Controller = ResolveDubitoController())
	{
		// A fresh seed each start so repeated local sessions do not deal an identical shuffle.
		Controller->RequestStartMatch(FMath::Rand());
	}
}
