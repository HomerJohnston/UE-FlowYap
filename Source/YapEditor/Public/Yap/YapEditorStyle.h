#pragma once

#include "YapColors.h"
#include "Styling/SlateStyle.h"
#include "Delegates/IDelegateInstance.h"

// ==============================================

struct FYapBrushes
{
	FName Icon_AudioTime;
	FName Icon_DialogueExpand;
	FName Icon_Edit;
	FName Icon_LocalLimit;
	FName Icon_MoodKeyMissing;
	FName Icon_Speaker;
	FName Icon_Tag;
	FName Icon_TextTime;
	FName Icon_Timer;
	FName Icon_Delete;
	FName Icon_UpArrow;
	FName Icon_DownArrow;
	FName Icon_CrossX;
	
	FName Border_DeburredSquare;
	FName Border_RoundedSquare;
	FName Border_Thick_RoundedSquare;
	FName Border_SharpSquare;
	
	FName Box_SolidLightGray;
	FName Box_SolidLightGray_Deburred;
	FName Box_SolidLightGray_Rounded;
	
	FName Box_SolidWhite;
	FName Box_SolidWhite_Deburred;
	FName Box_SolidWhite_Rounded;
	
	FName Box_SolidRed;
	FName Box_SolidRed_Deburred;
	FName Box_SolidRed_Rounded;

	FName Panel_Deburred;
	FName Panel_Rounded;
	FName Panel_Sharp;
	
	FName Outline_White_Deburred;

	FName Pin_OptionalOutput;
};

struct FYapStyles
{	
	FName ButtonStyle_ActivationLimit;
	FName ButtonStyle_ConditionWidget;
	FName ButtonStyle_FragmentControls;
	FName ButtonStyle_HeaderButton;
	FName ButtonStyle_SequencingSelector;

	FName CheckBoxStyle_Skippable;
	
	FName EditableTextBoxStyle_Dialogue;
	FName EditableTextBoxStyle_TitleText;
	
	FName ProgressBarStyle_FragmentTimePadding;
	
	FName SliderStyle_FragmentTimePadding;
	
	FName ScrollBarStyle_DialogueBox;
	
	FName TextBlockStyle_Dialogue;
	FName TextBlockStyle_NodeHeader;
	FName TextBlockStyle_NodeSequencing;
	FName TextBlockStyle_TitleText;
};

struct FYapFonts
{
	FSlateFontInfo Font_DialogueText;
	FSlateFontInfo Font_TitleText;
	FSlateFontInfo Font_NodeHeader;	
	FSlateFontInfo Font_NodeSequencing;	
};

extern FYapFonts YapFonts;
extern FYapBrushes YapBrushes;
extern FYapStyles YapStyles;

class FYapEditorStyle final : public FSlateStyleSet
{
public:
	static TArray<TStrongObjectPtr<UTexture2D>> Textures;
	
	static FYapEditorStyle& Get()
	{
		static FYapEditorStyle Instance;
		return Instance;
	}

	static const FSlateBrush* GetImageBrush(FName BrushName)
	{
		return Get().GetBrush(BrushName);
	}
	
	FYapEditorStyle();
	virtual ~FYapEditorStyle() override;

protected:
	void Initialize();
	void OnPatchComplete();
	FDelegateHandle OnPatchCompleteHandle;
};
