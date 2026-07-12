#include "DubitoMainMenuWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "DubitoGameInstance.h"
#include "GameFramework/PlayerController.h"

namespace
{
	const FName MenuRootName(TEXT("MainMenuRoot"));

	UButton* AddMenuButton(UWidgetTree& Tree, UVerticalBox& Stack, const FName Name, const FString& Label,
		UObject* Handler, FName HandlerFn)
	{
		UButton* Button = Tree.ConstructWidget<UButton>(UButton::StaticClass(), Name);

		UTextBlock* Text = Tree.ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		Text->SetText(FText::FromString(Label));
		Text->SetJustification(ETextJustify::Center);
		Text->SetColorAndOpacity(FSlateColor(FLinearColor(0.95f, 0.97f, 0.94f, 1.0f)));
		Button->AddChild(Text);

		FScriptDelegate Delegate;
		Delegate.BindUFunction(Handler, HandlerFn);
		Button->OnClicked.Add(Delegate);

		UVerticalBoxSlot* Slot = Stack.AddChildToVerticalBox(Button);
		Slot->SetHorizontalAlignment(HAlign_Fill);
		Slot->SetPadding(FMargin(0.0f, 6.0f));
		return Button;
	}
}

UDubitoMainMenuWidget::UDubitoMainMenuWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetVisibility(ESlateVisibility::Visible);
}

TSharedRef<SWidget> UDubitoMainMenuWidget::RebuildWidget()
{
	EnsureMenuTree();
	return Super::RebuildWidget();
}

void UDubitoMainMenuWidget::EnsureMenuTree()
{
	if (!WidgetTree || (WidgetTree->RootWidget && WidgetTree->RootWidget->GetFName() == MenuRootName))
	{
		return;
	}

	UBorder* Root = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), MenuRootName);
	Root->SetBrushColor(FLinearColor(0.03f, 0.035f, 0.05f, 0.92f));
	Root->SetPadding(FMargin(40.0f, 32.0f));
	Root->SetHorizontalAlignment(HAlign_Center);
	Root->SetVerticalAlignment(VAlign_Center);

	UVerticalBox* Stack = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
	Root->SetContent(Stack);

	UTextBlock* Title = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	Title->SetText(FText::FromString(TEXT("DUBITO")));
	Title->SetJustification(ETextJustify::Center);
	Title->SetColorAndOpacity(FSlateColor(FLinearColor(0.98f, 0.86f, 0.42f, 1.0f)));
	{
		UVerticalBoxSlot* TitleSlot = Stack->AddChildToVerticalBox(Title);
		TitleSlot->SetHorizontalAlignment(HAlign_Center);
		TitleSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 14.0f));
	}

	AddMenuButton(*WidgetTree, *Stack, TEXT("HostBtn"), TEXT("HOST (listen server)"), this, GET_FUNCTION_NAME_CHECKED(UDubitoMainMenuWidget, HandleHost));
	AddMenuButton(*WidgetTree, *Stack, TEXT("JoinBtn"), TEXT("JOIN (127.0.0.1)"), this, GET_FUNCTION_NAME_CHECKED(UDubitoMainMenuWidget, HandleJoin));
	AddMenuButton(*WidgetTree, *Stack, TEXT("QuitBtn"), TEXT("QUIT"), this, GET_FUNCTION_NAME_CHECKED(UDubitoMainMenuWidget, HandleQuit));

	StatusText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	StatusText->SetJustification(ETextJustify::Center);
	StatusText->SetColorAndOpacity(FSlateColor(FLinearColor(0.58f, 0.82f, 0.74f, 1.0f)));
	StatusText->SetText(FText::FromString(TEXT("Host on one instance, Join on the other (same PC).")));
	{
		UVerticalBoxSlot* StatusSlot = Stack->AddChildToVerticalBox(StatusText);
		StatusSlot->SetHorizontalAlignment(HAlign_Center);
		StatusSlot->SetPadding(FMargin(0.0f, 14.0f, 0.0f, 0.0f));
	}

	WidgetTree->RootWidget = Root;
}

void UDubitoMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// A menu needs the cursor and UI focus so its buttons are clickable in a standalone game.
	if (APlayerController* PC = GetOwningPlayer())
	{
		PC->bShowMouseCursor = true;
		FInputModeUIOnly InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PC->SetInputMode(InputMode);
	}
}

UDubitoGameInstance* UDubitoMainMenuWidget::ResolveGameInstance() const
{
	return GetGameInstance<UDubitoGameInstance>();
}

void UDubitoMainMenuWidget::HandleHost()
{
	if (StatusText)
	{
		StatusText->SetText(FText::FromString(TEXT("Hosting... waiting for a player to join.")));
	}

	if (UDubitoGameInstance* GameInstance = ResolveGameInstance())
	{
		GameInstance->HostListenServer();
	}
}

void UDubitoMainMenuWidget::HandleJoin()
{
	if (StatusText)
	{
		StatusText->SetText(FText::FromString(TEXT("Joining 127.0.0.1...")));
	}

	if (UDubitoGameInstance* GameInstance = ResolveGameInstance())
	{
		GameInstance->JoinGameDefault();
	}
}

void UDubitoMainMenuWidget::HandleQuit()
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		PC->ConsoleCommand(TEXT("quit"));
	}
}
