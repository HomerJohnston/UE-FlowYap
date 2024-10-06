#include "Yap/YapEditorStyle.h"

#include "FindInBlueprints.h"
#include "ILiveCodingModule.h"
#include "Styling/SlateStyleMacros.h"
#include "Styling/SlateStyleRegistry.h"
#include "Yap/FlowYapColors.h"

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
	const FVector2f Icon16(16.f);
	const FVector2f Icon20(20.f);
	const FVector2f Icon22(22.f);
	const FVector2f Icon25(25.f);
	const FVector2f Icon32(32.f);
	const FVector2f Icon40(40.f);
	
	Set("ImageBrush.Icon.AudioTime",		new IMAGE_BRUSH("DialogueNodeIcons/AudioTime",	Icon16));
	Set("ImageBrush.Icon.TextTime",			new IMAGE_BRUSH("DialogueNodeIcons/TextTime",	Icon16));
	Set("ImageBrush.Icon.Timer",			new IMAGE_BRUSH("DialogueNodeIcons/Timer",		Icon16));
	Set("ImageBrush.Icon.LocalLimit",		new IMAGE_BRUSH("DialogueNodeIcons/LocalLimit",	Icon16));
	Set("ImageBrush.Icon.Audio",			new IMAGE_BRUSH("Icon_Audio",	Icon16));
	Set("ImageBrush.Icon.Tag",				new IMAGE_BRUSH("Icon_Tag",	Icon16));

	Set("ImageBrush.Brush.DiagonalLine",	new IMAGE_BRUSH("Brush_Diagonal_16px", Icon16));

	Set("ImageBrush.Border.SharpSquare",				new BOX_BRUSH("Border_SharpSquare", FMargin(4 / 8.0f)));
	Set("ImageBrush.Border.DeburredSquare",				new BOX_BRUSH("Border_DeburredSquare", FMargin(4 / 8.0f)));
	Set("ImageBrush.Border.RoundedSquare",				new BOX_BRUSH("Border_RoundedSquare", FMargin(4 / 8.0f)));
	Set("ImageBrush.Box.SolidWhite",					new BOX_BRUSH("Box_SolidWhite", FMargin(4 / 8.0f)));
	Set("ImageBrush.Box.SolidWhite.DeburredCorners",	new BOX_BRUSH("Box_SolidWhite_DeburredCorners", FMargin(4 / 8.0f)));
	Set("ImageBrush.Box.SolidWhite.RoundedCorners",		new BOX_BRUSH("Box_SolidWhite_RoundedCorners", FMargin(4 / 8.0f)));

	
	Set("SliderStyle.FragmentTimePadding", FSliderStyle(FSliderStyle::GetDefault())
		.SetBarThickness(0.f)
		///.SetNormalBarImage(BOX_BRUSH("ProgressBar_Fill", 2.0f/8.0f, YapColor::Red))
		///.SetHoveredBarImage(BOX_BRUSH("ProgressBar_Fill", 2.0f/8.0f, YapColor::LightRed))
		.SetNormalThumbImage(IMAGE_BRUSH("ProgressBar_Fill", CoreStyleConstants::Icon8x8, YapColor::Gray))
		.SetHoveredThumbImage(IMAGE_BRUSH("ProgressBar_Fill", CoreStyleConstants::Icon8x8, YapColor::LightGray))
		);

	Set("ButtonStyle.ActivationLimit", FButtonStyle(FButtonStyle::GetDefault())
		.SetNormal(CORE_BOX_BRUSH("Common/ButtonHoverHint", FMargin(4 / 16.0f), YapColor::DeepGray_SemiTrans))
		.SetHovered(CORE_BOX_BRUSH("Common/ButtonHoverHint", FMargin(4 / 16.0f), YapColor::DarkGray_Trans))
		.SetPressed(CORE_BOX_BRUSH("Common/ButtonHoverHint", FMargin(4 / 16.0f), YapColor::DimGray_Trans))
		.SetNormalPadding(0)
		.SetPressedPadding(0));
		
	Set("ProgressBarStyle.FragmentTimePadding", FProgressBarStyle()
	.SetBackgroundImage(BOX_BRUSH("ProgressBar_Fill", 2.0f/8.0f, YapColor::Transparent))
	.SetFillImage(BOX_BRUSH("ProgressBar_Fill", 2.0f/8.0f, YapColor::White))
	.SetEnableFillAnimation(false));

	const ISlateStyle* ParentStyle = GetParentStyle();
	
	NormalText = ParentStyle->GetWidgetStyle<FTextBlockStyle>("NormalText");

	Set("Text.TitleText", FTextBlockStyle(NormalText)
		.SetFont(DEFAULT_FONT("Italic", 10))
		.SetColorAndOpacity(YapColor::Gray)
	);
	
	Set("Text.DialogueText", FTextBlockStyle(NormalText)
		.SetFont(DEFAULT_FONT("Regular", 10))
		.SetColorAndOpacity(YapColor::White)
	);
	
	Set("Text.NodeHeader", FTextBlockStyle(NormalText)
		.SetFont(DEFAULT_FONT("Bold", 16))
		.SetColorAndOpacity(YapColor::White)
	);
}
