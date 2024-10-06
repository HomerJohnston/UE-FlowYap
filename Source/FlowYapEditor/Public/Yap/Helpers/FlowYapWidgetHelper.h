#pragma once
#include "Yap/FlowYapColors.h"
#include "Yap/YapEditorStyle.h"

class UFlowYapCondition;

class FFlowYapWidgetHelper
{
public:
	
	static TSharedRef<SWidget> CreateConditionWidget(const UFlowYapCondition* Condition);

	template <class T>
	static TSharedRef<SWidget> CreateTagPreviewWidget(T* Object, FText (T::*TextDelegate)() const, EVisibility (T::*VisibilityDelegate)() const)
	{
		return SNew(SBorder)
		.BorderImage(FYapEditorStyle::Get().GetBrush("ImageBrush.Box.SolidWhite.DeburredCorners"))
		.BorderBackgroundColor(YapColor::Transparent)
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Center)
		.Padding(4, 0, 4, 0)
		.Visibility(Object, VisibilityDelegate)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(-2, -1, 2, 0)
			[
				SNew(SImage)
				.DesiredSizeOverride(FVector2D(16, 16))
				.Image(FYapEditorStyle::Get().GetBrush("ImageBrush.Icon.Tag"))
				.ColorAndOpacity(YapColor::Gray_SemiTrans)
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(2, 0, 0, 0)
			[
				SNew(STextBlock)
				.Text(Object, TextDelegate)
				.ColorAndOpacity(YapColor::DimGray)
				.Font(FCoreStyle::GetDefaultFontStyle("Normal", 10))
			]
		];
	}

	static TSharedRef<SWidget> CreateActivationCounterWidget(int A, int B);
};
