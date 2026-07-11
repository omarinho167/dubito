#include "DubitoShellButton.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Widgets/SWidget.h"

namespace
{
	const FName ButtonRootName(TEXT("ButtonRoot"));
	const FName ButtonLabelName(TEXT("ButtonLabelText"));
}

UDubitoShellButton::UDubitoShellButton(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ButtonText = NSLOCTEXT("DubitoShell", "DefaultShellButtonText", "Confirm");
	ButtonTint = FLinearColor(0.12f, 0.18f, 0.20f, 1.0f);
}

void UDubitoShellButton::SetButtonText(const FText& InText)
{
	ButtonText = InText;
	RefreshButtonText();
}

TSharedRef<SWidget> UDubitoShellButton::RebuildWidget()
{
	EnsureButtonTree();
	return Super::RebuildWidget();
}

void UDubitoShellButton::NativePreConstruct()
{
	Super::NativePreConstruct();

	EnsureButtonTree();
	RefreshButtonText();
	SetIsFocusable(true);
	SetIsInteractionEnabled(true);
	SetIsSelectable(true);
	SetShouldSelectUponReceivingFocus(true);
	SetClickMethod(EButtonClickMethod::DownAndUp);
	SetTouchMethod(EButtonTouchMethod::DownAndUp);
	SetPressMethod(EButtonPressMethod::DownAndUp);
}

void UDubitoShellButton::EnsureButtonTree()
{
	if (!WidgetTree)
	{
		return;
	}

	if (WidgetTree->RootWidget && WidgetTree->RootWidget->GetFName() != ButtonRootName)
	{
		return;
	}

	UBorder* Root = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), ButtonRootName);
	Root->SetBrushColor(ButtonTint);
	Root->SetPadding(FMargin(18.0f, 10.0f));

	UTextBlock* Label = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), ButtonLabelName);
	Label->SetAutoWrapText(true);
	Label->SetJustification(ETextJustify::Center);
	Label->SetColorAndOpacity(FSlateColor(FLinearColor(0.95f, 0.97f, 0.94f, 1.0f)));

	Root->SetContent(Label);
	WidgetTree->RootWidget = Root;
	RefreshButtonText();
}

void UDubitoShellButton::RefreshButtonText() const
{
	if (!WidgetTree)
	{
		return;
	}

	UTextBlock* Label = Cast<UTextBlock>(WidgetTree->FindWidget(ButtonLabelName));
	if (Label)
	{
		Label->SetText(ButtonText);
	}
}
