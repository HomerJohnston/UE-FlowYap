// Copyright Ghost Pepper Games, Inc. All Rights Reserved.

#include "Yap/YapEditorSubsystem.h"

#include "GameplayTagsEditorModule.h"
#include "GameplayTagsManager.h"
#include "Yap/YapColors.h"
#include "ImageUtils.h"
#include "Yap/YapEditorSettings.h"
#include "Yap/YapEngineUtils.h"
#include "Yap/YapInputTracker.h"
#include "Yap/YapProjectSettings.h"
#include "Yap/YapEditorStyle.h"
#include "Yap/Nodes/FlowNode_YapDialogue.h"

#define LOCTEXT_NAMESPACE "FlowYap"

FCheckBoxStyles UYapEditorSubsystem::CheckBoxStyles;

void UYapEditorSubsystem::UpdateMoodKeyIconsMap()
{
	const UYapProjectSettings* ProjectSettings = UYapProjectSettings::Get();

	FGameplayTagContainer MoodKeys = ProjectSettings->GetMoodTags();
	
	MoodKeyIconTextures.Empty(MoodKeys.Num());
	MoodKeyIconBrushes.Empty(MoodKeys.Num());

	for (const FGameplayTag& MoodKey : MoodKeys)
	{
		BuildIcon(MoodKey);
	}

	BuildIcon(FGameplayTag::EmptyTag);
}

void UYapEditorSubsystem::BuildIcon(const FGameplayTag& MoodKey)
{
	const UYapProjectSettings* ProjectSettings = UYapProjectSettings::Get();

	FString IconPath = ProjectSettings->GetPortraitIconPath(MoodKey);
	UTexture2D* MoodKeyIcon = FImageUtils::ImportFileAsTexture2D(IconPath);

	if (!IsValid(MoodKeyIcon))
	{
		return;
	}

	MoodKeyIconTextures.Add(MoodKey, MoodKeyIcon);

	TSharedPtr<FSlateBrush> MoodKeyBrush = MakeShareable(new FSlateBrush);

	MoodKeyBrush->SetResourceObject(MoodKeyIcon);
	MoodKeyBrush->SetImageSize(FVector2D(16, 16));
		
	MoodKeyIconBrushes.Add(MoodKey, MoodKeyBrush);
}

UTexture2D* UYapEditorSubsystem::GetMoodKeyIcon(FGameplayTag MoodKey)
{
	UTexture2D** Texture = MoodKeyIconTextures.Find(MoodKey);

	if (Texture)
	{
		return *Texture;
	}

	return nullptr;
}

const FSlateBrush* UYapEditorSubsystem::GetMoodKeyBrush(FGameplayTag Name)
{
	TSharedPtr<FSlateBrush>* Brush = MoodKeyIconBrushes.Find(Name);

	return Brush ? Brush->Get() : FYapEditorStyle::GetImageBrush(YapBrushes.Icon_MoodKeyMissing);
}

// TODO move these to my editor style
#define INITALIZE_CHECKBOX_STYLE(Name, Col) CheckBoxStyles.Name = FAppStyle::Get().GetWidgetStyle<FCheckBoxStyle>("ToggleButtonCheckBox");\
	CheckBoxStyles.Name.CheckedImage.TintColor = YapColor::Col;\
	CheckBoxStyles.Name.CheckedHoveredImage.TintColor = YapColor::Col##Hovered;\
	CheckBoxStyles.Name.CheckedPressedImage.TintColor = YapColor::Col##Pressed\

void UYapEditorSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	if (UGameplayTagsManager::Get().FindTagSource("YapGameplayTags.ini") == nullptr)
	{
		IGameplayTagsEditorModule::Get().AddNewGameplayTagSource("YapGameplayTags.ini");
	}

	UYapProjectSettings* ProjectSettings = GetMutableDefault<UYapProjectSettings>();

	ProjectSettings->OnMoodTagsChanged.AddUObject(this, &ThisClass::UpdateMoodKeyIconsMap);

	UpdateMoodKeyIconsMap();

	// TODO move these into my style
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

	InputTracker = MakeShared<FYapInputTracker>(this);

	FSlateApplication::Get().RegisterInputPreProcessor(InputTracker);

	SetupGameplayTagFiltering();
}

void UYapEditorSubsystem::LoadIcon(FString LocalResourcePath, UTexture2D*& Texture, FSlateBrush& Brush, int32 XYSize)
{
	FString ResourcePath = FYapEngineUtils::GetFlowYapPluginDir() / LocalResourcePath;
	Texture = FImageUtils::ImportFileAsTexture2D(ResourcePath);

	Brush.ImageSize = FVector2D(XYSize, XYSize);
	Brush.SetResourceObject(Texture);
}

void UYapEditorSubsystem::Deinitialize()
{
	UYapProjectSettings* ProjectSettings = GetMutableDefault<UYapProjectSettings>();
	ProjectSettings->OnMoodTagsChanged.RemoveAll(this);

	FSlateApplication::Get().UnregisterInputPreProcessor(InputTracker);

	Super::Deinitialize();
}

const FCheckBoxStyles& UYapEditorSubsystem::GetCheckBoxStyles()
{
	return CheckBoxStyles;
}

FYapInputTracker* UYapEditorSubsystem::GetInputTracker()
{
	return InputTracker.Get();
}

void UYapEditorSubsystem::SetupGameplayTagFiltering()
{
	FragmentTagFilterDelegateHandle = UGameplayTagsManager::Get().OnGetCategoriesMetaFromPropertyHandle.AddUObject(this, &ThisClass::OnGetCategoriesMetaFromPropertyHandle);
}

void UYapEditorSubsystem::OnGetCategoriesMetaFromPropertyHandle(TSharedPtr<IPropertyHandle> PropertyHandle, FString& MetaString) const
{
	if (!PropertyHandle->HasMetaData("Yap"))
	{
		return;
	}

	if (IsMoodKeyProperty(PropertyHandle))
	{
		MetaString = GetDefault<UYapProjectSettings>()->MoodTagsParent.ToString();
		return;
	}
}

bool UYapEditorSubsystem::IsMoodKeyProperty(TSharedPtr<IPropertyHandle> PropertyHandle) const
{
	TArray<FName> PropertyNames
	{
		"MoodKey",
		"MoodKeys",
		"MoodKeys2"
	};
	
	if (PropertyNames.Contains(PropertyHandle->GetProperty()->GetFName()))
	{
		return true;
	}
		
	return false;
}

#undef LOCTEXT_NAMESPACE
