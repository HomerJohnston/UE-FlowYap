#pragma once
#include "Yap/FlowYapColors.h"
#include "Yap/YapEditorStyle.h"

class UFlowYapCondition;

class FFlowYapWidgetHelper
{
public:
	
	static TSharedRef<SWidget> CreateConditionWidget(const UFlowYapCondition* Condition);

	template <class T>
	static TSharedRef<SWidget> CreateTagPreviewWidget(T* Object, FText (T::*wtf)() const)
	{
		return SNew(SBorder)
		.BorderImage(FYapEditorStyle::Get().GetBrush("ImageBrush.Box.SolidWhite.DeburredCorners"))
		.BorderBackgroundColor(YapColor::DeepGray_SemiTrans)
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Center)
		.Padding(4, 0, 4, 0)
		[
			SNew(STextBlock)
			.Text(Object, wtf)
			.IsEnabled(false)
		];
	}
};
