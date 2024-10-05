#include "Yap/Helpers/FlowYapWidgetHelper.h"

#include "Yap/FlowYapColors.h"
#include "Yap/FlowYapCondition.h"
#include "Yap/YapEditorStyle.h"



TSharedRef<SWidget> FFlowYapWidgetHelper::CreateConditionWidget(const UFlowYapCondition* Condition)
{
	FString Description = IsValid(Condition) ? Condition->GetDescription() : "<Null Condition>";
	
	return SNew(SBorder)
	.BorderImage(FYapEditorStyle::Get().GetBrush("ImageBrush.Box.SolidWhite.DeburredCorners"))
	.BorderBackgroundColor(YapColor::DarkOrangeRed)
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
