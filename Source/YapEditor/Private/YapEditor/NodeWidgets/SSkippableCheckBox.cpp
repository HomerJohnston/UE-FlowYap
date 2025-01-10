// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "YapEditor/NodeWidgets/SSkippableCheckBox.h"

#include "Yap/YapProjectSettings.h"
#include "Yap/Enums/YapDialogueSkippable.h"
#include "YapEditor/YapColors.h"
#include "YapEditor/YapEditorStyle.h"

#define LOCTEXT_NAMESPACE "YapEditor"

void SYapSkippableCheckBox::Construct(const FArguments& InArgs)
{
	IsSkippable = InArgs._IsSkippable;
	SkippableSetting = InArgs._SkippableSetting;
	
	ChildSlot
	[
		SNew(SCheckBox)
		.Cursor(EMouseCursor::Default)
		.Style(FYapEditorStyle::Get(), YapStyles.CheckBoxStyle_Skippable)
		.Type(ESlateCheckBoxType::ToggleButton)
		.Padding(FMargin(0, 0))
		.CheckBoxContentUsesAutoWidth(true)
		.ToolTipText(LOCTEXT("SkippableCheckBox_ToolTip", "Toggle whether this can be skipped by the player, hold CTRL while clicking to use default"))
		.IsChecked(this, &SYapSkippableCheckBox::SkippableIsChecked)
		.OnCheckStateChanged(InArgs._OnCheckStateChanged)
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
					.ColorAndOpacity(this, &SYapSkippableCheckBox::ColorAndOpacity_SkipIcon)
					.DesiredSizeOverride(FVector2D(16, 16))
					.Image(FAppStyle::Get().GetBrush("Icons.Rotate180"))
				]
				+ SOverlay::Slot()
				[
					SNew(SImage)
					.ColorAndOpacity(this, &SYapSkippableCheckBox::ColorAndOpacity_NoEntryIcon)
					.DesiredSizeOverride(FVector2D(16, 16))
					.Image(FAppStyle::Get().GetBrush("SourceControl.StatusIcon.Off"))
					.Visibility(this, &SYapSkippableCheckBox::Visibility_NoSkipIcon)
				]
			]
		]
	];
}

FSlateColor SYapSkippableCheckBox::ColorAndOpacity_SkipIcon() const
{
	if (SkippableSetting.Get() == EYapDialogueSkippable::Default)
	{
		return YapColor::DarkGray;
	}

	return IsSkippable.Get() ? YapColor::LightGreen : YapColor::LightYellow;
}

FSlateColor SYapSkippableCheckBox::ColorAndOpacity_NoEntryIcon() const
{
	if (SkippableSetting.Get() == EYapDialogueSkippable::Default)
	{
		return YapColor::DarkGray;
	}
	
	return YapColor::LightYellow * YapColor::LightRed;
}

EVisibility SYapSkippableCheckBox::Visibility_NoSkipIcon() const
{
	return IsSkippable.Get() ? EVisibility::Collapsed : EVisibility::HitTestInvisible;
}

ECheckBoxState SYapSkippableCheckBox::SkippableIsChecked() const
{
	if (SkippableSetting.Get() == EYapDialogueSkippable::Default)
	{
		return ECheckBoxState::Undetermined;
	}

	return IsSkippable.Get() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

#undef LOCTEXT_NAMESPACE
