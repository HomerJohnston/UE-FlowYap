#include "Yap/YapEditorStyle.h"

#include "FindInBlueprints.h"
#include "ILiveCodingModule.h"
#include "ImageUtils.h"
#include "Styling/SlateStyleMacros.h"
#include "Styling/SlateStyleRegistry.h"
#include "Yap/FlowYapColors.h"

TArray<TStrongObjectPtr<UTexture2D>> FYapEditorStyle::Textures;

FYapStyles YapStyles;

#define YAP_QUOTE(X) #X
	
#define YAP_DEFINE_STYLE(TYPE, MEMBER, TEMPLATE, MODS)\
YapStyles.MEMBER = YAP_QUOTE(MEMBER);\
Set(YAP_QUOTE(MEMBER), TYPE(TEMPLATE) MODS);\
const TYPE& MEMBER = *static_cast<const TYPE*>(GetWidgetStyleInternal(YAP_QUOTE(TYPE), YAP_QUOTE(MEMBER), &TEMPLATE, true))

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

void FYapEditorStyle::OnPatchComplete()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*this);
	Initialize();
	FSlateStyleRegistry::RegisterSlateStyle(*this);
}

FString SanitizeStyleName(FString InString)
{
	return InString.Replace(TEXT("."), TEXT("_"));
}

#define YAP_DEFINE_IMAGEBRUSH(MEMBER, FILENAME, BRUSH_SIZE)\
YapStyles.MEMBER = YAP_QUOTE(MEMBER);\
Set(YAP_QUOTE(MEMBER), new IMAGE_BRUSH(FILENAME, FVector2f BRUSH_SIZE ));\
const FSlateBrush& MEMBER = *GetBrush(YAP_QUOTE(MEMBER))
	
#define YAP_DEFINE_BOXBRUSH(MEMBER, FILENAME, MARGIN_SLASH_TOTAL)\
YapStyles.MEMBER = YAP_QUOTE(MEMBER);\
Set(YAP_QUOTE(MEMBER), new BOX_BRUSH(FILENAME, FMargin MARGIN_SLASH_TOTAL ))

#define YAP_DEFINE_BRUSH(TYPE, MEMBER, FILENAME, EXTENSION, ARGS)\
YapStyles.MEMBER = YAP_QUOTE(MEMBER);\
Set(YAP_QUOTE(MEMBER), new TYPE(RootToContentDir(FILENAME, TEXT(EXTENSION)), ARGS));\
const TYPE& MEMBER = *static_cast<const TYPE*>(GetBrush(YAP_QUOTE(MEMBER)));

