#include "DubitoPlayActionWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "DubitoCardVisuals.h"
#include "DubitoGameState.h"
#include "Engine/Texture2D.h"
#include "DubitoPlayerController.h"
#include "DubitoPlayerState.h"
#include "Engine/World.h"
#include "GameFramework/PlayerState.h"
#include "Math/UnrealMathUtility.h"
#include "TimerManager.h"

namespace
{
	const FName ActionRootName(TEXT("PlayActionRoot"));

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

	FString CardToString(const FDubitoCard& Card)
	{
		return RankToString(Card.Rank) + SuitToString(Card.Suit);
	}

	FString ComposeErrorToString(EDubitoPlayComposeError Error)
	{
		switch (Error)
		{
		case EDubitoPlayComposeError::None: return TEXT("Ready to play");
		case EDubitoPlayComposeError::NotYourTurn: return TEXT("Not your turn");
		case EDubitoPlayComposeError::NoCardsSelected: return TEXT("Select 1-4 cards");
		case EDubitoPlayComposeError::InvalidSelection: return TEXT("Invalid selection");
		case EDubitoPlayComposeError::TooManyCardsSelected: return TEXT("Too many cards (max 4)");
		case EDubitoPlayComposeError::ValueNotChosen: return TEXT("Choose a claimed value");
		case EDubitoPlayComposeError::BadClaimedCount: return TEXT("Claim 1-4");
		default: return TEXT("");
		}
	}

	UButton* AddControlButton(UWidgetTree& Tree, UHorizontalBox& Row, const FName Name, const FString& Label,
		UObject* Handler, FName HandlerFn)
	{
		UButton* Button = Tree.ConstructWidget<UButton>(UButton::StaticClass(), Name);

		UTextBlock* Text = Tree.ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		Text->SetText(FText::FromString(Label));
		Text->SetColorAndOpacity(FSlateColor(FLinearColor(0.95f, 0.97f, 0.94f, 1.0f)));
		Button->AddChild(Text);

		FScriptDelegate Delegate;
		Delegate.BindUFunction(Handler, HandlerFn);
		Button->OnClicked.Add(Delegate);

		UHorizontalBoxSlot* Slot = Row.AddChildToHorizontalBox(Button);
		Slot->SetPadding(FMargin(3.0f, 0.0f));
		return Button;
	}
}

UDubitoPlayActionWidget::UDubitoPlayActionWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Interactive: the bar must receive clicks and focus, unlike the read-only HUD overlay.
	SetVisibility(ESlateVisibility::Visible);
}

TSharedRef<SWidget> UDubitoPlayActionWidget::RebuildWidget()
{
	EnsureActionTree();
	return Super::RebuildWidget();
}

