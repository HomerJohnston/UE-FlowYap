// Copyright Ghost Pepper Games, Inc. All Rights Reserved.

#include "FlowYapEditorSubsystem.h"

#include "FlowYapColors.h"
#include "FlowYap/FlowYapProjectSettings.h"
#include "ImageUtils.h"
#include "FlowYap/FlowYapEngineUtils.h"
#include "FlowYapInputTracker.h"

#define LOCTEXT_NAMESPACE "FlowYap"

FCheckBoxStyles UFlowYapEditorSubsystem::CheckBoxStyles;

void UFlowYapEditorSubsystem::UpdatePortraitKeyIconsMap()
{
	const UFlowYapProjectSettings* ProjectSettings = GetDefault<UFlowYapProjectSettings>();

	const TArray<FName>& PortraitKeys = ProjectSettings->GetPortraitKeys();

	PortraitKeyIconTextures.Empty(PortraitKeys.Num());

	for (const FName& PortraitKey : PortraitKeys)
	{
		if (PortraitKey.IsNone())
		{
			continue;
		}
		
		FString IconPath = ProjectSettings->GetPortraitIconPath(PortraitKey);
		UTexture2D* PortraitKeyIcon = FImageUtils::ImportFileAsTexture2D(IconPath);

		if (!IsValid(PortraitKeyIcon))
		{
			continue;
		}

		PortraitKeyIconTextures.Add(PortraitKey, PortraitKeyIcon);

		TSharedPtr<FSlateBrush> PortraitKeyBrush = MakeShareable(new FSlateBrush);

		PortraitKeyBrush->SetResourceObject(PortraitKeyIcon);
		PortraitKeyBrush->SetImageSize({16, 16});
		
		PortraitKeyIconBrushes.Add(PortraitKey, PortraitKeyBrush);
	}
}

UTexture2D* UFlowYapEditorSubsystem::GetPortraitKeyIcon(FName PortraitKey)
{
	UTexture2D** Texture = PortraitKeyIconTextures.Find(PortraitKey);

	if (Texture)
	{
		return *Texture;
	}

	return nullptr;
}

const FSlateBrush* UFlowYapEditorSubsystem::GetPortraitKeyBrush(FName Name)
{
	TSharedPtr<FSlateBrush>* Brush = PortraitKeyIconBrushes.Find(Name);

	return Brush ? Brush->Get() : nullptr;
}

#define INITALIZE_CHECKBOX_STYLE(Name, Color) CheckBoxStyles.Name = FAppStyle::Get().GetWidgetStyle<FCheckBoxStyle>("ToggleButtonCheckBox");\
	CheckBoxStyles.Name##.CheckedImage.TintColor = FLinearColor(FlowYapColor::##Color##);\
	CheckBoxStyles.Name##.CheckedHoveredImage.TintColor = FLinearColor(FlowYapColor::##Color##Hovered);\
	CheckBoxStyles.Name##.CheckedPressedImage.TintColor = FLinearColor(FlowYapColor::##Color##Pressed)\

void UFlowYapEditorSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UFlowYapProjectSettings* ProjectSettings = GetMutableDefault<UFlowYapProjectSettings>();
	ProjectSettings->OnPortraitKeysChanged.AddUObject(this, &ThisClass::UpdatePortraitKeyIconsMap);

	UpdatePortraitKeyIconsMap();

	LoadIcon("Resources/DialogueNodeIcons/Icon_Timer_16x16.png", TimerIcon, TimerBrush);
	LoadIcon("Resources/DialogueNodeIcons/Icon_UserInterrupt_16x16.png", UserInterruptIcon, UserInterruptBrush);
	LoadIcon("Resources/DialogueNodeIcons/Icon_TextTime_16x16.png", TextTimeIcon, TextTimeBrush);
	LoadIcon("Resources/DialogueNodeIcons/Icon_AudioTime_16x16.png", AudioTimeIcon, AudioTimeBrush);
	
	INITALIZE_CHECKBOX_STYLE(ToggleButtonCheckBox_Red, Red);
	INITALIZE_CHECKBOX_STYLE(ToggleButtonCheckBox_Green, Green);
	INITALIZE_CHECKBOX_STYLE(ToggleButtonCheckBox_Blue, Blue);
	INITALIZE_CHECKBOX_STYLE(ToggleButtonCheckBox_Orange, Orange);
	INITALIZE_CHECKBOX_STYLE(ToggleButtonCheckBox_White, White);

	CheckBoxStyles.ToggleButtonCheckBox_White.Padding = FMargin(0);

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
	ProjectSettings->OnPortraitKeysChanged.RemoveAll(this);

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
