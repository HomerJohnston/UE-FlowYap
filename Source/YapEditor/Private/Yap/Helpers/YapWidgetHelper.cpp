#include "Yap/Helpers/YapWidgetHelper.h"

#include "SGraphPanel.h"
#include "Graph/FlowGraphEditor.h"
#include "Graph/FlowGraphUtils.h"
#include "Yap/YapColors.h"
#include "Yap/YapCondition.h"
#include "Yap/YapEditorStyle.h"
#include "Yap/Nodes/FlowNode_YapDialogue.h"
#include "Yap/SlateWidgets/SGameplayTagComboFiltered.h"


TSharedRef<SWidget> FYapWidgetHelper::CreateConditionWidget(UFlowNode_YapDialogue* Dialogue, UYapCondition* Condition, TSharedPtr<SBox> VirtualWindowBox)
{
	FString Description = IsValid(Condition) ? Condition->GetDescription() : "-";

	FLinearColor ButtonColor = IsValid(Condition) ? Condition->GetNodeColor() : YapColor::DeepOrangeRed;
	FLinearColor TextColor = ButtonColor.GetLuminance() < 0.6 ? YapColor::White : YapColor::Black;

	TWeakObjectPtr<UFlowNode_YapDialogue> DialogueWeakPtr = Dialogue;
	
	return SNew(SButton)
	.ButtonColorAndOpacity(ButtonColor)
	.ContentPadding(FMargin(4, 0, 4, 0))
	.ForegroundColor(TextColor)
	.ButtonStyle(FYapEditorStyle::Get(), YapStyles.ButtonStyle_ConditionWidget)
	.ToolTipText(INVTEXT("Prerequisite for this to run."))
	.OnClicked(FOnClicked::CreateLambda([DialogueWeakPtr, Condition, VirtualWindowBox]()
	{
		if (!DialogueWeakPtr.IsValid())
		{
			return  FReply::Handled();
		}
		
		if (!IsValid(Condition))
		{
			return FReply::Handled();
		}
		
		if (!VirtualWindowBox.IsValid())
		{
			return FReply::Handled();
		}
		
		FDetailsViewArgs Args;
		Args.bHideSelectionTip = true;
		Args.bLockable = false;
		Args.bShowOptions = false;
		Args.DefaultsOnlyVisibility = EEditDefaultsOnlyNodeVisibility::Show;
	
		FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
		TSharedPtr<IDetailsView> DetailsWidget = PropertyEditorModule.CreateDetailView(Args);
		DetailsWidget->SetObject(Condition, true);

		VirtualWindowBox->GetParentWidget()->SetVisibility(EVisibility::Visible);
		VirtualWindowBox->SetContent(DetailsWidget.ToSharedRef());
		VirtualWindowBox->SetVisibility(EVisibility::Visible);
		
		TSharedPtr<SFlowGraphEditor> GraphEditor = FFlowGraphUtils::GetFlowGraphEditor(DialogueWeakPtr->GetGraphNode()->GetGraph());
		if (GraphEditor)
		{
			GraphEditor->SelectSingleNode(DialogueWeakPtr->GetGraphNode());
		}

		return FReply::Handled();
	}))
	[
		SNew(STextBlock)
		.Text(FText::FromString(Description))
		.ColorAndOpacity(FSlateColor::UseForeground())
		.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
	];
}

TSharedRef<SWidget> FYapWidgetHelper::CreateTagPreviewWidget(TAttribute<FText> Text,
	TAttribute<EVisibility> Visibility)
{
	return SNew(SBorder)
		.BorderImage(FYapEditorStyle::GetImageBrush(YapBrushes.Box_SolidWhite_Deburred))
		.BorderBackgroundColor(YapColor::Transparent)
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Center)
		.Padding(4, 0, 4, 0)
		.Visibility(Visibility)
		.ToolTipText(INVTEXT("Gameplay tag for this element"))
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(2, 0, 0, 0)
		[
			SNew(STextBlock)
				.Text(Text)
				.ColorAndOpacity(YapColor::DimGray)
				.Font(FCoreStyle::GetDefaultFontStyle("Normal", 10))
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(-2, -1, 2, 0)
		[
			SNew(SImage)
				.DesiredSizeOverride(FVector2D(16, 16))
				.Image(FYapEditorStyle::GetImageBrush(YapBrushes.Icon_Tag))
				.ColorAndOpacity(YapColor::Gray_SemiTrans)
		]
	];
}