void UDubitoPlayActionWidget::EnsureActionTree()
{
	if (!WidgetTree || (WidgetTree->RootWidget && WidgetTree->RootWidget->GetFName() == ActionRootName))
	{
		return;
	}

	UBorder* Root = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), ActionRootName);
	Root->SetBrushColor(FLinearColor(0.03f, 0.035f, 0.045f, 0.90f));
	Root->SetPadding(FMargin(18.0f, 14.0f));
	Root->SetHorizontalAlignment(HAlign_Center);
	Root->SetVerticalAlignment(VAlign_Bottom);

	UVerticalBox* Stack = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
	Root->SetContent(Stack);

	TitleText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	TitleText->SetColorAndOpacity(FSlateColor(FLinearColor(0.72f, 0.77f, 0.80f, 1.0f)));
	Stack->AddChildToVerticalBox(TitleText);

	HandBox = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
	{
		UVerticalBoxSlot* HandSlot = Stack->AddChildToVerticalBox(HandBox);
		HandSlot->SetHorizontalAlignment(HAlign_Center);
		HandSlot->SetPadding(FMargin(0.0f, 8.0f));
	}

	ClaimText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	ClaimText->SetColorAndOpacity(FSlateColor(FLinearColor(0.90f, 0.80f, 0.55f, 1.0f)));
	Stack->AddChildToVerticalBox(ClaimText);

	// Control row: hand cursor, card toggle, value stepper, count stepper, confirm/clear.
	UHorizontalBox* Controls = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
	{
		UVerticalBoxSlot* ControlSlot = Stack->AddChildToVerticalBox(Controls);
		ControlSlot->SetHorizontalAlignment(HAlign_Center);
		ControlSlot->SetPadding(FMargin(0.0f, 8.0f, 0.0f, 0.0f));
	}

	AddControlButton(*WidgetTree, *Controls, TEXT("CursorLeftBtn"), TEXT("<"), this, GET_FUNCTION_NAME_CHECKED(UDubitoPlayActionWidget, HandleCursorLeft));
	AddControlButton(*WidgetTree, *Controls, TEXT("CursorRightBtn"), TEXT(">"), this, GET_FUNCTION_NAME_CHECKED(UDubitoPlayActionWidget, HandleCursorRight));
	AddControlButton(*WidgetTree, *Controls, TEXT("ToggleBtn"), TEXT("Pick/Unpick"), this, GET_FUNCTION_NAME_CHECKED(UDubitoPlayActionWidget, HandleToggleAtCursor));
	AddControlButton(*WidgetTree, *Controls, TEXT("ValueDownBtn"), TEXT("Value -"), this, GET_FUNCTION_NAME_CHECKED(UDubitoPlayActionWidget, HandleValueDown));
	AddControlButton(*WidgetTree, *Controls, TEXT("ValueUpBtn"), TEXT("Value +"), this, GET_FUNCTION_NAME_CHECKED(UDubitoPlayActionWidget, HandleValueUp));
	AddControlButton(*WidgetTree, *Controls, TEXT("CountDownBtn"), TEXT("Claim -"), this, GET_FUNCTION_NAME_CHECKED(UDubitoPlayActionWidget, HandleCountDown));
	AddControlButton(*WidgetTree, *Controls, TEXT("CountUpBtn"), TEXT("Claim +"), this, GET_FUNCTION_NAME_CHECKED(UDubitoPlayActionWidget, HandleCountUp));
	AddControlButton(*WidgetTree, *Controls, TEXT("ConfirmBtn"), TEXT("PLAY"), this, GET_FUNCTION_NAME_CHECKED(UDubitoPlayActionWidget, HandleConfirm));
	AddControlButton(*WidgetTree, *Controls, TEXT("ClearBtn"), TEXT("Clear"), this, GET_FUNCTION_NAME_CHECKED(UDubitoPlayActionWidget, HandleClear));

	// Doubt is hold-to-confirm, not a one-tap destructive action (Documentation/DESIGN.md):
	// bind press/release so holding the button past the threshold sends the Doubt.
	{
		UButton* DoubtButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("DoubtHoldBtn"));

		UTextBlock* DoubtLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		DoubtLabel->SetText(FText::FromString(TEXT("Hold to DOUBT")));
		DoubtLabel->SetColorAndOpacity(FSlateColor(FLinearColor(0.98f, 0.86f, 0.60f, 1.0f)));
		DoubtButton->AddChild(DoubtLabel);

		FScriptDelegate PressedDelegate;
		PressedDelegate.BindUFunction(this, GET_FUNCTION_NAME_CHECKED(UDubitoPlayActionWidget, HandleDoubtPressed));
		DoubtButton->OnPressed.Add(PressedDelegate);

		FScriptDelegate ReleasedDelegate;
		ReleasedDelegate.BindUFunction(this, GET_FUNCTION_NAME_CHECKED(UDubitoPlayActionWidget, HandleDoubtReleased));
		DoubtButton->OnReleased.Add(ReleasedDelegate);

		UHorizontalBoxSlot* DoubtSlot = Controls->AddChildToHorizontalBox(DoubtButton);
		DoubtSlot->SetPadding(FMargin(12.0f, 0.0f, 3.0f, 0.0f));
	}

	// Discard uses a light confirmation: a first click arms it, a second click confirms.
	AddControlButton(*WidgetTree, *Controls, TEXT("DiscardBtn"), TEXT("Discard"), this, GET_FUNCTION_NAME_CHECKED(UDubitoPlayActionWidget, HandleDiscard));

	DoubtText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	DoubtText->SetColorAndOpacity(FSlateColor(FLinearColor(0.98f, 0.86f, 0.60f, 1.0f)));
	{
		UVerticalBoxSlot* DoubtSlot = Stack->AddChildToVerticalBox(DoubtText);
		DoubtSlot->SetPadding(FMargin(0.0f, 8.0f, 0.0f, 0.0f));
	}

	DiscardText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	DiscardText->SetColorAndOpacity(FSlateColor(FLinearColor(0.82f, 0.74f, 0.70f, 1.0f)));
	{
		UVerticalBoxSlot* DiscardSlot = Stack->AddChildToVerticalBox(DiscardText);
		DiscardSlot->SetPadding(FMargin(0.0f, 4.0f, 0.0f, 0.0f));
	}

	StatusText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	StatusText->SetColorAndOpacity(FSlateColor(FLinearColor(0.58f, 0.82f, 0.74f, 1.0f)));
	{
		UVerticalBoxSlot* StatusSlot = Stack->AddChildToVerticalBox(StatusText);
		StatusSlot->SetPadding(FMargin(0.0f, 8.0f, 0.0f, 0.0f));
	}

	WidgetTree->RootWidget = Root;
}

