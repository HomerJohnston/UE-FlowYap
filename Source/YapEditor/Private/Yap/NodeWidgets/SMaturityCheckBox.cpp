#include "Yap/NodeWidgets/SMaturityCheckBox.h"
#include "Yap/YapEditorStyle.h"

namespace YapEditor
{
	TSharedRef<SCheckBox> CreateMaturityCheckBox
	(
		const TAttribute<ECheckBoxState>& IsCheckedAtt,
		const FOnCheckStateChanged& OnCheckStateChangedEvt,
		const TAttribute<FSlateColor>& ColorAndOpacityAtt,
		const TAttribute<EVisibility>& ActiveIndicatorAtt
	)
	{
		return SNew(SCheckBox)
		.Cursor(EMouseCursor::Default)
		.Style(FYapEditorStyle::Get(), YapStyles.CheckBoxStyle_Skippable)
		.Type(ESlateCheckBoxType::ToggleButton)
		.Padding(FMargin(0, 0))
		.CheckBoxContentUsesAutoWidth(true)
		.ToolTipText(INVTEXT("Child-safe settings."))
		.IsChecked(IsCheckedAtt)
		.OnCheckStateChanged(OnCheckStateChangedEvt)
			//.BackgroundImage(FYapEditorStyle::GetImageBrush(YapBrushes.Box_SolidRed))
			//.BorderBackgroundColor(YapColor::Error)
		.Content()
		[
			SNew(SBox)
			.WidthOverride(20)
			.HeightOverride(20)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(SImage)
				.ColorAndOpacity(ColorAndOpacityAtt)
				.DesiredSizeOverride(FVector2D(16, 16))
				.Image(FYapEditorStyle::GetImageBrush(YapBrushes.Icon_Baby))
			]
		];
	}
}