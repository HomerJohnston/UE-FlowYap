#include "Yap/YapEditorStyle.h"

#include "FindInBlueprints.h"
#include "ILiveCodingModule.h"
#include "Styling/SlateStyleMacros.h"
#include "Styling/SlateStyleRegistry.h"
#include "Yap/FlowYapColors.h"

//FYapStyles FYapEditorStyle::Styles;

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

void FYapEditorStyle::Initialize()
{
#define YAP_BOX_BRUSH(MEMBER, FILENAME, MARGIN_SIZE, TOTAL_SIZE) Styles().MEMBER = MakeShareable(new BOX_BRUSH("FILENAME", FMargin(MARGIN_SIZE / TOTAL_SIZE)))
#define YAP_IMAGE_BRUSH(MEMBER, FILENAME, BRUSH_SIZE) Styles().MEMBER = MakeShareable(new IMAGE_BRUSH("FILENAME", FVector2f(BRUSH_SIZE)))

	YAP_IMAGE_BRUSH(ImageBrush.Icon.AudioTime,		"DialogueNodeIcons/AudioTime",	16);
	YAP_IMAGE_BRUSH(ImageBrush.Icon.TextTime,		"DialogueNodeIcons/TextTime",	16);
	YAP_IMAGE_BRUSH(ImageBrush.Icon.Timer,			"DialogueNodeIcons/Timer",		16);
	YAP_IMAGE_BRUSH(ImageBrush.Icon.LocalLimit,		"DialogueNodeIcons/LocalLimit",	16);
	YAP_IMAGE_BRUSH(ImageBrush.Icon.Speaker,		"Icon_Audio",					16);
	YAP_IMAGE_BRUSH(ImageBrush.Icon.Tag,			"Icon_Tag",						16);
	YAP_IMAGE_BRUSH(ImageBrush.Icon.DialogueExpand,	"Icon_DialogueExpand",			16);
	YAP_IMAGE_BRUSH(ImageBrush.Icon.Edit,			"Icon_Edit",					16);

	YAP_BOX_BRUSH(ImageBrush.Border.SharpSquare,	"Border_SharpSquare",		4,	8);
	YAP_BOX_BRUSH(ImageBrush.Border.DeburredSquare, "Border_DeburredSquare",	4,	8);
	YAP_BOX_BRUSH(ImageBrush.Border.RoundedSquare,	"Border_RoundedSquare",		4,	8);
	
	Set("SliderStyle.FragmentTimePadding", FSliderStyle(FSliderStyle::GetDefault())
		.SetBarThickness(0.f)
		///.SetNormalBarImage(BOX_BRUSH("ProgressBar_Fill", 2.0f/8.0f, YapColor::Red))
		///.SetHoveredBarImage(BOX_BRUSH("ProgressBar_Fill", 2.0f/8.0f, YapColor::LightRed))
		.SetNormalThumbImage(IMAGE_BRUSH("ProgressBar_Fill", CoreStyleConstants::Icon8x8, YapColor::Gray))
		.SetHoveredThumbImage(IMAGE_BRUSH("ProgressBar_Fill", CoreStyleConstants::Icon8x8, YapColor::LightGray))
		);

	Styles().Button.SequencingSelector = FButtonStyle(FButtonStyle::GetDefault())
		.SetNormal(CORE_BOX_BRUSH("Common/ButtonHoverHint", FMargin(4 / 16.0f), YapColor::DeepGray))
		.SetHovered(CORE_BOX_BRUSH("Common/ButtonHoverHint", FMargin(4 / 16.0f), YapColor::DarkGray))
		.SetPressed(CORE_BOX_BRUSH("Common/ButtonHoverHint", FMargin(4 / 16.0f), YapColor::Noir))
		.SetNormalPadding(FMargin(0, 0, 0, 0))
		.SetPressedPadding(FMargin(0, 1, 0, -1));
	
	Styles().Button.ActivationLimit = FButtonStyle(FButtonStyle::GetDefault())
		.SetNormal(CORE_BOX_BRUSH("Common/ButtonHoverHint", FMargin(4 / 16.0f), YapColor::DeepGray))
		.SetHovered(CORE_BOX_BRUSH("Common/ButtonHoverHint", FMargin(4 / 16.0f), YapColor::DarkGray))
		.SetPressed(CORE_BOX_BRUSH("Common/ButtonHoverHint", FMargin(4 / 16.0f), YapColor::Noir))
		.SetNormalPadding(FMargin(0, 0, 0, 0))
		.SetPressedPadding(FMargin(0, 1, 0, -1));

	const FScrollBarStyle DialogueScrollBarStyle = FScrollBarStyle(FCoreStyle::Get().GetWidgetStyle<FScrollBarStyle>("ScrollBar"))
		.SetThickness(2);

	Set("ScrollBarStyle.Dialogue", DialogueScrollBarStyle);
	
	Set("EditableTextBoxStyle.Dialogue", FEditableTextBoxStyle(FEditableTextBoxStyle::GetDefault())
		.SetBackgroundImageNormal(BOX_BRUSH("Box_SolidWhite_DeburredCorners", FMargin(4 / 8.0f)))
		.SetBackgroundColor(YapColor::DeepGray)
		.SetScrollBarStyle(DialogueScrollBarStyle)
		.SetHScrollBarPadding(0)
		.SetVScrollBarPadding(0));
	
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