void UDubitoPlayActionWidget::NativeConstruct()
{
	Super::NativeConstruct();

	BindToGameState();
	RefreshFromReplicatedState();
}

void UDubitoPlayActionWidget::NativeDestruct()
{
	CancelDoubtHold();
	DisarmDiscard();
	ClearPendingRequest();
	UnbindFromGameState();
	Super::NativeDestruct();
}

ADubitoGameState* UDubitoPlayActionWidget::ResolveGameState() const
{
	const UWorld* World = GetWorld();
	return World ? World->GetGameState<ADubitoGameState>() : nullptr;
}

ADubitoPlayerController* UDubitoPlayActionWidget::ResolveDubitoController() const
{
	return Cast<ADubitoPlayerController>(GetOwningPlayer());
}

int32 UDubitoPlayActionWidget::ResolveLocalPlayerId() const
{
	if (const ADubitoPlayerController* Controller = ResolveDubitoController())
	{
		return Controller->ResolveAuthorityPlayerId();
	}
	return DubitoConstants::NoPlayerId;
}

void UDubitoPlayActionWidget::BindToGameState()
{
	ADubitoGameState* GameState = ResolveGameState();
	if (!GameState || BoundGameState.Get() == GameState)
	{
		return;
	}

	UnbindFromGameState();
	PublicStateChangedHandle = GameState->OnPublicMatchStateChanged.AddUObject(this, &UDubitoPlayActionWidget::HandlePublicMatchStateChanged);
	BoundGameState = GameState;
}

void UDubitoPlayActionWidget::UnbindFromGameState()
{
	if (ADubitoGameState* GameState = BoundGameState.Get())
	{
		GameState->OnPublicMatchStateChanged.Remove(PublicStateChangedHandle);
	}
	PublicStateChangedHandle.Reset();
	BoundGameState = nullptr;
}

void UDubitoPlayActionWidget::HandlePublicMatchStateChanged()
{
	// Authoritative state moved on (our play/doubt/discard confirmed, or the turn passed). Drop
	// any stale in-progress intent so the bar always reflects the current turn's legal choices,
	// and release the pending-request guard now that the server has answered.
	CancelDoubtHold();
	DisarmDiscard();
	ClearPendingRequest();
	ClearIntent();
}

FDubitoPlaySelectionInputs UDubitoPlayActionWidget::GatherInputs() const
{
	FDubitoPlaySelectionInputs Inputs;

	const ADubitoPlayerController* Controller = ResolveDubitoController();
	if (Controller)
	{
		Inputs.LocalHand = Controller->GetPrivateHandCards();
	}

	if (const ADubitoGameState* GameState = ResolveGameState())
	{
		Inputs.Phase = GameState->GetPublicPhase();
		Inputs.RoundValue = GameState->GetRoundValue();
		Inputs.bIsLocalPlayerActive = ResolveLocalPlayerId() != DubitoConstants::NoPlayerId
			&& GameState->GetActivePlayerId() == ResolveLocalPlayerId();
	}

	Inputs.SelectedHandIndices = SelectedIndices;
	Inputs.ChosenValue = ChosenValue;
	Inputs.ClaimedCount = ClaimedCount;
	return Inputs;
}

void UDubitoPlayActionWidget::RefreshFromReplicatedState()
{
	// Keep the cursor inside the current hand.
	const int32 HandNum = CachedView.Cards.Num();
	const FDubitoPlaySelectionInputs Inputs = GatherInputs();
	const int32 LiveHandNum = Inputs.LocalHand.Num();
	if (LiveHandNum <= 0)
	{
		CursorIndex = 0;
	}
	else
	{
		CursorIndex = FMath::Clamp(CursorIndex, 0, LiveHandNum - 1);
	}

	CachedView = BuildDubitoPlaySelectionView(Inputs);

	ApplyViewToWidgets();
	OnPlaySelectionViewUpdated(CachedView);
	(void)HandNum;
}

