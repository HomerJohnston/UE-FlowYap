// Copyright Ghost Pepper Games, Inc. All Rights Reserved.

#include "Yap/FlowYapEditorSubsystem.h"

#include "Yap/FlowYapColors.h"
#include "ImageUtils.h"
#include "Yap/FlowYapEngineUtils.h"
#include "Yap/FlowYapInputTracker.h"
#include "Yap/FlowYapProjectSettings.h"
#include "Yap/YapEditorStyle.h"

#define LOCTEXT_NAMESPACE "FlowYap"

FCheckBoxStyles UFlowYapEditorSubsystem::CheckBoxStyles;

void UFlowYapEditorSubsystem::UpdateMoodKeyIconsMap()
{
	const UFlowYapProjectSettings* ProjectSettings = UFlowYapProjectSettings::Get();

	const TArray<FGameplayTag>& MoodKeys = ProjectSettings->GetMoodKeys();

	MoodKeyIconTextures.Empty(MoodKeys.Num());
	MoodKeyIconBrushes.Empty(MoodKeys.Num());

	for (const FGameplayTag& MoodKey : MoodKeys)
	{
		if (!MoodKey.IsValid())
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
		MoodKeyBrush->SetImageSize(FVector2D(16, 16));
		
		MoodKeyIconBrushes.Add(MoodKey, MoodKeyBrush);
	}
}

UTexture2D* UFlowYapEditorSubsystem::GetMoodKeyIcon(FGameplayTag MoodKey)
{
	UTexture2D** Texture = MoodKeyIconTextures.Find(MoodKey);

	if (Texture)
	{
		return *Texture;
	}

	return nullptr;
}

const FSlateBrush* UFlowYapEditorSubsystem::GetMoodKeyBrush(FGameplayTag Name)
{
	TSharedPtr<FSlateBrush>* Brush = MoodKeyIconBrushes.Find(Name);

	return Brush ? Brush->Get() : FYapEditorStyle::GetImageBrush(YapBrushes.Icon_MoodKeyMissing);
}

// TODO move these to my editor style
#define INITALIZE_CHECKBOX_STYLE(Name, Col) CheckBoxStyles.Name = FAppStyle::Get().GetWidgetStyle<FCheckBoxStyle>("ToggleButtonCheckBox");\
	CheckBoxStyles.Name.CheckedImage.TintColor = YapColor::Col;\
	CheckBoxStyles.Name.CheckedHoveredImage.TintColor = YapColor::Col##Hovered;\
	CheckBoxStyles.Name.CheckedPressedImage.TintColor = YapColor::Col##Pressed\

void UFlowYapEditorSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UFlowYapProjectSettings* ProjectSettings = GetMutableDefault<UFlowYapProjectSettings>();
	ProjectSettings->OnMoodKeysChanged.AddUObject(this, &ThisClass::UpdateMoodKeyIconsMap);

	UpdateMoodKeyIconsMap();

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

	Tmp.CheckedImage.TintColor = YapColor::LightGreen;
	Tmp.CheckedHoveredImage.TintColor = YapColor::GreenHovered;
	Tmp.CheckedPressedImage.TintColor = YapColor::GreenPressed;
	Tmp.UncheckedImage.TintColor = YapColor::LightRed;
	Tmp.UncheckedHoveredImage.TintColor = YapColor::RedHovered;
	Tmp.UncheckedPressedImage.TintColor = YapColor::RedPressed;

	Tmp.UndeterminedForeground = YapColor::Green;
	Tmp.UndeterminedImage = Tmp.CheckedImage;
	Tmp.UndeterminedImage.TintColor = YapColor::LightGray;
	Tmp.UndeterminedHoveredImage.TintColor = YapColor::Gray;
	Tmp.UndeterminedPressedImage.TintColor = YapColor::Gray;

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
