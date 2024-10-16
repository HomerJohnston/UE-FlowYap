#pragma once

#include "FlowYapColors.h"
#include "Styling/SlateStyle.h"
#include "Delegates/IDelegateInstance.h"

// ==============================================

struct FYapBrushes
{
	FName Icon_AudioTime;
	FName Icon_TextTime;
	FName Icon_Timer;
	FName Icon_LocalLimit;
	FName Icon_Speaker;
	FName Icon_Tag;
	FName Icon_DialogueExpand;
	FName Icon_Edit;
	
	FName Border_SharpSquare;
	FName Border_DeburredSquare;
	FName Border_RoundedSquare;
	
	FName Box_SolidWhite;
	FName Box_SolidWhiteDeburred;
	FName Box_SolidWhiteRounded;
};

struct FYapStyles
{	
	FName SliderStyle_FragmentTimePadding;

	FName ButtonStyle_SequencingSelector;
	FName ButtonStyle_ActivationLimit;

	FName ScrollBarStyle_DialogueBox;

	FName TextBlockStyle_Dialogue;
	
	FName EditableTextBoxStyle_DialogueBox;
};

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