void UDubitoPlayActionWidget::ToggleCardSelected(int32 HandIndex)
{
	if (SelectedIndices.Contains(HandIndex))
	{
		SelectedIndices.Remove(HandIndex);
	}
	else
	{
		SelectedIndices.Add(HandIndex);
	}
	RefreshFromReplicatedState();
}

void UDubitoPlayActionWidget::SetChosenValue(int32 Value)
{
	ChosenValue = FMath::Clamp(Value, DubitoConstants::MinCardRank, DubitoConstants::MaxCardRank);
	RefreshFromReplicatedState();
}

void UDubitoPlayActionWidget::StepChosenValue(int32 Delta)
{
	ChosenValue = FMath::Clamp(ChosenValue + Delta, DubitoConstants::MinCardRank, DubitoConstants::MaxCardRank);
}

void UDubitoPlayActionWidget::StepClaimedCount(int32 Delta)
{
	ClaimedCount = FMath::Clamp(ClaimedCount + Delta, DubitoConstants::MinCardsPerPlay, DubitoConstants::MaxCardsPerPlay);
	RefreshFromReplicatedState();
}

void UDubitoPlayActionWidget::ClearIntent()
{
	SelectedIndices.Reset();
	ChosenValue = DubitoConstants::NoRoundValue;
	ClaimedCount = DubitoConstants::MinCardsPerPlay;
	RefreshFromReplicatedState();
}

bool UDubitoPlayActionWidget::ConfirmPlay()
{
	if (bAwaitingServerResponse)
	{
		// A request is already in flight; ignore the spammed submit.
		return false;
	}

	RefreshFromReplicatedState();
	if (!CachedView.bIsSubmittable)
	{
		return false;
	}

	ADubitoPlayerController* Controller = ResolveDubitoController();
	if (!Controller)
	{
		return false;
	}

	Controller->RequestPlayCards(CachedView.ActualCards, CachedView.Announcement);
	BeginPendingRequest();

	// Optimistically clear; the confirmed hand/turn will arrive as replicated state and the
	// bound state-changed handler will refresh the bar again.
	ClearIntent();
	return true;
}

void UDubitoPlayActionWidget::BeginPendingRequest()
{
	bAwaitingServerResponse = true;
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(PendingRequestTimerHandle, this, &UDubitoPlayActionWidget::ClearPendingRequest, PendingRequestSafetySeconds, false);
	}
}

void UDubitoPlayActionWidget::ClearPendingRequest()
{
	bAwaitingServerResponse = false;
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(PendingRequestTimerHandle);
	}
}

void UDubitoPlayActionWidget::HandleCursorLeft()
{
	CursorIndex = FMath::Max(0, CursorIndex - 1);
	RefreshFromReplicatedState();
}

void UDubitoPlayActionWidget::HandleCursorRight()
{
	CursorIndex = CursorIndex + 1; // clamped against the live hand in RefreshFromReplicatedState
	RefreshFromReplicatedState();
}

void UDubitoPlayActionWidget::HandleToggleAtCursor()
{
	const FDubitoPlaySelectionInputs Inputs = GatherInputs();
	if (Inputs.LocalHand.IsValidIndex(CursorIndex))
	{
		ToggleCardSelected(CursorIndex);
	}
}

void UDubitoPlayActionWidget::HandleValueDown()
{
	StepChosenValue(-1);
	RefreshFromReplicatedState();
}

void UDubitoPlayActionWidget::HandleValueUp()
{
	StepChosenValue(1);
	RefreshFromReplicatedState();
}

void UDubitoPlayActionWidget::HandleCountDown()
{
	StepClaimedCount(-1);
}

void UDubitoPlayActionWidget::HandleCountUp()
{
	StepClaimedCount(1);
}

void UDubitoPlayActionWidget::HandleConfirm()
{
	ConfirmPlay();
}

void UDubitoPlayActionWidget::HandleClear()
{
	ClearIntent();
}

bool UDubitoPlayActionWidget::IsDoubtAvailable() const
{
	const ADubitoGameState* GameState = ResolveGameState();
	if (!GameState)
	{
		return false;
	}

	// Mirrors DubitoRules::CanDoubt from public state: the local active player, on a player
	// turn, with a doubtable previous claim.
	const int32 LocalId = ResolveLocalPlayerId();
	return LocalId != DubitoConstants::NoPlayerId
		&& GameState->GetPublicPhase() == EDubitoPhase::PlayerTurn
		&& GameState->GetActivePlayerId() == LocalId
		&& GameState->IsPreviousClaimDoubtable();
}

