// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

// This file is formatted to be viewable on tabs, 4-space tabs. If you use another setting, sorry!

#include "YapEditor/YapEditorStyle.h"

#include "FindInBlueprints.h"
#include "ILiveCodingModule.h"
#include "ImageUtils.h"
#include "Styling/SlateStyleMacros.h"
#include "Styling/SlateStyleRegistry.h"
#include "YapEditor/YapColors.h"

TArray<TStrongObjectPtr<UTexture2D>> FYapEditorStyle::Textures;

FYapFonts YapFonts;
FYapBrushes YapBrushes;
FYapStyles YapStyles;

#define YAP_QUOTE(X) #X

#define YAP_DEFINE_FONT(NAME, STYLE, SIZE)\
YapFonts.NAME = DEFAULT_FONT(STYLE, SIZE);\
FSlateFontInfo& NAME = YapFonts.NAME

#define YAP_DEFINE_BRUSH(TYPE, BRUSHNAME, FILENAME, EXTENSION, ...)\
YapBrushes.BRUSHNAME = YAP_QUOTE(BRUSHNAME);\
Set(YAP_QUOTE(BRUSHNAME), new TYPE(RootToContentDir(FILENAME, TEXT(EXTENSION)), __VA_ARGS__));\
const TYPE& BRUSHNAME = *static_cast<const TYPE*>(GetBrush(YAP_QUOTE(BRUSHNAME)))

#define YAP_COPY_BRUSH(TYPE, BRUSHNAME, TEMPLATE, ...)\
YapBrushes.BRUSHNAME = YAP_QUOTE(BRUSHNAME);\
Set(YAP_QUOTE(BRUSHNAME), const_cast<FSlateBrush*>(TEMPLATE));\
const TYPE& BRUSHNAME = *static_cast<const TYPE*>(GetBrush(YAP_QUOTE(BRUSHNAME)))

#define YAP_DEFINE_STYLE(TYPE, STYLENAME, TEMPLATE, MODS)\
YapStyles.STYLENAME = YAP_QUOTE(STYLENAME);\
Set(YAP_QUOTE(STYLENAME), TYPE(TEMPLATE));\
TYPE& STYLENAME = const_cast<TYPE&>(GetWidgetStyle<TYPE>(YAP_QUOTE(STYLENAME)));\
STYLENAME MODS;

/*
#define YAP_SET_BRUSH(TYPE, BRUSH, MARGIN, COLOR)\ 
.Set##TYPE(FSlateBrush(BRUSH), FMargin(MARGIN), COLOR)
YAP_SET_BRUSH(Normal, ButtonHoverHintBrush, (4.0 / 16.0), YapColor::White);
*/

/*	
#define YAP_SET_FOREGROUND(TYPE, COLOR)\
	.Set##TYPE##Foreground(COLOR)
*/

/*
#define YAP_SETUP_BUTTON_PARAMS(BRUSH, MARGIN, NORMAL_COLOR, HOVERED_COLOR, PRESSED_COLOR, DISABLED_COLOR, FOREGROUND_NORMAL_COLOR, FOREGROUND_HOVERED_COLOR, FOREGROUND_PRESSED_COLOR, FOREGROUND_DISABLED_COLOR)\
YAP_SET_BRUSH(Normal, BRUSH, FMargin(MARGIN), NORMAL_COLOR))\
YAP_SET_BRUSH(Hovered(BRUSH, FMargin(MARGIN), HOVERED_COLOR))\
YAP_SET_BRUSH(Pressed(BRUSH, FMargin(MARGIN), PRESSED_COLOR))\
YAP_SET_BRUSH(Disabled(BRUSH, FMargin(MARGIN), DISABLED_COLOR))\
YAP_SET_FOREGROUND(Normal, FOREGROUND_NORMAL_COLOR)\
YAP_SET_FOREGROUND(Hovered, FOREGROUND_HOVERED_COLOR)\
YAP_SET_FOREGROUND(Pressed, FOREGROUND_PRESSED_COLOR)\
YAP_SET_FOREGROUND(Disabled, FOREGROUND_DISABLED_COLOR)
*/
	
