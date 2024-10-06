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

TSharedRef<SWidget> FFlowYapWidgetHelper::CreateActivationCounterWidget(int A, int B)
{
	FText Denominator = B > 0 ? FText::AsNumber(B) : INVTEXT("\x221E");
	FLinearColor NumeratorColor = A > 0 ? YapColor::LightGray : YapColor::DarkGray;
	FLinearColor DenominatorColor = B > 0 ? YapColor::LightGray : YapColor::DarkGray;
	
	return SNew(SVerticalBox)
	+ SVerticalBox::Slot()
	.VAlign(VAlign_Bottom)
	.Padding(0, 0, 0, -1)
	[
		SNew(STextBlock)
		.Text(FText::AsNumber(A))
		.ColorAndOpacity(NumeratorColor)
		.Font(FCoreStyle::GetDefaultFontStyle("Normal", 8))
		.Justification(ETextJustify::Center)
	]
	+ SVerticalBox::Slot()
	.AutoHeight()
	.Padding(4, 0, 4, 0)
	[
		SNew(SSeparator)
		.Orientation(Orient_Horizontal)
		.Thickness(1)
		.ColorAndOpacity(YapColor::DarkGray)
		.SeparatorImage(FAppStyle::GetBrush("WhiteBrush"))
	]
	+ SVerticalBox::Slot()
	.VAlign(VAlign_Top)
	.Padding(0, -1, 0, 0)
	[
		SNew(STextBlock)
		.Text(Denominator)
		.ColorAndOpacity(DenominatorColor)
		.Font(FCoreStyle::GetDefaultFontStyle("Normal", 8))
		.Justification(ETextJustify::Center)
	];
}