void UDubitoPlayActionWidget::BeginDoubtHold()
{
	if (bDoubtHoldActive || bAwaitingServerResponse || !IsDoubtAvailable())
	{
		return;
	}

	bDoubtHoldActive = true;
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(DoubtHoldTimerHandle, this, &UDubitoPlayActionWidget::CompleteDoubtHold, DoubtHoldSeconds, false);
	}
	RefreshFromReplicatedState();
}

void UDubitoPlayActionWidget::CancelDoubtHold()
{
	if (!bDoubtHoldActive)
	{
		return;
	}

	bDoubtHoldActive = false;
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(DoubtHoldTimerHandle);
	}
}

void UDubitoPlayActionWidget::CompleteDoubtHold()
{
	bDoubtHoldActive = false;

	// Re-check availability at the moment the hold completes; the turn may have moved on.
	if (!IsDoubtAvailable())
	{
		RefreshFromReplicatedState();
		return;
	}

	if (ADubitoPlayerController* Controller = ResolveDubitoController())
	{
		Controller->RequestDoubt();
		BeginPendingRequest();
	}
	RefreshFromReplicatedState();
}

void UDubitoPlayActionWidget::HandleDoubtPressed()
{
	BeginDoubtHold();
}

void UDubitoPlayActionWidget::HandleDoubtReleased()
{
	// Released before the threshold fired: cancel without doubting.
	CancelDoubtHold();
	RefreshFromReplicatedState();
}

FDubitoDiscardView UDubitoPlayActionWidget::GetDiscardView() const
{
	FDubitoDiscardInputs In;
	if (const ADubitoGameState* GameState = ResolveGameState())
	{
		const int32 LocalId = ResolveLocalPlayerId();
		In.Phase = GameState->GetPublicPhase();
		In.bIsLocalPlayerActive = LocalId != DubitoConstants::NoPlayerId && GameState->GetActivePlayerId() == LocalId;
		In.ClaimedPileCount = GameState->GetClaimedPileCount();
		In.bHasPendingWin = GameState->HasPendingWin();
	}
	return BuildDubitoDiscardView(In);
}

void UDubitoPlayActionWidget::RequestDiscardConfirm()
{
	if (bAwaitingServerResponse)
	{
		// A request is already in flight; ignore the spammed submit.
		return;
	}

	if (!GetDiscardView().bCanDiscard)
	{
		// Not discardable right now; leave it disarmed and let the status line explain why.
		DisarmDiscard();
		RefreshFromReplicatedState();
		return;
	}

	if (!bDiscardArmed)
	{
		// First press arms; the status line asks for a confirming second press.
		bDiscardArmed = true;
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimer(DiscardArmTimerHandle, this, &UDubitoPlayActionWidget::DisarmDiscard, DiscardArmSeconds, false);
		}
		RefreshFromReplicatedState();
		return;
	}

	// Second press within the window confirms and sends the Discard.
	DisarmDiscard();
	if (ADubitoPlayerController* Controller = ResolveDubitoController())
	{
		Controller->RequestDiscard();
		BeginPendingRequest();
	}
	RefreshFromReplicatedState();
}

void UDubitoPlayActionWidget::CancelDiscardArm()
{
	DisarmDiscard();
	RefreshFromReplicatedState();
}

void UDubitoPlayActionWidget::DisarmDiscard()
{
	bDiscardArmed = false;
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(DiscardArmTimerHandle);
	}
}

void UDubitoPlayActionWidget::HandleDiscard()
{
	RequestDiscardConfirm();
}

