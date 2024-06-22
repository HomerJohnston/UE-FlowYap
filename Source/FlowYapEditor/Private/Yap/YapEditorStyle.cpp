#include "Yap/YapEditorStyle.h"

#include "FindInBlueprints.h"
#include "ILiveCodingModule.h"
#include "Styling/SlateStyleMacros.h"
#include "Styling/SlateStyleRegistry.h"
#include "Yap/FlowYapColors.h"

FYapEditorStyle::FYapEditorStyle()
	: FSlateStyleSet("YapEditor")
{
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
	/*
	FSlateStyleRegistry::UnRegisterSlateStyle(*this);
	Initialize();

	FSlateStyleRegistry::RegisterSlateStyle(*this);
	*/
}

void FYapEditorStyle::Initialize()
{
	const FVector2f Icon16(16.f);
	const FVector2f Icon20(20.f);
	const FVector2f Icon22(22.f);
	const FVector2f Icon25(25.f);
	const FVector2f Icon32(32.f);
	const FVector2f Icon40(40.f);
	
	Set("ImageBrush.Icon.AudioTime",	new IMAGE_BRUSH("DialogueNodeIcons/AudioTime",	Icon16));
	Set("ImageBrush.Icon.TextTime",		new IMAGE_BRUSH("DialogueNodeIcons/TextTime",	Icon16));
	Set("ImageBrush.Icon.Timer",		new IMAGE_BRUSH("DialogueNodeIcons/Timer",		Icon16));

	Set("SliderStyle.FragmentTimePadding", FSliderStyle(FSliderStyle::GetDefault())
		.SetBarThickness(0.f)
		///.SetNormalBarImage(BOX_BRUSH("ProgressBar_Fill", 2.0f/8.0f, YapColor::Red))
		///.SetHoveredBarImage(BOX_BRUSH("ProgressBar_Fill", 2.0f/8.0f, YapColor::LightRed))
		.SetNormalThumbImage(IMAGE_BRUSH("ProgressBar_Fill", CoreStyleConstants::Icon8x8, YapColor::Gray))
		.SetHoveredThumbImage(IMAGE_BRUSH("ProgressBar_Fill", CoreStyleConstants::Icon8x8, YapColor::LightGray))
		);

	Set("ProgressBarStyle.FragmentTimePadding", FProgressBarStyle()
	.SetBackgroundImage(BOX_BRUSH("ProgressBar_Fill", 2.0f/8.0f, YapColor::Transparent))
	.SetFillImage(BOX_BRUSH("ProgressBar_Fill", 2.0f/8.0f, YapColor::White))
	.SetEnableFillAnimation(false));
}
