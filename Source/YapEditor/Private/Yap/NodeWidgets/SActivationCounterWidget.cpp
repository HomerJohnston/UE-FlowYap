#include "Yap/NodeWidgets/SActivationCounterWidget.h"

#include "Yap/YapColors.h"


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
			.Padding(0, 0, 0, (FontHeight - 10))
			[
				SNew(STextBlock)
				.Visibility(this, &SActivationCounterWidget::Visibility_UpperElements)
				.Text(this, &SActivationCounterWidget::NumeratorText)
				.ColorAndOpacity(this, &SActivationCounterWidget::NumeratorColor)
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", FontHeight))
				.Justification(ETextJustify::Center)
				.ToolTipText(INVTEXT("Activation count"))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Fill)
			.Padding(4, 0, 4, 0)
			[
				SNew(SSeparator)
				.Visibility(this, &SActivationCounterWidget::Visibility_UpperElements)
				.Orientation(Orient_Horizontal)
				.Thickness(1)
				.ColorAndOpacity(this, &SActivationCounterWidget::DenominatorColor)
				.SeparatorImage(FAppStyle::GetBrush("WhiteBrush"))
			]
			+ SVerticalBox::Slot()
			.VAlign(VAlign_Top)
			.HAlign(HAlign_Fill)
			.Padding(0, (FontHeight - 11), 0, 0)
			[
				// TODO: SEditableText messes up justification on refresh, why??? STextBlock is ok
				SAssignNew(Denominator, SEditableText)
				.Text(this, &SActivationCounterWidget::DenominatorText)
				.ColorAndOpacity(this, &SActivationCounterWidget::DenominatorColor)
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", FontHeight))
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

EVisibility SActivationCounterWidget::Visibility_UpperElements() const
{
	if (GEditor->PlayWorld)
	{
		return EVisibility::Visible;
	}

	return EVisibility::Collapsed;
}
