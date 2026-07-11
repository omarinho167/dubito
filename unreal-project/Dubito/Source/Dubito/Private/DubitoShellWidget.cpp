#include "DubitoShellWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "DubitoShellButton.h"
#include "UObject/UObjectGlobals.h"
#include "Widgets/SWidget.h"

namespace
{
	const FName ShellRootName(TEXT("ShellRoot"));
	const FName ShellStackName(TEXT("ShellStack"));
	const FName ShellTitleName(TEXT("ShellTitleText"));
	const FName ShellSubtitleName(TEXT("ShellSubtitleText"));
	const FName ShellPrimaryName(TEXT("ShellPrimaryPlaceholderText"));
	const FName ShellButtonRowName(TEXT("ShellButtonRow"));
	const FName ShellPrimaryActionButtonName(TEXT("ShellPrimaryActionButton"));
	const FName ShellSecondaryActionButtonName(TEXT("ShellSecondaryActionButton"));
	const FName ShellInputHintName(TEXT("ShellInputHintText"));

	bool IsBlankText(const FText& Text)
	{
		return Text.ToString().TrimStartAndEnd().IsEmpty();
	}

	FText ResolveShellText(const FText& Text, const FString& Fallback)
	{
		return IsBlankText(Text) ? FText::FromString(Fallback) : Text;
	}

	UTextBlock* BuildTextBlock(UWidgetTree& InWidgetTree, const FName Name, const FLinearColor Color)
	{
		UTextBlock* TextBlock = InWidgetTree.ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), Name);
		TextBlock->SetAutoWrapText(true);
		TextBlock->SetColorAndOpacity(FSlateColor(Color));
		TextBlock->SetJustification(ETextJustify::Left);
		return TextBlock;
	}

	void AddStackChild(UVerticalBox& Stack, UWidget& Child, const FMargin Padding)
	{
		UVerticalBoxSlot* Slot = Stack.AddChildToVerticalBox(&Child);
		Slot->SetHorizontalAlignment(HAlign_Fill);
		Slot->SetPadding(Padding);
	}

	void AddButtonRowChild(UHorizontalBox& Row, UWidget& Child, const FMargin Padding)
	{
		UHorizontalBoxSlot* Slot = Row.AddChildToHorizontalBox(&Child);
		Slot->SetHorizontalAlignment(HAlign_Left);
		Slot->SetPadding(Padding);
	}
}

UDubitoShellWidget::UDubitoShellWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SurfaceTitle = NSLOCTEXT("DubitoShell", "DefaultSurfaceTitle", "Dubito Shell");
	SurfaceSubtitle = NSLOCTEXT("DubitoShell", "DefaultSurfaceSubtitle", "Greybox UI surface");
	PrimaryPlaceholder = NSLOCTEXT("DubitoShell", "DefaultPrimaryPlaceholder", "Placeholder content");
	PrimaryActionLabel = NSLOCTEXT("DubitoShell", "DefaultPrimaryActionLabel", "Confirm");
	SecondaryActionLabel = NSLOCTEXT("DubitoShell", "DefaultSecondaryActionLabel", "Back");
	InputHintText = NSLOCTEXT("DubitoShell", "DefaultInputHint", "Mouse, keyboard, and gamepad focus/confirm baseline");
}

TSharedRef<SWidget> UDubitoShellWidget::RebuildWidget()
{
	EnsurePlaceholderTree();
	return Super::RebuildWidget();
}

void UDubitoShellWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	EnsurePlaceholderTree();
	RefreshPlaceholderText();
}

UWidget* UDubitoShellWidget::NativeGetDesiredFocusTarget() const
{
	if (!WidgetTree)
	{
		return Super::NativeGetDesiredFocusTarget();
	}

	if (UWidget* PrimaryButton = WidgetTree->FindWidget(ShellPrimaryActionButtonName))
	{
		return PrimaryButton;
	}

	return Super::NativeGetDesiredFocusTarget();
}

