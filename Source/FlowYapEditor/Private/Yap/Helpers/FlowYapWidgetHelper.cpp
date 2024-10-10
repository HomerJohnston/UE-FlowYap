#include "Yap/Helpers/FlowYapWidgetHelper.h"

#include "Yap/FlowYapColors.h"
#include "Yap/FlowYapCondition.h"
#include "Yap/YapEditorStyle.h"



TSharedRef<SWidget> FFlowYapWidgetHelper::CreateConditionWidget(const UFlowYapCondition* Condition)
{
	FString Description = IsValid(Condition) ? Condition->GetDescription() : "-";
	FLinearColor Color = IsValid(Condition) ? Condition->GetNodeColor() : YapColor::DeepOrangeRed;
	
	return SNew(SBorder)
	.BorderImage(FYapEditorStyle::Get().GetBrush("ImageBrush.Box.SolidWhite.DeburredCorners"))
	.BorderBackgroundColor(Color)
	.VAlign(VAlign_Center)
	.HAlign(HAlign_Center)
	.Padding(4, 0, 4, 0)
	[
		SNew(STextBlock)
			.Text(FText::FromString(Description))
			.ColorAndOpacity(YapColor::White)
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
	];
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
	int32 ActivationLimitVal = ActivationLimit.Get();

	if (ActivationLimitVal > 0 && ActivationCount.Get() >= ActivationLimitVal)
	{
		return YapColor::Red;
	}
	
	return ActivationLimitVal > 0 ? YapColor::LightGray : YapColor::DarkGray;
}

void SActivationCounterWidget::Construct(const FArguments& InArgs)
{
	ActivationCount = InArgs._ActivationCount;
	ActivationLimit = InArgs._ActivationLimit;
	FontHeight = InArgs._FontHeight;

	
	ChildSlot
	[
		SNew(SBox)
		.WidthOverride(20 + 2 * (FontHeight - 8))
		.ToolTipText(INVTEXT("Activation Count / Activation Limit"))
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.VAlign(VAlign_Bottom)
			.Padding(0, 0, 0, (FontHeight - 8))
			[
				SNew(STextBlock)
				.Text(this, &SActivationCounterWidget::NumeratorText)
				.ColorAndOpacity(this, &SActivationCounterWidget::NumeratorColor)
				.Font(FCoreStyle::GetDefaultFontStyle("Normal", FontHeight))
				.Justification(ETextJustify::Center)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
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
			.Padding(0, (FontHeight - 8), 0, 0)
			[
				SNew(STextBlock)
				.Text(this, &SActivationCounterWidget::DenominatorText)
				.ColorAndOpacity(this, &SActivationCounterWidget::DenominatorColor)
				.Font(FCoreStyle::GetDefaultFontStyle("Normal", FontHeight))
				.Justification(ETextJustify::Center)
			]
		]
	];
}