void UDubitoPlayActionWidget::ApplyViewToWidgets()
{
	if (TitleText)
	{
		FString Title;
		if (!CachedView.bCanCompose)
		{
			Title = TEXT("Play: waiting for your turn");
		}
		else if (CachedView.bIsOpeningRound)
		{
			Title = TEXT("Your turn - opening the round (choose value + count)");
		}
		else
		{
			Title = FString::Printf(TEXT("Your turn - round value %d locked (choose count)"), CachedView.EffectiveValue);
		}
		TitleText->SetText(FText::FromString(Title));
	}

	if (HandBox)
	{
		HandBox->ClearChildren();
		for (const FDubitoPlayCardView& CardView : CachedView.Cards)
		{
			const bool bAtCursor = CardView.HandIndex == CursorIndex;

			// The selection/cursor state reads through the framing border: gold when picked, blue
			// when under the cursor, dark otherwise. The card face itself is the imported texture.
			UBorder* CardBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
			const FLinearColor BorderColor = CardView.bSelected
				? FLinearColor(0.96f, 0.80f, 0.30f, 1.0f)
				: (bAtCursor ? FLinearColor(0.55f, 0.80f, 0.98f, 1.0f) : FLinearColor(0.10f, 0.12f, 0.15f, 1.0f));
			CardBorder->SetBrushColor(BorderColor);
			CardBorder->SetPadding(FMargin(3.0f));

			if (UTexture2D* FaceTexture = LoadDubitoCardFaceTexture(CardView.Card))
			{
				UImage* CardImage = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());
				CardImage->SetBrushFromTexture(FaceTexture, false);
				CardImage->SetDesiredSizeOverride(FVector2D(64.0f, 90.0f));
				CardBorder->SetContent(CardImage);
			}
			else
			{
				// Fallback when the owner-local card pack is absent: keep the legible text label.
				UTextBlock* CardText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
				CardText->SetText(FText::FromString(CardToString(CardView.Card)));
				CardText->SetColorAndOpacity(FSlateColor(FLinearColor(0.92f, 0.94f, 0.92f, 1.0f)));
				CardBorder->SetContent(CardText);
			}

			UHorizontalBoxSlot* CardSlot = HandBox->AddChildToHorizontalBox(CardBorder);
			CardSlot->SetPadding(FMargin(4.0f, 0.0f));
			// Lift the focused card so the cursor position is obvious at a glance.
			CardBorder->SetRenderTranslation(FVector2D(0.0f, bAtCursor ? -10.0f : 0.0f));
		}
	}

	if (ClaimText)
	{
		const FString ValueLabel = CachedView.bIsOpeningRound
			? (CachedView.Announcement.HasValidValue() ? RankToString(CachedView.EffectiveValue) : FString(TEXT("--")))
			: RankToString(CachedView.EffectiveValue);
		const FString ClaimLine = FString::Printf(TEXT("Claim: value %s x count %d   (selected %d actual)"),
			*ValueLabel,
			CachedView.ClaimedCount,
			CachedView.SelectedCount);
		ClaimText->SetText(FText::FromString(ClaimLine));
	}

	if (DoubtText)
	{
		FString DoubtLine;
		if (bDoubtHoldActive)
		{
			DoubtLine = TEXT("Doubt: holding... release cancels");
		}
		else if (IsDoubtAvailable())
		{
			const ADubitoGameState* GameState = ResolveGameState();
			const FDubitoAnnouncement Claim = GameState ? GameState->GetPreviousPublicAnnouncement() : FDubitoAnnouncement();
			DoubtLine = FString::Printf(TEXT("Doubt: available - Player %d claimed %d x value %d (hold to challenge)"),
				GameState ? GameState->GetPreviousClaimantId() : DubitoConstants::NoPlayerId,
				Claim.ClaimedCount,
				Claim.ClaimedValue);
		}
		else
		{
			DoubtLine = TEXT("Doubt: unavailable");
		}
		DoubtText->SetText(FText::FromString(DoubtLine));
	}

	if (DiscardText)
	{
		const FDubitoDiscardView Discard = GetDiscardView();
		FString DiscardLine;
		if (bDiscardArmed)
		{
			DiscardLine = TEXT("Discard: press Discard again to confirm (clears the pile, skips your turn)");
		}
		else if (Discard.bCanDiscard)
		{
			DiscardLine = TEXT("Discard: available - click to confirm clearing the pile");
		}
		else
		{
			switch (Discard.BlockedReason)
			{
			case EDubitoDiscardBlockedReason::NotYourTurn: DiscardLine = TEXT("Discard: only on your turn"); break;
			case EDubitoDiscardBlockedReason::PendingWin: DiscardLine = TEXT("Discard: blocked while a win is pending"); break;
			case EDubitoDiscardBlockedReason::EmptyPile: DiscardLine = TEXT("Discard: pile is empty, nothing to discard"); break;
			default: DiscardLine = TEXT("Discard: unavailable"); break;
			}
		}
		DiscardText->SetText(FText::FromString(DiscardLine));
	}

	if (StatusText)
	{
		FString Status;
		if (bAwaitingServerResponse)
		{
			Status = TEXT("Sending... waiting for the server");
		}
		else
		{
			Status = ComposeErrorToString(CachedView.Error);
			if (CachedView.bIsSubmittable)
			{
				Status = TEXT("Ready - press PLAY to send");
			}
		}
		StatusText->SetText(FText::FromString(Status));
	}
}