#define LOCTEXT_NAMESPACE "YapEditor"

FYapEditorStyle::FYapEditorStyle()
	: FSlateStyleSet("YapEditor")
{
	SetParentStyleName(FAppStyle::GetAppStyleSetName());
	SetContentRoot(FPaths::ProjectPluginsDir() / TEXT("Yap/Resources"));
	SetCoreContentRoot(FPaths::EngineContentDir() / TEXT("Editor/Slate"));

	Initialize();
	
	FSlateStyleRegistry::RegisterSlateStyle(*this);
	
#if WITH_LIVE_CODING
	if (ILiveCodingModule* LiveCoding = FModuleManager::LoadModulePtr<ILiveCodingModule>(LIVE_CODING_MODULE_NAME))
	{
		OnPatchCompleteHandle = LiveCoding->GetOnPatchCompleteDelegate().AddRaw(this, &FYapEditorStyle::OnPatchComplete);
	}
#endif
}

FYapEditorStyle::~FYapEditorStyle()
{
	Textures.Empty();
	
#if WITH_LIVE_CODING
	if (ILiveCodingModule* LiveCoding = FModuleManager::GetModulePtr<ILiveCodingModule>(LIVE_CODING_MODULE_NAME))
	{
		LiveCoding->GetOnPatchCompleteDelegate().Remove(OnPatchCompleteHandle);
	}
#endif
	
	FSlateStyleRegistry::UnRegisterSlateStyle(*this);
}

#if WITH_LIVE_CODING
void FYapEditorStyle::OnPatchComplete()
{
#if 1
	FSlateStyleRegistry::UnRegisterSlateStyle(*this);
	Initialize();
	FSlateStyleRegistry::RegisterSlateStyle(*this);
#endif
}
#endif

#define YAP_COMMON_BRUSH "Common/ButtonHoverHint"

//#define YAP_COMMON_BRUSH "Common/Button/simple_sharp_normal"
//#define YAP_COMMON_BRUSH_HOVERED "Common/Button/simple_sharp_hovered"
#define YAP_COMMON_MARGIN FMargin(4.0 / 16.0)
#define YAP_COMMON_PRESSED_PADDING FMargin(0, 1, 0, -1)
#define YAP_COMMON_CHECKBOXSTYLE FAppStyle::Get().GetWidgetStyle<FCheckBoxStyle>("ToggleButtonCheckBox")

