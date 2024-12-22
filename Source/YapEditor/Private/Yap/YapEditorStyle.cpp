#include "Yap/YapEditorStyle.h"

#include "FindInBlueprints.h"
#include "ILiveCodingModule.h"
#include "ImageUtils.h"
#include "Styling/SlateStyleMacros.h"
#include "Styling/SlateStyleRegistry.h"
#include "Yap/YapColors.h"

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
Set(YAP_QUOTE(STYLENAME), TYPE(TEMPLATE) MODS);\
const TYPE& STYLENAME = *static_cast<const TYPE*>(GetWidgetStyleInternal(YAP_QUOTE(TYPE), YAP_QUOTE(STYLENAME), &TEMPLATE, true))

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
#if 0
	FSlateStyleRegistry::UnRegisterSlateStyle(*this);
	Initialize();
	FSlateStyleRegistry::RegisterSlateStyle(*this);
#endif
}
#endif

void FYapEditorStyle::Initialize()
{
	YAP_DEFINE_FONT(Font_DialogueText,		"Normal",	9);
	YAP_DEFINE_FONT(Font_TitleText,			"Italic",	9);
	YAP_DEFINE_FONT(Font_NodeHeader,		"Bold",		16);
	YAP_DEFINE_FONT(Font_NodeSequencing,	"Italic",	9);
	
	YAP_DEFINE_BRUSH(FSlateImageBrush,	Icon_AudioTime,					"DialogueNodeIcons/AudioTime", ".png",	FVector2f(16, 16));
	YAP_DEFINE_BRUSH(FSlateImageBrush,	Icon_TextTime,					"DialogueNodeIcons/TextTime", ".png",	FVector2f(16, 16));
	YAP_DEFINE_BRUSH(FSlateImageBrush,	Icon_Timer,						"DialogueNodeIcons/Timer", ".png",		FVector2f(16, 16));
	YAP_DEFINE_BRUSH(FSlateImageBrush,	Icon_LocalLimit,				"DialogueNodeIcons/LocalLimit", ".png",	FVector2f(16, 16));
	YAP_DEFINE_BRUSH(FSlateImageBrush,	Icon_Speaker,					"Icon_Audio", ".png",					FVector2f(16, 16));
	YAP_DEFINE_BRUSH(FSlateImageBrush,	Icon_Tag,						"Icon_Tag", ".png",						FVector2f(16, 16));
	YAP_DEFINE_BRUSH(FSlateImageBrush,	Icon_DialogueExpand,			"Icon_DialogueExpand", ".png",			FVector2f(16, 16));
	YAP_DEFINE_BRUSH(FSlateImageBrush,	Icon_Edit,						"Icon_Edit", ".png",					FVector2f(16, 16));
	YAP_DEFINE_BRUSH(FSlateImageBrush,	Icon_MoodKeyMissing,			"Icon_MoodKey_Missing", ".png",			FVector2f(16, 16));
	YAP_DEFINE_BRUSH(FSlateImageBrush,	Icon_Delete,					"Icon_Delete", ".png",					FVector2f(16, 16));
	YAP_DEFINE_BRUSH(FSlateImageBrush,	Icon_UpArrow,					"Icon_UpArrow", ".png",					FVector2f(8, 8));
	YAP_DEFINE_BRUSH(FSlateImageBrush,	Icon_DownArrow,					"Icon_DownArrow", ".png",				FVector2f(8, 8));
	
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
	
	YAP_DEFINE_BRUSH(FSlateBoxBrush,	Outline_White_Deburred,			"Outline_Deburred", ".png",				FMargin(4.0/8.0));
	
	YAP_DEFINE_STYLE(FSliderStyle, SliderStyle_FragmentTimePadding, FSliderStyle::GetDefault(),
		.SetBarThickness(0.f)
		.SetNormalThumbImage(IMAGE_BRUSH("ProgressBar_Fill", CoreStyleConstants::Icon8x8, YapColor::Gray))
		.SetHoveredThumbImage(IMAGE_BRUSH("ProgressBar_Fill", CoreStyleConstants::Icon8x8, YapColor::LightGray))
	);

#define YAP_COMMON_BRUSH "Common/ButtonHoverHint"
//#define YAP_COMMON_BRUSH "Common/Button/simple_sharp_normal"
//#define YAP_COMMON_BRUSH_HOVERED "Common/Button/simple_sharp_hovered"
#define YAP_COMMON_MARGIN FMargin(4.0 / 16.0)
#define YAP_COMMON_PRESSED_PADDING FMargin(0, 1, 0, -1)

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
	
	
	YAP_DEFINE_STYLE(FButtonStyle, ButtonStyle_ConditionWidget, FButtonStyle::GetDefault(),
		.SetNormal(Box_SolidLightGray_Deburred)
		.SetHovered(Box_SolidWhite_Deburred)
		.SetPressed(Box_SolidLightGray_Deburred)	
		.SetNormalPadding(FMargin(0, 0, 0, 0))
		.SetPressedPadding(FMargin(0, 1, 0, -1))
	);
	
	YAP_DEFINE_STYLE(FScrollBarStyle, ScrollBarStyle_DialogueBox, FCoreStyle::Get().GetWidgetStyle<FScrollBarStyle>("ScrollBar"),
		.SetThickness(2)
	);

	YAP_DEFINE_STYLE(FTextBlockStyle, TextBlockStyle_Dialogue, GetParentStyle()->GetWidgetStyle<FTextBlockStyle>("NormalText"),
		.SetFont(Font_DialogueText)
		.SetColorAndOpacity(YapColor::White)
	);

	YAP_DEFINE_STYLE(FTextBlockStyle, TextBlockStyle_TitleText, GetParentStyle()->GetWidgetStyle<FTextBlockStyle>("NormalText"),
		.SetFont(Font_TitleText)
		.SetColorAndOpacity(YapColor::YellowGray)
	);
	
	YAP_DEFINE_STYLE(FTextBlockStyle, TextBlockStyle_NodeHeader, GetParentStyle()->GetWidgetStyle<FTextBlockStyle>("NormalText"),
		.SetFont(Font_NodeHeader)
		.SetColorAndOpacity(YapColor::White)
	);

	YAP_DEFINE_STYLE(FTextBlockStyle, TextBlockStyle_NodeSequencing, GetParentStyle()->GetWidgetStyle<FTextBlockStyle>("NormalText"),
		.SetFont(Font_NodeSequencing)
		.SetColorAndOpacity(YapColor::White)
	);
	
	YAP_DEFINE_STYLE(FEditableTextBoxStyle, EditableTextBoxStyle_Dialogue, FEditableTextBoxStyle::GetDefault(),
		.SetScrollBarStyle(ScrollBarStyle_DialogueBox)
		.SetTextStyle(TextBlockStyle_Dialogue)
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
		.SetPadding(0)
		//.SetBackgroundImageNormal(BOX_BRUSH("Common/WhiteGroupBorder", FMargin(4.0f / 16.0f)))
		//.SetBackgroundImageHovered(BOX_BRUSH("Common/WhiteGroupBorder", FMargin(4.0f / 16.0f)))
		//.SetBackgroundImageFocused(BOX_BRUSH("Common/WhiteGroupBorder", FMargin(4.0f / 16.0f)))
		//.SetBackgroundImageReadOnly(BOX_BRUSH("Common/WhiteGroupBorder", FMargin(4.0f / 16.0f)))
		.SetBackgroundColor(FStyleColors::Recessed)
	);
	
	YAP_DEFINE_STYLE(FProgressBarStyle, ProgressBarStyle_FragmentTimePadding, FProgressBarStyle::GetDefault(),
		.SetBackgroundImage(BOX_BRUSH("ProgressBar_Fill", 2.0f/8.0f, YapColor::Transparent))
		.SetFillImage(BOX_BRUSH("ProgressBar_Fill", 2.0f/8.0f, YapColor::White))
		.SetEnableFillAnimation(false)
	);
}

#undef YAP_QUOTE
#undef YAP_DEFINE_FONT
#undef YAP_DEFINE_BRUSH
#undef YAP_DEFINE_STYLE
