// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license.

#pragma once

#include "YapEditor/NodeWidgets/SYapButtonPopup.h"
#include "YapEditor/YapEditorStyle.h"

#define LOCTEXT_NAMESPACE "YapEditor"

TSharedRef<SWidget> PopupContentGetter_ProgressionSettings(TOptional<bool>* bSkippable, TOptional<bool>* bAutoAdvance);

SVerticalBox::FSlot::FSlotArguments MakeFragmentProgressionSettingRow(TOptional<bool>* Setting, FText Label);

template<typename TObj, auto TFunc, typename... TFuncArgs>
SOverlay::FOverlaySlot::FSlotArguments MakePopupImage(TOptional<bool>* bSetting, TObj* Obj, FName OffIcon, FName OnIcon, TFuncArgs... Args)
{
	SOverlay::FOverlaySlot::FSlotArguments Slot(SOverlay::Slot());

	Slot
	[
		SNew(SImage)
		.Image_Lambda( [Obj, OnIcon, OffIcon, Args...] ()
		{
			return FYapEditorStyle::GetImageBrush( (Obj->*TFunc)(Args...) ? OnIcon : OffIcon);
		})
		.ColorAndOpacity_Lambda( [Obj, bSetting, Args...] ()
		{
			bool bEvaluatedValue = (Obj->*TFunc)(Args...);
			bool bUsingDefaults = !bSetting->IsSet();

			if (bUsingDefaults)
			{
				return YapColor::DarkGray;
			}
				
			return bEvaluatedValue ? YapColor::LightGreen : YapColor::DarkOrange_Trans;  
		})
	];
	
	return Slot;
}

// ------------------------------------------------------------------------------------------------

template<typename TObj, auto TSkippableFunc, auto TAdvanceFunc, typename... TFuncArgs>
TSharedRef<SWidget> MakeProgressionPopupButton(TOptional<bool>* bSkippable, TOptional<bool>* bAutoAdvance, TObj* Obj, TFuncArgs... FuncArgs)
{
	return SNew(SYapButtonPopup)
	.ButtonStyle(FYapEditorStyle::Get(), YapStyles.ButtonStyle_HoverHintOnly)
	.PopupPlacement(MenuPlacement_RightLeftCenter)
	.ButtonForegroundColor(YapColor::DarkGray_SemiGlass)
	.ButtonContentPadding(0)
	.HAlign(HAlign_Center)
	.VAlign(VAlign_Center)
	.PopupContentGetter(FPopupContentGetter::CreateLambda( [bSkippable, bAutoAdvance] () { return PopupContentGetter_ProgressionSettings(bSkippable, bAutoAdvance); }))
	.ButtonContent()
	[
		SNew(SBox)
		.WidthOverride(16)
		.HeightOverride(16)
		[
			SNew(SOverlay)
			+ MakePopupImage<TObj, TSkippableFunc, TFuncArgs...>(bSkippable, Obj, YapBrushes.Icon_NotSkippable, YapBrushes.Icon_Skippable, FuncArgs...)
			+ MakePopupImage<TObj, TAdvanceFunc, TFuncArgs...>(bAutoAdvance, Obj, YapBrushes.Icon_ManualAdvance, YapBrushes.Icon_AutoAdvance, FuncArgs...)
		]
	];
}

// ------------------------------------------------------------------------------------------------


// ------------------------------------------------------------------------------------------------

#undef LOCTEXT_NAMESPACE