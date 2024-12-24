#include "Yap/NodeWidgets/SSkippableCheckBox.h"

#include "Yap/YapEditorStyle.h"

TSharedRef<SCheckBox> CreateSkippableCheckbox
(
	const TAttribute<ECheckBoxState>& IsCheckedAtt,
	const FOnCheckStateChanged& OnCheckStateChangedEvt,
	const TAttribute<FSlateColor>& ColorAndOpacityAtt,
	const TAttribute<EVisibility>& VisibilityOffIconAtt
)
{
	return SNew(SCheckBox)
	.Cursor(EMouseCursor::Default)
	.Style(FYapEditorStyle::Get(), YapStyles.CheckBoxStyle_Skippable)
	.Type(ESlateCheckBoxType::ToggleButton)
	.Padding(FMargin(0, 0))
	.CheckBoxContentUsesAutoWidth(true)
	.ToolTipText(INVTEXT("Toggle whether this can be skipped by the player. Hold CTRL while clicking to use default."))
	.IsChecked(IsCheckedAtt)
	.OnCheckStateChanged(OnCheckStateChangedEvt)
	.Content()
	[
		SNew(SBox)
		.WidthOverride(20)
		.HeightOverride(20)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			[
				SNew(SImage)
				.ColorAndOpacity(ColorAndOpacityAtt)
				.DesiredSizeOverride(FVector2D(16, 16))
				.Image(FAppStyle::Get().GetBrush("Icons.Rotate180"))
			]
			+ SOverlay::Slot()
			[
				SNew(SImage)
				.ColorAndOpacity(ColorAndOpacityAtt)
				.DesiredSizeOverride(FVector2D(16, 16))
				.Image(FAppStyle::Get().GetBrush("SourceControl.StatusIcon.Off"))
				.Visibility(VisibilityOffIconAtt)
			]
		]
	];
}