void FYapEditorStyle::Initialize()
{
	YAP_DEFINE_IMAGEBRUSH(ImageBrush_Icon_TextTime,			"DialogueNodeIcons/TextTime",	(16, 16));
	YAP_DEFINE_IMAGEBRUSH(ImageBrush_Icon_Timer,			"DialogueNodeIcons/Timer",		(16, 16));
	YAP_DEFINE_IMAGEBRUSH(ImageBrush_Icon_LocalLimit,		"DialogueNodeIcons/LocalLimit",	(16, 16));
	YAP_DEFINE_IMAGEBRUSH(ImageBrush_Icon_Speaker,			"Icon_Audio",					(16, 16));
	YAP_DEFINE_IMAGEBRUSH(ImageBrush_Icon_Tag,				"Icon_Tag",						(16, 16));
	YAP_DEFINE_IMAGEBRUSH(ImageBrush_Icon_DialogueExpand,	"Icon_DialogueExpand",			(16, 16));
	YAP_DEFINE_IMAGEBRUSH(ImageBrush_Icon_Edit,				"Icon_Edit",					(16, 16));

	YAP_DEFINE_BOXBRUSH(ImageBrush_Border_SharpSquare,		"Border_SharpSquare",			(4/8));
	YAP_DEFINE_BOXBRUSH(ImageBrush_Border_DeburredSquare,	"Border_DeburredSquare",		(4/8));
	YAP_DEFINE_BOXBRUSH(ImageBrush_Border_RoundedSquare,	"Border_RoundedSquare",			(4/8));

	YAP_DEFINE_BRUSH(FSlateImageBrush, ImageBrush_Icon_AudioTime, "DialogueNodeIcons/AudioTime", ".png", FVector2f(16, 16));
	YAP_DEFINE_BRUSH(FSlateBorderBrush, ImageBrush_Border_RoundedSquare, "Border_RoundedSquare", ".png", FMargin(4/8));
	
	YAP_DEFINE_STYLE(FSliderStyle, SliderStyle_FragmentTimePadding, FSliderStyle::GetDefault(),
		.SetBarThickness(0.f)
		.SetNormalThumbImage(IMAGE_BRUSH("ProgressBar_Fill", CoreStyleConstants::Icon8x8, YapColor::Gray))
		.SetHoveredThumbImage(IMAGE_BRUSH("ProgressBar_Fill", CoreStyleConstants::Icon8x8, YapColor::LightGray))
	);

	YAP_DEFINE_STYLE(FButtonStyle, ButtonStyle_SequencingSelector, FButtonStyle::GetDefault(),
		.SetNormal(CORE_BOX_BRUSH("Common/ButtonHoverHint", FMargin(4 / 16.0f), YapColor::DeepGray))
		.SetHovered(CORE_BOX_BRUSH("Common/ButtonHoverHint", FMargin(4 / 16.0f), YapColor::DarkGray))
		.SetPressed(CORE_BOX_BRUSH("Common/ButtonHoverHint", FMargin(4 / 16.0f), YapColor::Noir))
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
		.SetThickness(2));

	YAP_DEFINE_STYLE(FEditableTextBoxStyle, EditableTextBoxStyle_DialogueBox, FEditableTextBoxStyle::GetDefault(),
		.SetScrollBarStyle(ScrollBarStyle_DialogueBox));

	
	
	Set("ProgressBarStyle.FragmentTimePadding", FProgressBarStyle()
	.SetBackgroundImage(BOX_BRUSH("ProgressBar_Fill", 2.0f/8.0f, YapColor::Transparent))
	.SetFillImage(BOX_BRUSH("ProgressBar_Fill", 2.0f/8.0f, YapColor::White))
	.SetEnableFillAnimation(false));
	
	const ISlateStyle* ParentStyle = GetParentStyle();
	
	FTextBlockStyle NormalTextStyle = ParentStyle->GetWidgetStyle<FTextBlockStyle>("NormalText");

	FTextBlockStyle DialogueTextStyle = NormalTextStyle
		.SetColorAndOpacity(YapColor::White);
	
	FTextBlockStyle TitleTextStyle = NormalTextStyle
		.SetColorAndOpacity(YapColor::YellowGray)
		.SetFont(DEFAULT_FONT("Italic", 9));

	Set("TextStyle.TextBlock.Dialogue", DialogueTextStyle);
	Set("TextStyle.TextBlock.TitleText", TitleTextStyle);
	
	Set("Text.NodeHeader", FTextBlockStyle(NormalTextStyle)
		.SetFont(DEFAULT_FONT("Bold", 16))
		.SetColorAndOpacity(YapColor::White));
		
	Set("Text.NodeSequencing", FTextBlockStyle(NormalTextStyle)
		.SetFont(DEFAULT_FONT("Italic", 9))
		.SetColorAndOpacity(YapColor::White));
	
	const FEditableTextBoxStyle& NormalEditableTextBoxStyle = FAppStyle::GetWidgetStyle<FEditableTextBoxStyle>("NormalEditableTextBox");
	
	Set("EditableTextBox.Dialogue", FEditableTextBoxStyle(NormalEditableTextBoxStyle)
		.SetTextStyle(DialogueTextStyle)
		.SetBackgroundImageNormal(BOX_BRUSH("Common/WhiteGroupBorder", FMargin(4.0f / 16.0f)))
		.SetBackgroundImageHovered(BOX_BRUSH("Common/WhiteGroupBorder", FMargin(4.0f / 16.0f)))
		.SetBackgroundImageFocused(BOX_BRUSH("Common/WhiteGroupBorder", FMargin(4.0f / 16.0f)))
		.SetBackgroundImageReadOnly(BOX_BRUSH("Common/WhiteGroupBorder", FMargin(4.0f / 16.0f)))
		.SetBackgroundColor(FStyleColors::Recessed)
	);
	
	Set("EditableTextBox.TitleText", FEditableTextBoxStyle(NormalEditableTextBoxStyle)
		.SetTextStyle(TitleTextStyle)
		.SetBackgroundImageNormal(BOX_BRUSH("Common/WhiteGroupBorder", FMargin(4.0f / 16.0f)))
		.SetBackgroundImageHovered(BOX_BRUSH("Common/WhiteGroupBorder", FMargin(4.0f / 16.0f)))
		.SetBackgroundImageFocused(BOX_BRUSH("Common/WhiteGroupBorder", FMargin(4.0f / 16.0f)))
		.SetBackgroundImageReadOnly(BOX_BRUSH("Common/WhiteGroupBorder", FMargin(4.0f / 16.0f)))
		.SetBackgroundColor(FStyleColors::Recessed)
	);
}
