#pragma once

#include "FlowYapColors.h"
#include "Styling/SlateStyle.h"
#include "Delegates/IDelegateInstance.h"

struct FYapIcons
{
	TSharedPtr<FSlateImageBrush> AudioTime;
	TSharedPtr<FSlateImageBrush> TextTime;
	TSharedPtr<FSlateImageBrush> Timer;
	TSharedPtr<FSlateImageBrush> LocalLimit;
	TSharedPtr<FSlateImageBrush> Speaker;
	TSharedPtr<FSlateImageBrush> Tag;
	TSharedPtr<FSlateImageBrush> DialogueExpand;
	TSharedPtr<FSlateImageBrush> Edit;
};

struct FYapBorders
{
	TSharedPtr<FSlateBoxBrush> SharpSquare;
	TSharedPtr<FSlateBoxBrush> DeburredSquare;
	TSharedPtr<FSlateBoxBrush> RoundedSquare;
};

struct FYapBoxes
{
	TSharedPtr<FSlateBoxBrush> SolidWhite;
	TSharedPtr<FSlateBoxBrush> SolidWhiteDeburred;
	TSharedPtr<FSlateBoxBrush> SolidWhiteRounded;
};

struct FYapImageBrushes
{
	FYapIcons Icon;
	FYapBorders Border;
	FYapBoxes Box;
};

struct FYapSliderStyles
{
	FSliderStyle FragmentTimePadding;
};

struct FYapButtonStyles
{
	FButtonStyle SequencingSelector;
	FButtonStyle ActivationLimit;
};

struct FYapStyles
{
	FYapImageBrushes ImageBrush;
	FYapSliderStyles Slider;
	FYapButtonStyles Button;
};

class FYapEditorStyle final : public FSlateStyleSet
{
public:
	FYapStyles Styles_Internal;
	
	static FYapStyles& Styles()
	{
		return Get().Styles_Internal;
	}

	static FYapEditorStyle& Get()
	{
		static FYapEditorStyle Instance;
		return Instance;
	}
	
	FYapEditorStyle();

	virtual ~FYapEditorStyle() override;

	FDelegateHandle OnPatchCompleteHandle;

protected:
	void OnPatchComplete();

	void Initialize();
};
