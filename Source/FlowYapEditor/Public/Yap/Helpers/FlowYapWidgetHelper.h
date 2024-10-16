#pragma once
#include "Widgets/SUserWidget.h"
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
		.BorderImage(FYapEditorStyle::GetImageBrush(YapBrushes.Box_SolidWhiteDeburred))
		.BorderBackgroundColor(YapColor::Transparent)
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Center)
		.Padding(4, 0, 4, 0)
		.Visibility(Object, VisibilityDelegate)
		.ToolTipText(INVTEXT("Gameplay tag for this element"))
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(-2, -1, 2, 0)
			[
				SNew(SImage)
				.DesiredSizeOverride(FVector2D(16, 16))
				.Image(FYapEditorStyle::GetImageBrush(YapBrushes.Icon_Tag))
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
};

class SActivationCounterWidget : public SCompoundWidget
{
	public:
	SLATE_USER_ARGS( SActivationCounterWidget )
		: _FontHeight(8) {}

	SLATE_ATTRIBUTE(int32, ActivationCount)
	SLATE_ATTRIBUTE(int32, ActivationLimit)
	SLATE_ARGUMENT(int32, FontHeight)
	
	SLATE_END_ARGS()

	TAttribute<int32> ActivationCount;
	TAttribute<int32> ActivationLimit;
	int32 FontHeight;
	
	FText NumeratorText() const;
	FText DenominatorText() const;

	FSlateColor NumeratorColor() const;
	FSlateColor DenominatorColor() const;

	TSharedPtr<SEditableText> Denominator;
	
	// MUST Provide this function for SNew to call!
	virtual void Construct(const FArguments& InArgs, FOnTextCommitted OnTextCommitted);

	void TestTest(const FText&, ETextCommit::Type) { UE_LOG(LogTemp, Warning, TEXT("Yes it worked")); };
};