void FYapEditorStyle::Initialize()
{
	const FSlateBrush& FilledCircle = *FAppStyle::GetBrush("Icons.FilledCircle");
	const FSlateBrush& NoBorder = *FAppStyle::GetBrush("NoBorder");
	
	// ============================================================================================
	// FONTS
	// ============================================================================================
	YAP_DEFINE_FONT(Font_DialogueText,		"Normal",	10); // TODO font settings in project editor settings
	YAP_DEFINE_FONT(Font_TitleText,			"Italic",	10);
	YAP_DEFINE_FONT(Font_NodeHeader,		"Bold",		15);
	YAP_DEFINE_FONT(Font_NodeSequencing,	"Italic",	9);
	
	// ============================================================================================
	// BRUSHES
	// ============================================================================================
	YAP_DEFINE_BRUSH(FSlateImageBrush,	Icon_AudioTime,					"DialogueNodeIcons/AudioTime", ".png",	FVector2f(16, 16));
	YAP_DEFINE_BRUSH(FSlateImageBrush,	Icon_Baby,						"Icon_Baby", ".png",					FVector2f(16, 16));
	YAP_DEFINE_BRUSH(FSlateImageBrush,	Icon_Delete,					"Icon_Delete", ".png",					FVector2f(16, 16));
	YAP_DEFINE_BRUSH(FSlateImageBrush,	Icon_DialogueExpand,			"Icon_DialogueExpand", ".png",			FVector2f(16, 16));
	YAP_DEFINE_BRUSH(FSlateImageBrush,	Icon_DownArrow,					"Icon_DownArrow", ".png",				FVector2f(8, 8));
	YAP_DEFINE_BRUSH(FSlateImageBrush,	Icon_TextTime,					"DialogueNodeIcons/TextTime", ".png",	FVector2f(16, 16));
	YAP_DEFINE_BRUSH(FSlateImageBrush,	Icon_Timer,						"DialogueNodeIcons/Timer", ".png",		FVector2f(16, 16));
	YAP_DEFINE_BRUSH(FSlateImageBrush,	Icon_LocalLimit,				"DialogueNodeIcons/LocalLimit", ".png",	FVector2f(16, 16));
	YAP_DEFINE_BRUSH(FSlateImageBrush,	Icon_Speaker,					"Icon_Audio", ".png",					FVector2f(16, 16));
	YAP_DEFINE_BRUSH(FSlateImageBrush,	Icon_Tag,						"Icon_Tag", ".png",						FVector2f(16, 16));
	YAP_DEFINE_BRUSH(FSlateImageBrush,	Icon_Edit,						"Icon_Edit", ".png",					FVector2f(16, 16));
	YAP_DEFINE_BRUSH(FSlateImageBrush,	Icon_MoodKeyMissing,			"Icon_MoodKey_Missing", ".png",			FVector2f(16, 16));
	YAP_DEFINE_BRUSH(FSlateImageBrush,	Icon_UpArrow,					"Icon_UpArrow", ".png",					FVector2f(8, 8));
	YAP_DEFINE_BRUSH(FSlateImageBrush,	Icon_SettingsExpander,			"Icon_SettingsExpander_12px", ".png",	FVector2f(12, 12));
	YAP_DEFINE_BRUSH(FSlateImageBrush,	Icon_Circle_22px,				"Icon_Circle_22px", ".png",				FVector2f(22, 22));
	
	YAP_DEFINE_BRUSH(FSlateBorderBrush, Border_SharpSquare,				"Border_Sharp", ".png",					FMargin(4.0/8.0));
	YAP_DEFINE_BRUSH(FSlateBorderBrush, Border_DeburredSquare,			"Border_Deburred", ".png",				FMargin(4.0/8.0));
	YAP_DEFINE_BRUSH(FSlateBorderBrush, Border_RoundedSquare,			"Border_Rounded", ".png",				FMargin(4.0/8.0));
	
	YAP_DEFINE_BRUSH(FSlateBorderBrush, Border_Thick_RoundedSquare,		"Border_Thick_Rounded", ".png",			FMargin(8.0/16.0));
	
	YAP_DEFINE_BRUSH(FSlateBoxBrush,	Panel_Sharp,					"Panel_Sharp", ".png",					FMargin(4.0/8.0));
	YAP_DEFINE_BRUSH(FSlateBoxBrush,	Panel_Deburred,					"Panel_Deburred", ".png",				FMargin(4.0/8.0));
	YAP_DEFINE_BRUSH(FSlateBoxBrush,	Panel_Rounded,					"Panel_Rounded", ".png",				FMargin(4.0/8.0));
	
	YAP_DEFINE_BRUSH(FSlateBoxBrush,	Box_SolidWhite,					"Box_SolidWhite", ".png",				FMargin(4.0/8.0));
	YAP_DEFINE_BRUSH(FSlateBoxBrush,	Box_SolidWhite_Deburred,		"Box_SolidWhite_Deburred", ".png",		FMargin(4.0/8.0));
	YAP_DEFINE_BRUSH(FSlateBoxBrush,	Box_SolidWhite_Rounded,			"Box_SolidWhite_Rounded", ".png",		FMargin(4.0/8.0));
	
	YAP_DEFINE_BRUSH(FSlateBoxBrush,	Box_SolidLightGray,				"Box_SolidWhite", ".png",				FMargin(4.0/8.0), YapColor::LightGray);
	YAP_DEFINE_BRUSH(FSlateBoxBrush,	Box_SolidLightGray_Deburred,	"Box_SolidWhite_Deburred", ".png",		FMargin(4.0/8.0), YapColor::LightGray);
	YAP_DEFINE_BRUSH(FSlateBoxBrush,	Box_SolidLightGray_Rounded,		"Box_SolidWhite_Rounded", ".png",		FMargin(4.0/8.0), YapColor::LightGray);
	
	YAP_DEFINE_BRUSH(FSlateBoxBrush,	Box_SolidRed,					"Box_SolidWhite", ".png",				FMargin(4.0/8.0), YapColor::Red);
	YAP_DEFINE_BRUSH(FSlateBoxBrush,	Box_SolidRed_Deburred,			"Box_SolidWhite_Deburred", ".png",		FMargin(4.0/8.0), YapColor::Red);
	YAP_DEFINE_BRUSH(FSlateBoxBrush,	Box_SolidRed_Rounded,			"Box_SolidWhite_Rounded", ".png",		FMargin(4.0/8.0), YapColor::Red);
	
	YAP_DEFINE_BRUSH(FSlateBoxBrush,	Box_SolidNoir,					"Box_SolidWhite", ".png",				FMargin(4.0/8.0), YapColor::Noir);
	YAP_DEFINE_BRUSH(FSlateBoxBrush,	Box_SolidNoir_Deburred,			"Box_SolidWhite_Deburred", ".png",		FMargin(4.0/8.0), YapColor::Noir);
	YAP_DEFINE_BRUSH(FSlateBoxBrush,	Box_SolidNoir_Rounded,			"Box_SolidWhite_Rounded", ".png",		FMargin(4.0/8.0), YapColor::Noir);
	
	YAP_DEFINE_BRUSH(FSlateBoxBrush,	Box_SolidBlack,					"Box_SolidWhite", ".png",				FMargin(4.0/8.0), YapColor::Black);
	YAP_DEFINE_BRUSH(FSlateBoxBrush,	Box_SolidBlack_Deburred,		"Box_SolidWhite_Deburred", ".png",		FMargin(4.0/8.0), YapColor::Black);
	YAP_DEFINE_BRUSH(FSlateBoxBrush,	Box_SolidBlack_Rounded,			"Box_SolidWhite_Rounded", ".png",		FMargin(4.0/8.0), YapColor::Black);
	
	YAP_DEFINE_BRUSH(FSlateBoxBrush,	Outline_White_Deburred,			"Outline_Deburred", ".png",				FMargin(4.0/8.0));

	YAP_DEFINE_BRUSH(FSlateImageBrush,	CheckBox_Test,					"Box_SolidWhite_Deburred", ".png",				FVector2f(8, 8), YapColor::Error);

	// ============================================================================================
	// SLIDER STYLES
	// ============================================================================================
	
	YAP_DEFINE_STYLE(FSliderStyle, SliderStyle_FragmentTimePadding, FSliderStyle::GetDefault(),
		.SetBarThickness(0.f)
		.SetNormalThumbImage(IMAGE_BRUSH("ProgressBar_Fill", CoreStyleConstants::Icon8x8, YapColor::Gray))
		.SetHoveredThumbImage(IMAGE_BRUSH("ProgressBar_Fill", CoreStyleConstants::Icon8x8, YapColor::LightGray))
	);
	
	// ============================================================================================
	// BUTTON STYLES
	// ============================================================================================

	YAP_DEFINE_STYLE(FButtonStyle, ButtonStyle_HeaderButton, FButtonStyle::GetDefault(),
		.SetNormal(CORE_BOX_BRUSH(YAP_COMMON_BRUSH, YAP_COMMON_MARGIN, YapColor::Gray))
		.SetHovered(CORE_BOX_BRUSH(YAP_COMMON_BRUSH, YAP_COMMON_MARGIN, YapColor::White))
		.SetPressed(CORE_BOX_BRUSH(YAP_COMMON_BRUSH, YAP_COMMON_MARGIN, YapColor::DarkGray))
		.SetNormalForeground(YapColor::DimWhite)
		.SetHoveredForeground(YapColor::White)
		.SetPressedForeground(YapColor::LightGray)
		.SetPressedPadding(YAP_COMMON_PRESSED_PADDING)
	);
	
	YAP_DEFINE_STYLE(FButtonStyle, ButtonStyle_SequencingSelector, FButtonStyle::GetDefault(),
		.SetNormal(CORE_BOX_BRUSH(YAP_COMMON_BRUSH, YAP_COMMON_MARGIN, YapColor::DeepGray))
		.SetHovered(CORE_BOX_BRUSH(YAP_COMMON_BRUSH, YAP_COMMON_MARGIN, YapColor::DarkGray))
		.SetPressed(CORE_BOX_BRUSH(YAP_COMMON_BRUSH, YAP_COMMON_MARGIN, YapColor::Noir))
		.SetNormalForeground(YapColor::White_Glass)
		.SetHoveredForeground(YapColor::White)
		.SetPressedForeground(YapColor::LightGray)
		.SetPressedPadding(YAP_COMMON_PRESSED_PADDING)
	);
	
	YAP_DEFINE_STYLE(FButtonStyle, ButtonStyle_ActivationLimit, FButtonStyle::GetDefault(),
		.SetNormal(CORE_BOX_BRUSH(YAP_COMMON_BRUSH, YAP_COMMON_MARGIN, YapColor::DeepGray))
		.SetHovered(CORE_BOX_BRUSH(YAP_COMMON_BRUSH, YAP_COMMON_MARGIN, YapColor::DarkGray))
		.SetPressed(CORE_BOX_BRUSH(YAP_COMMON_BRUSH, YAP_COMMON_MARGIN, YapColor::Noir))
		.SetNormalForeground(YapColor::White_Glass)
		.SetHoveredForeground(YapColor::White)
		.SetPressedForeground(YapColor::LightGray)
		.SetPressedPadding(YAP_COMMON_PRESSED_PADDING)
	);

	YAP_DEFINE_STYLE(FButtonStyle, ButtonStyle_FragmentControls, FButtonStyle::GetDefault(),
		.SetNormal(CORE_BOX_BRUSH(YAP_COMMON_BRUSH, YAP_COMMON_MARGIN, YapColor::DeepGray_Glass))
		.SetHovered(CORE_BOX_BRUSH(YAP_COMMON_BRUSH, YAP_COMMON_MARGIN, YapColor::DarkGray))
		.SetPressed(CORE_BOX_BRUSH(YAP_COMMON_BRUSH, YAP_COMMON_MARGIN, YapColor::Noir))
		.SetNormalForeground(YapColor::White_Glass)
		.SetHoveredForeground(YapColor::White)
		.SetPressedForeground(YapColor::LightGray)
		.SetPressedPadding(YAP_COMMON_PRESSED_PADDING)
	);

	YAP_DEFINE_STYLE(FButtonStyle, ButtonStyle_DialogueSettings, FButtonStyle::GetDefault(),
		.SetNormal(CORE_BOX_BRUSH("Icons.Toolbar.Settings", YAP_COMMON_MARGIN, YapColor::DarkGray))
	);

	YAP_DEFINE_STYLE(FButtonStyle, ButtonStyle_DialogueCornerFoldout, FButtonStyle::GetDefault(),
		.SetNormal(CORE_BOX_BRUSH(YAP_COMMON_BRUSH, YAP_COMMON_MARGIN, YapColor::Transparent))
		.SetHovered(CORE_BOX_BRUSH(YAP_COMMON_BRUSH, YAP_COMMON_MARGIN, YapColor::Transparent))
		.SetPressed(CORE_BOX_BRUSH(YAP_COMMON_BRUSH, YAP_COMMON_MARGIN, YapColor::Transparent))
		.SetNormalForeground(YapColor::Gray_SemiGlass)
		.SetHoveredForeground(YapColor::Gray_SemiTrans)
		.SetPressedForeground(YapColor::DarkGray)
		.SetPressedPadding(FMargin(0))
		);

	YAP_DEFINE_STYLE(FButtonStyle, ButtonStyle_TimeSetting, FButtonStyle::GetDefault(),
		.SetNormal(CORE_BOX_BRUSH(YAP_COMMON_BRUSH, YAP_COMMON_MARGIN, YapColor::LightGray))
		.SetHovered(CORE_BOX_BRUSH(YAP_COMMON_BRUSH, YAP_COMMON_MARGIN, YapColor::White))
		.SetPressed(CORE_BOX_BRUSH(YAP_COMMON_BRUSH, YAP_COMMON_MARGIN, YapColor::Gray))
		.SetNormalForeground(YapColor::DimWhite)
		.SetHoveredForeground(YapColor::White)
		.SetPressedForeground(YapColor::LightGray)
		.SetPressedPadding(YAP_COMMON_PRESSED_PADDING)
	);

	YAP_DEFINE_STYLE(FButtonStyle, ButtonStyle_TimeSettingOpener, FButtonStyle::GetDefault(),
		.SetNormal(NoBorder)
		.SetHovered(NoBorder)
		.SetPressed(NoBorder)
		.SetDisabled(NoBorder)
		.SetNormalForeground(YapColor::DimWhite)
		.SetHoveredForeground(YapColor::White)
		.SetPressedForeground(YapColor::LightGray)
		.SetPressedPadding(FMargin(0))
	);

	YAP_DEFINE_STYLE(FButtonStyle, ButtonStyle_ConditionWidget, FButtonStyle::GetDefault(),
		.SetNormal(Box_SolidLightGray_Deburred)
		.SetHovered(Box_SolidWhite_Deburred)
		.SetPressed(Box_SolidLightGray_Deburred)	
		.SetNormalPadding(FMargin(0, 0, 0, 0))
		.SetPressedPadding(FMargin(0, 1, 0, -1))
	);
	
	YAP_DEFINE_STYLE(FButtonStyle, ButtonStyle_AudioPreview, FButtonStyle::GetDefault(),
		.SetNormal(CORE_BOX_BRUSH(YAP_COMMON_BRUSH, YAP_COMMON_MARGIN, YapColor::Transparent))
		.SetHovered(CORE_BOX_BRUSH(YAP_COMMON_BRUSH, YAP_COMMON_MARGIN, YapColor::Transparent))
		.SetPressed(CORE_BOX_BRUSH(YAP_COMMON_BRUSH, YAP_COMMON_MARGIN, YapColor::Transparent))	
		.SetNormalPadding(FMargin(0, 0, 0, 0))
		.SetPressedPadding(FMargin(0, 1, 0, -1))
		.SetNormalForeground(YapColor::Gray_SemiGlass)
		.SetHoveredForeground(YapColor::Gray_SemiTrans)
		.SetPressedForeground(YapColor::DarkGray)
	);
	
	// ============================================================================================
	// CHECKBOX STYLES
	// ============================================================================================
	
	YAP_DEFINE_STYLE(FCheckBoxStyle, CheckBoxStyle_Skippable, YAP_COMMON_CHECKBOXSTYLE,
		.SetCheckBoxType(ESlateCheckBoxType::ToggleButton)
		.SetCheckedImage(YAP_COMMON_CHECKBOXSTYLE.UncheckedImage)
		.SetCheckedHoveredImage(YAP_COMMON_CHECKBOXSTYLE.UncheckedHoveredImage)
		.SetCheckedPressedImage(YAP_COMMON_CHECKBOXSTYLE.UncheckedPressedImage)
		.SetUndeterminedImage(YAP_COMMON_CHECKBOXSTYLE.UncheckedImage)
		.SetUndeterminedHoveredImage(YAP_COMMON_CHECKBOXSTYLE.UncheckedHoveredImage)
		.SetUndeterminedPressedImage(YAP_COMMON_CHECKBOXSTYLE.UncheckedPressedImage)
	);
		
	// ============================================================================================
	// SCROLLBAR STYLES
	// ============================================================================================
	
	YAP_DEFINE_STYLE(FScrollBarStyle, ScrollBarStyle_DialogueBox, FCoreStyle::Get().GetWidgetStyle<FScrollBarStyle>("ScrollBar"),
		.SetThickness(2)
		.SetHorizontalBackgroundImage(Box_SolidBlack)
		.SetHorizontalBottomSlotImage(Box_SolidWhite)
		.SetDraggedThumbImage(Box_SolidWhite)
		.SetHoveredThumbImage(Box_SolidWhite)
		.SetNormalThumbImage(Box_SolidLightGray)
	);
		
	// ============================================================================================
	// TEXT BLOCK STYLES
	// ============================================================================================
	
	YAP_DEFINE_STYLE(FTextBlockStyle, TextBlockStyle_DialogueText, GetParentStyle()->GetWidgetStyle<FTextBlockStyle>("NormalText"),
		.SetFont(Font_DialogueText)
		.SetColorAndOpacity(FSlateColor::UseForeground())
	);

	YAP_DEFINE_STYLE(FTextBlockStyle, TextBlockStyle_TitleText, GetParentStyle()->GetWidgetStyle<FTextBlockStyle>("NormalText"),
		.SetFont(Font_TitleText)
		.SetColorAndOpacity(FSlateColor::UseForeground())
	);
	
	YAP_DEFINE_STYLE(FTextBlockStyle, TextBlockStyle_NodeHeader, GetParentStyle()->GetWidgetStyle<FTextBlockStyle>("NormalText"),
		.SetFont(Font_NodeHeader)
		.SetColorAndOpacity(FSlateColor::UseForeground())
	);

	YAP_DEFINE_STYLE(FTextBlockStyle, TextBlockStyle_NodeSequencing, GetParentStyle()->GetWidgetStyle<FTextBlockStyle>("NormalText"),
		.SetFont(Font_NodeSequencing)
		.SetColorAndOpacity(FSlateColor::UseForeground())
	);
	
	// ============================================================================================
	// EDITABLE TEXT BLOCK STYLES
	// ============================================================================================
	
	YAP_DEFINE_STYLE(FEditableTextBoxStyle, EditableTextBoxStyle_Dialogue, FEditableTextBoxStyle::GetDefault(),
		.SetScrollBarStyle(ScrollBarStyle_DialogueBox) // This doesn't do dick, thanks Epic
		.SetTextStyle(TextBlockStyle_DialogueText)
		.SetFont(Font_DialogueText)
		.SetForegroundColor(FSlateColor::UseForeground())
		.SetPadding(0)
		.SetBackgroundImageNormal(CORE_BOX_BRUSH("Common/WhiteGroupBorder", FMargin(4.0f / 16.0f)))
		.SetBackgroundImageHovered(CORE_BOX_BRUSH("Common/WhiteGroupBorder", FMargin(4.0f / 16.0f)))
		.SetBackgroundImageFocused(CORE_BOX_BRUSH("Common/WhiteGroupBorder", FMargin(4.0f / 16.0f)))
		.SetBackgroundImageReadOnly(CORE_BOX_BRUSH("Common/WhiteGroupBorder", FMargin(4.0f / 16.0f)))
		.SetBackgroundColor(FStyleColors::Recessed)
	);

	YAP_DEFINE_STYLE(FEditableTextBoxStyle, EditableTextBoxStyle_TitleText, FEditableTextBoxStyle::GetDefault(),
		.SetScrollBarStyle(ScrollBarStyle_DialogueBox)
		.SetTextStyle(TextBlockStyle_TitleText)
		.SetFont(Font_TitleText)
		.SetForegroundColor(FSlateColor::UseForeground())
		.SetPadding(0)
		//.SetBackgroundImageNormal(BOX_BRUSH("Common/WhiteGroupBorder", FMargin(4.0f / 16.0f)))
		//.SetBackgroundImageHovered(BOX_BRUSH("Common/WhiteGroupBorder", FMargin(4.0f / 16.0f)))
		//.SetBackgroundImageFocused(BOX_BRUSH("Common/WhiteGroupBorder", FMargin(4.0f / 16.0f)))
		//.SetBackgroundImageReadOnly(BOX_BRUSH("Common/WhiteGroupBorder", FMargin(4.0f / 16.0f)))
		.SetBackgroundColor(FStyleColors::Recessed)
	);
		
	// ============================================================================================
	// PROGRESS BAR STYLES
	// ============================================================================================
	
	YAP_DEFINE_STYLE(FProgressBarStyle, ProgressBarStyle_FragmentTimePadding, FProgressBarStyle::GetDefault(),
		.SetBackgroundImage(BOX_BRUSH("ProgressBar_Fill", 2.0f/8.0f, YapColor::Transparent))
		.SetFillImage(BOX_BRUSH("ProgressBar_Fill", 2.0f/8.0f, YapColor::White))
		.SetEnableFillAnimation(false)
	);
}

#undef LOCTEXT_NAMESPACE