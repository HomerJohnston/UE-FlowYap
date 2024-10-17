#include "Yap/YapEditorStyle.h"

#include "FindInBlueprints.h"
#include "ILiveCodingModule.h"
#include "ImageUtils.h"
#include "Styling/SlateStyleMacros.h"
#include "Styling/SlateStyleRegistry.h"
#include "Yap/FlowYapColors.h"

TArray<TStrongObjectPtr<UTexture2D>> FYapEditorStyle::Textures;

FYapFonts YapFonts;
FYapBrushes YapBrushes;
FYapStyles YapStyles;

#define YAP_QUOTE(X) #X

#define YAP_DEFINE_FONT(NAME, STYLE, SIZE)\
YapFonts.NAME = DEFAULT_FONT(STYLE, SIZE);\
FSlateFontInfo& NAME = YapFonts.NAME;

#define YAP_DEFINE_BRUSH(TYPE, BRUSHNAME, FILENAME, EXTENSION, ARGS)\
YapBrushes.BRUSHNAME = YAP_QUOTE(BRUSHNAME);\
Set(YAP_QUOTE(BRUSHNAME), new TYPE(RootToContentDir(FILENAME, TEXT(EXTENSION)), ARGS));\
const TYPE& BRUSHNAME = *static_cast<const TYPE*>(GetBrush(YAP_QUOTE(BRUSHNAME)));

#define YAP_DEFINE_STYLE(TYPE, STYLENAME, TEMPLATE, MODS)\
YapStyles.STYLENAME = YAP_QUOTE(STYLENAME);\
Set(YAP_QUOTE(STYLENAME), TYPE(TEMPLATE) MODS);\
const TYPE& STYLENAME = *static_cast<const TYPE*>(GetWidgetStyleInternal(YAP_QUOTE(TYPE), YAP_QUOTE(STYLENAME), &TEMPLATE, true))

FYapEditorStyle::FYapEditorStyle()
	: FSlateStyleSet("YapEditor")
{
	SetParentStyleName(FAppStyle::GetAppStyleSetName());
	SetContentRoot(FPaths::ProjectPluginsDir() / TEXT("FlowYap/Resources"));
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
	FSlateStyleRegistry::UnRegisterSlateStyle(*this);
	Initialize();
	FSlateStyleRegistry::RegisterSlateStyle(*this);
}
#endif

void FYapEditorStyle::Initialize()
{
	YAP_DEFINE_FONT(Font_DialogueText,		"Normal",	9);
	YAP_DEFINE_FONT(Font_TitleText,			"Italic",	9);
	YAP_DEFINE_FONT(Font_NodeHeader,		"Bold",		16);
	YAP_DEFINE_FONT(Font_NodeSequencing,	"Italic",	9);
	
	YAP_DEFINE_BRUSH(FSlateImageBrush, Icon_AudioTime,			"DialogueNodeIcons/AudioTime", ".png",	FVector2f(16, 16) );
	YAP_DEFINE_BRUSH(FSlateImageBrush, Icon_TextTime,			"DialogueNodeIcons/TextTime", ".png",	FVector2f(16, 16) );
	YAP_DEFINE_BRUSH(FSlateImageBrush, Icon_Timer,				"DialogueNodeIcons/Timer", ".png",		FVector2f(16, 16) );
	YAP_DEFINE_BRUSH(FSlateImageBrush, Icon_LocalLimit,			"DialogueNodeIcons/LocalLimit", ".png",	FVector2f(16, 16) );
	YAP_DEFINE_BRUSH(FSlateImageBrush, Icon_Speaker,			"Icon_Audio", ".png",					FVector2f(16, 16) );
	YAP_DEFINE_BRUSH(FSlateImageBrush, Icon_Tag,				"Icon_Tag", ".png",						FVector2f(16, 16) );
	YAP_DEFINE_BRUSH(FSlateImageBrush, Icon_DialogueExpand,		"Icon_DialogueExpand", ".png",			FVector2f(16, 16) );
	YAP_DEFINE_BRUSH(FSlateImageBrush, Icon_Edit,				"Icon_Edit", ".png",					FVector2f(16, 16) );

	YAP_DEFINE_BRUSH(FSlateBorderBrush, Border_SharpSquare,		"Border_SharpSquare", ".png",			FMargin(4.0/8.0) );
	YAP_DEFINE_BRUSH(FSlateBorderBrush, Border_DeburredSquare,	"Border_DeburredSquare", ".png",		FMargin(4.0/8.0) );
	YAP_DEFINE_BRUSH(FSlateBorderBrush, Border_RoundedSquare,	"Border_RoundedSquare", ".png",			FMargin(4.0/8.0) );
	
	YAP_DEFINE_STYLE(FSliderStyle, SliderStyle_FragmentTimePadding, FSliderStyle::GetDefault(),
		.SetBarThickness(0.f)
		.SetNormalThumbImage(IMAGE_BRUSH("ProgressBar_Fill", CoreStyleConstants::Icon8x8, YapColor::Gray))
		.SetHoveredThumbImage(IMAGE_BRUSH("ProgressBar_Fill", CoreStyleConstants::Icon8x8, YapColor::LightGray))
	);

	YAP_DEFINE_STYLE(FButtonStyle, ButtonStyle_SequencingSelector, FButtonStyle::GetDefault(),
		.SetNormal(CORE_BOX_BRUSH("Common/ButtonHoverHint", FMargin(4.0 / 16.0), YapColor::DeepGray))
		.SetHovered(CORE_BOX_BRUSH("Common/ButtonHoverHint", FMargin(4.0 / 16.0), YapColor::DarkGray))
		.SetPressed(CORE_BOX_BRUSH("Common/ButtonHoverHint", FMargin(4.0 / 16.0), YapColor::Noir))
		.SetNormalPadding(FMargin(0, 0, 0, 0))
		.SetPressedPadding(FMargin(0, 1, 0, -1))
	);
	
	YAP_DEFINE_STYLE(FButtonStyle, ButtonStyle_ActivationLimit, FButtonStyle::GetDefault(),
		.SetNormal(CORE_BOX_BRUSH("Common/ButtonHoverHint", FMargin(4 / 16.0f), YapColor::DeepGray))
		.SetHovered(CORE_BOX_BRUSH("Common/ButtonHoverHint", FMargin(4 / 16.0f), YapColor::DarkGray))
		.SetPressed(CORE_BOX_BRUSH("Common/ButtonHoverHint", FMargin(4 / 16.0f), YapColor::Noir))
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