// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license.

#include "YapEditor/Helpers/ProgressionSettingWidget.h"

#define LOCTEXT_NAMESPACE "YapEditor"

TSharedRef<SWidget> PopupContentGetter_ProgressionSettings(TOptional<bool>* bSkippable, TOptional<bool>* bAutoAdvance)
{
	TSharedRef<SWidget> Box = SNew(SVerticalBox)
	+ MakeFragmentProgressionSettingRow(bSkippable, LOCTEXT("SkippableCheckBox_Label", "Skippable"))
	+ MakeFragmentProgressionSettingRow(bAutoAdvance, LOCTEXT("AutoAdvanceCheckBox_Label", "Auto Advance"));

	return Box;
}

// ------------------------------------------------------------------------------------------------

SVerticalBox::FSlot::FSlotArguments MakeFragmentProgressionSettingRow(TOptional<bool>* Setting, FText Label)
{
	SVerticalBox::FSlot::FSlotArguments Slot(SVerticalBox::Slot());
	
	Slot.AutoHeight()
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(4, 0, 4, 0)
		[
			SNew(SCheckBox)
			.IsChecked_Lambda( [Setting] ()
			{
				if (!Setting->IsSet())
				{
					return ECheckBoxState::Undetermined;
				}

				return Setting->GetValue() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
			})
			.OnCheckStateChanged_Lambda( [Setting] (ECheckBoxState InState)
			{
				*Setting = (InState == ECheckBoxState::Checked); 
			})
		]
		+ SHorizontalBox::Slot()
		.FillWidth(1.0)
		.VAlign(VAlign_Center)
		.Padding(4, 0, 4, 0)
		[
			SNew(STextBlock)
			.Text(Label)
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(4, 0, 4, 0)
		[
			SNew(SBox)
			.Visibility_Lambda( [Setting] () { return Setting->IsSet() ? EVisibility::Visible : EVisibility::Hidden; }) 
			.WidthOverride(16)
			.HeightOverride(16)
			[
				SNew(SButton)
				.ButtonStyle(FYapEditorStyle::Get(), YapStyles.ButtonStyle_HoverHintOnly)
				.OnClicked_Lambda( [Setting] () { Setting->Reset(); return FReply::Handled(); } )
				.ContentPadding(2)
				[
					SNew(SImage)
					.Image(FYapEditorStyle::GetImageBrush(YapBrushes.Icon_Reset_Small))
				]
			]
		]
	];

	return Slot;
}

#undef LOCTEXT_NAMESPACE