TSharedRef<SWidget> FYapWidgetHelper::CreateFilteredTagWidget(TAttribute<FGameplayTag> Tag, FString FilterString, TDelegate<void(const FGameplayTag)> OnTagChanged, FText ToolTipText)
{
	return SNew(SGameplayTagComboFiltered)
	.Tag(Tag)
	.Filter(FilterString)
	.OnTagChanged(OnTagChanged)
	.ToolTipText(ToolTipText);
}

FText SActivationCounterWidget::NumeratorText() const
{
	int32 Value = ActivationCount.Get();
	return FText::AsNumber(Value);
}

FText SActivationCounterWidget::DenominatorText() const
{
	int32 Value = ActivationLimit.Get();
	return Value > 0 ? FText::AsNumber(Value) : INVTEXT("\x221E");
}

FSlateColor SActivationCounterWidget::NumeratorColor() const
{
	int32 ActivationCountVal = ActivationCount.Get();

	if (ActivationCountVal == 0)
	{
		return YapColor::DarkGray;
	}
	
	int32 ActivationLimitVal = ActivationLimit.Get();

	if (ActivationLimitVal > 0)
	{
		return ActivationCountVal >= ActivationLimitVal ? YapColor::Red : YapColor::LightGray;
	}
	
	return ActivationCountVal > 0 ? YapColor::LightGray : YapColor::DarkGray;
}

FSlateColor SActivationCounterWidget::DenominatorColor() const
{
	if (Denominator->HasKeyboardFocus())
	{
		return YapColor::White;
	}
	
	int32 ActivationLimitVal = ActivationLimit.Get();

	if (ActivationLimitVal > 0 && ActivationCount.Get() >= ActivationLimitVal)
	{
		return YapColor::Red;
	}
	
	return ActivationLimitVal > 0 ? YapColor::LightGray : YapColor::DarkGray;
}

void SActivationCounterWidget::Construct(const FArguments& InArgs, FOnTextCommitted OnTextCommitted)
{
	ActivationCount = InArgs._ActivationCount;
	ActivationLimit = InArgs._ActivationLimit;
	FontHeight = InArgs._FontHeight;

	TArray<TCHAR> Numbers { '0', '1', '2', '3',	'4', '5', '6', '7', '8', '9' };

	ChildSlot
	[
		SNew(SBox)
		.WidthOverride(20 + 2 * (FontHeight - 8))
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.VAlign(VAlign_Bottom)
			.HAlign(HAlign_Fill)
			.Padding(0, 0, 0, (FontHeight - 8))
			[
				SNew(STextBlock)
				.Text(this, &SActivationCounterWidget::NumeratorText)
				.ColorAndOpacity(this, &SActivationCounterWidget::NumeratorColor)
				.Font(FCoreStyle::GetDefaultFontStyle("Normal", FontHeight))
				.Justification(ETextJustify::Center)
				.ToolTipText(INVTEXT("Activation count"))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Fill)
			.Padding(4, 0, 4, 0)
			[
				SNew(SSeparator)
				.Orientation(Orient_Horizontal)
				.Thickness(1)
				.ColorAndOpacity(this, &SActivationCounterWidget::DenominatorColor)
				.SeparatorImage(FAppStyle::GetBrush("WhiteBrush"))
			]
			+ SVerticalBox::Slot()
			.VAlign(VAlign_Top)
			.HAlign(HAlign_Fill)
			.Padding(0, (FontHeight - 8), 0, 0)
			[
				// TODO: SEditableText messes up justification on refresh, why??? STextBlock is ok
				SAssignNew(Denominator, SEditableText)
				.Text(this, &SActivationCounterWidget::DenominatorText)
				.ColorAndOpacity(this, &SActivationCounterWidget::DenominatorColor)
				.Font(FCoreStyle::GetDefaultFontStyle("Normal", FontHeight))
				.Justification(ETextJustify::Center)
				.OnTextCommitted(OnTextCommitted)
				.OnIsTypedCharValid(FOnIsTypedCharValid::CreateLambda([Numbers](const TCHAR InCh) { return Numbers.Contains(InCh); }))
				.ToolTipText(INVTEXT("Activation limit, use 0 for infinite"))
				.SelectAllTextWhenFocused(true)
				.TextShapingMethod(ETextShapingMethod::KerningOnly)
				.TextFlowDirection(ETextFlowDirection::LeftToRight)
			]
		]
	];
}
