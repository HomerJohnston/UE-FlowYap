// Copyright Ghost Pepper Games, Inc. All Rights Reserved.

#include "FlowYapEditorSubsystem.h"

#include "FlowYapColors.h"
#include "FlowYap/FlowYapProjectSettings.h"
#include "ImageUtils.h"
#include "FlowYap/FlowYapEngineUtils.h"
#include "FlowYapInputTracker.h"

#define LOCTEXT_NAMESPACE "FlowYap"

FCheckBoxStyles UFlowYapEditorSubsystem::CheckBoxStyles;

void UFlowYapEditorSubsystem::UpdateMoodKeyIconsMap()
{
	const UFlowYapProjectSettings* ProjectSettings = UFlowYapProjectSettings::Get();

	const TArray<FName>& MoodKeys = ProjectSettings->GetMoodKeys();

	MoodKeyIconTextures.Empty(MoodKeys.Num());

	for (const FName& MoodKey : MoodKeys)
	{
		if (MoodKey.IsNone())
		{
			continue;
		}
		
		FString IconPath = ProjectSettings->GetPortraitIconPath(MoodKey);
		UTexture2D* MoodKeyIcon = FImageUtils::ImportFileAsTexture2D(IconPath);

		if (!IsValid(MoodKeyIcon))
		{
			continue;
		}

		MoodKeyIconTextures.Add(MoodKey, MoodKeyIcon);

		TSharedPtr<FSlateBrush> MoodKeyBrush = MakeShareable(new FSlateBrush);

		MoodKeyBrush->SetResourceObject(MoodKeyIcon);
		MoodKeyBrush->SetImageSize({16, 16});
		
		MoodKeyIconBrushes.Add(MoodKey, MoodKeyBrush);
	}
}

UTexture2D* UFlowYapEditorSubsystem::GetMoodKeyIcon(FName MoodKey)
{
	UTexture2D** Texture = MoodKeyIconTextures.Find(MoodKey);

	if (Texture)
	{
		return *Texture;
	}

	return nullptr;
}

const FSlateBrush* UFlowYapEditorSubsystem::GetMoodKeyBrush(FName Name)
{
	TSharedPtr<FSlateBrush>* Brush = MoodKeyIconBrushes.Find(Name);

	return Brush ? Brush->Get() : nullptr;
}

#define INITALIZE_CHECKBOX_STYLE(Name, Color) CheckBoxStyles.Name = FAppStyle::Get().GetWidgetStyle<FCheckBoxStyle>("ToggleButtonCheckBox");\
	CheckBoxStyles.Name##.CheckedImage.TintColor = FlowYapColor::##Color##;\
	CheckBoxStyles.Name##.CheckedHoveredImage.TintColor = FlowYapColor::##Color##Hovered;\
	CheckBoxStyles.Name##.CheckedPressedImage.TintColor = FlowYapColor::##Color##Pressed\

void UFlowYapEditorSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UFlowYapProjectSettings* ProjectSettings = GetMutableDefault<UFlowYapProjectSettings>();
	ProjectSettings->OnMoodKeysChanged.AddUObject(this, &ThisClass::UpdateMoodKeyIconsMap);

	UpdateMoodKeyIconsMap();

	LoadIcon("Resources/DialogueNodeIcons/Icon_Timer_16x16.png", TimerIcon, TimerBrush);
	LoadIcon("Resources/DialogueNodeIcons/Icon_NoInterrupt_16x16.png", NoInterruptIcon, NoInterruptBrush);
	LoadIcon("Resources/DialogueNodeIcons/Icon_TextTime_16x16.png", TextTimeIcon, TextTimeBrush);
	LoadIcon("Resources/DialogueNodeIcons/Icon_AudioTime_16x16.png", AudioTimeIcon, AudioTimeBrush);
	
	INITALIZE_CHECKBOX_STYLE(ToggleButtonCheckBox_Red, Red);
	INITALIZE_CHECKBOX_STYLE(ToggleButtonCheckBox_Green, Green);
	INITALIZE_CHECKBOX_STYLE(ToggleButtonCheckBox_Blue, Blue);
	INITALIZE_CHECKBOX_STYLE(ToggleButtonCheckBox_Orange, Orange);
	INITALIZE_CHECKBOX_STYLE(ToggleButtonCheckBox_White, White);
	INITALIZE_CHECKBOX_STYLE(ToggleButtonCheckBox_Transparent, Transparent);

	CheckBoxStyles.ToggleButtonCheckBox_PlayerPrompt = CheckBoxStyles.ToggleButtonCheckBox_White;
	CheckBoxStyles.ToggleButtonCheckBox_PlayerPrompt.Padding = FMargin(0);

	CheckBoxStyles.ToggleButtonCheckBox_DialogueInterrupt = FAppStyle::Get().GetWidgetStyle<FCheckBoxStyle>("ToggleButtonCheckBox");
	FCheckBoxStyle& Tmp = CheckBoxStyles.ToggleButtonCheckBox_DialogueInterrupt;

	Tmp.CheckedImage.TintColor = FlowYapColor::LightGreen;
	Tmp.CheckedHoveredImage.TintColor = FlowYapColor::GreenHovered;
	Tmp.CheckedPressedImage.TintColor = FlowYapColor::GreenPressed;
	Tmp.UncheckedImage.TintColor = FlowYapColor::LightRed;
	Tmp.UncheckedHoveredImage.TintColor = FlowYapColor::RedHovered;
	Tmp.UncheckedPressedImage.TintColor = FlowYapColor::RedPressed;

	Tmp.UndeterminedForeground = FlowYapColor::Green;
	Tmp.UndeterminedImage = Tmp.CheckedImage;
	Tmp.UndeterminedImage.TintColor = FlowYapColor::LightGray;
	Tmp.UndeterminedHoveredImage.TintColor = FlowYapColor::Gray;
	Tmp.UndeterminedPressedImage.TintColor = FlowYapColor::Gray;

	InputTracker = MakeShared<FFlowYapInputTracker>(this);

	FSlateApplication::Get().RegisterInputPreProcessor(InputTracker);
}

void UFlowYapEditorSubsystem::LoadIcon(FString LocalResourcePath, UTexture2D*& Texture, FSlateBrush& Brush, int32 XYSize)
{
	FString ResourcePath = FFlowYapEngineUtils::GetFlowYapPluginDir() / LocalResourcePath;
	Texture = FImageUtils::ImportFileAsTexture2D(ResourcePath);

	Brush.ImageSize = FVector2D(XYSize, XYSize);
	Brush.SetResourceObject(Texture);
}

void UFlowYapEditorSubsystem::Deinitialize()
{
	UFlowYapProjectSettings* ProjectSettings = GetMutableDefault<UFlowYapProjectSettings>();
	ProjectSettings->OnMoodKeysChanged.RemoveAll(this);

	FSlateApplication::Get().UnregisterInputPreProcessor(InputTracker);

	Super::Deinitialize();
}

const FCheckBoxStyles& UFlowYapEditorSubsystem::GetCheckBoxStyles()
{
	return CheckBoxStyles;
}

FFlowYapInputTracker* UFlowYapEditorSubsystem::GetInputTracker()
{
	return InputTracker.Get();
}

#undef LOCTEXT_NAMESPACE
