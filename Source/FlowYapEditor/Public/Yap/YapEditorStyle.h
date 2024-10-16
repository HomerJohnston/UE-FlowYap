#pragma once

#include "FlowYapColors.h"
#include "Styling/SlateStyle.h"
#include "Delegates/IDelegateInstance.h"

// ==============================================

struct FYapStyles
{
	FName ImageBrush_Icon_AudioTime;
	FName ImageBrush_Icon_TextTime;
	FName ImageBrush_Icon_Timer;
	FName ImageBrush_Icon_LocalLimit;
	FName ImageBrush_Icon_Speaker;
	FName ImageBrush_Icon_Tag;
	FName ImageBrush_Icon_DialogueExpand;
	FName ImageBrush_Icon_Edit;
	
	FName ImageBrush_Border_SharpSquare;
	FName ImageBrush_Border_DeburredSquare;
	FName ImageBrush_Border_RoundedSquare;
	
	FName ImageBrush_Box_SolidWhite;
	FName ImageBrush_Box_SolidWhiteDeburred;
	FName ImageBrush_Box_SolidWhiteRounded;

	FName SliderStyle_FragmentTimePadding;

	FName ButtonStyle_SequencingSelector;
	FName ButtonStyle_ActivationLimit;

	FName ScrollBarStyle_DialogueBox;

	FName EditableTextBoxStyle_DialogueBox;
};

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