void UDubitoShellWidget::EnsurePlaceholderTree()
{
	if (!WidgetTree)
	{
		return;
	}

	if (WidgetTree->RootWidget && WidgetTree->RootWidget->GetFName() != ShellRootName)
	{
		return;
	}

	UBorder* Root = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), ShellRootName);
	Root->SetBrushColor(FLinearColor(0.025f, 0.028f, 0.032f, 0.92f));
	Root->SetPadding(FMargin(48.0f, 36.0f));

	UVerticalBox* Stack = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), ShellStackName);
	Root->SetContent(Stack);

	UTextBlock* TitleText = BuildTextBlock(*WidgetTree, ShellTitleName, FLinearColor(0.96f, 0.92f, 0.84f, 1.0f));
	UTextBlock* SubtitleText = BuildTextBlock(*WidgetTree, ShellSubtitleName, FLinearColor(0.72f, 0.77f, 0.80f, 1.0f));
	UTextBlock* PrimaryText = BuildTextBlock(*WidgetTree, ShellPrimaryName, FLinearColor(0.86f, 0.89f, 0.88f, 1.0f));
	UTextBlock* InputHintTextBlock = BuildTextBlock(*WidgetTree, ShellInputHintName, FLinearColor(0.58f, 0.72f, 0.74f, 1.0f));
	UHorizontalBox* ButtonRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), ShellButtonRowName);
	UDubitoShellButton* PrimaryButton = WidgetTree->ConstructWidget<UDubitoShellButton>(UDubitoShellButton::StaticClass(), ShellPrimaryActionButtonName);
	UDubitoShellButton* SecondaryButton = WidgetTree->ConstructWidget<UDubitoShellButton>(UDubitoShellButton::StaticClass(), ShellSecondaryActionButtonName);

	AddStackChild(*Stack, *TitleText, FMargin(0.0f, 0.0f, 0.0f, 12.0f));
	AddStackChild(*Stack, *SubtitleText, FMargin(0.0f, 0.0f, 0.0f, 28.0f));
	AddStackChild(*Stack, *PrimaryText, FMargin(0.0f, 0.0f, 0.0f, 26.0f));
	AddButtonRowChild(*ButtonRow, *PrimaryButton, FMargin(0.0f, 0.0f, 12.0f, 0.0f));
	AddButtonRowChild(*ButtonRow, *SecondaryButton, FMargin(0.0f));
	AddStackChild(*Stack, *ButtonRow, FMargin(0.0f, 0.0f, 0.0f, 16.0f));
	AddStackChild(*Stack, *InputHintTextBlock, FMargin(0.0f));

	WidgetTree->RootWidget = Root;
	RefreshPlaceholderText();
}

void UDubitoShellWidget::RefreshPlaceholderText() const
{
	if (!WidgetTree)
	{
		return;
	}

	UTextBlock* TitleText = Cast<UTextBlock>(WidgetTree->FindWidget(ShellTitleName));
	UTextBlock* SubtitleText = Cast<UTextBlock>(WidgetTree->FindWidget(ShellSubtitleName));
	UTextBlock* PrimaryText = Cast<UTextBlock>(WidgetTree->FindWidget(ShellPrimaryName));
	UTextBlock* InputHintTextBlock = Cast<UTextBlock>(WidgetTree->FindWidget(ShellInputHintName));
	UDubitoShellButton* PrimaryButton = Cast<UDubitoShellButton>(WidgetTree->FindWidget(ShellPrimaryActionButtonName));
	UDubitoShellButton* SecondaryButton = Cast<UDubitoShellButton>(WidgetTree->FindWidget(ShellSecondaryActionButtonName));

	if (TitleText)
	{
		TitleText->SetText(ResolveShellText(SurfaceTitle, GetClass()->GetName()));
	}

	if (SubtitleText)
	{
		SubtitleText->SetText(ResolveShellText(SurfaceSubtitle, TEXT("CommonUI placeholder surface")));
	}

	if (PrimaryText)
	{
		PrimaryText->SetText(ResolveShellText(PrimaryPlaceholder, TEXT("No gameplay authority")));
	}

	if (PrimaryButton)
	{
		PrimaryButton->SetButtonText(ResolveShellText(PrimaryActionLabel, TEXT("Confirm")));
	}

	if (SecondaryButton)
	{
		SecondaryButton->SetButtonText(ResolveShellText(SecondaryActionLabel, TEXT("Back")));
	}

	if (InputHintTextBlock)
	{
		InputHintTextBlock->SetText(ResolveShellText(InputHintText, TEXT("Mouse, keyboard, and gamepad confirm ready")));
	}
}
