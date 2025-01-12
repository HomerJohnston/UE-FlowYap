// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "YapEditor/YapEditorSubsystem.h"

#include "GameplayTagsEditorModule.h"
#include "GameplayTagsManager.h"
#include "ILiveCodingModule.h"
#include "YapEditor/YapColors.h"
#include "ImageUtils.h"
#include "UnrealEdGlobals.h"
#include "Editor/UnrealEdEngine.h"
#include "Yap/YapEngineUtils.h"
#include "Yap/YapGlobals.h"
#include "YapEditor/YapInputTracker.h"
#include "Yap/YapProjectSettings.h"
#include "YapEditor/YapEditorStyle.h"
#include "Yap/Nodes/FlowNode_YapDialogue.h"

#define LOCTEXT_NAMESPACE "YapEditor"

bool UYapEditorSubsystem::bLiveCodingInProgress = true;
TArray<TWeakObjectPtr<UObject>> UYapEditorSubsystem::OpenedAssets = {};


void UYapEditorSubsystem::UpdateMoodKeyBrushes()
{
	FGameplayTagContainer MoodKeys = UYapProjectSettings::GetMoodTags();
	
	MoodKeyIconBrushes.Empty(MoodKeys.Num() + 1);

	for (const FGameplayTag& MoodKey : MoodKeys)
	{
		BuildIcon(MoodKey);
	}

	BuildIcon(FGameplayTag::EmptyTag);
}

void UYapEditorSubsystem::BuildIcon(const FGameplayTag& MoodKey)
{
	const UYapProjectSettings* ProjectSettings = UYapProjectSettings::Get();

	//FString Dir = UYapProjectSettings::GetPortraitIconPath() FPaths::plu RootToContentDir("Icon_Caret_Right", L".svg");
	//FSlateVectorImageBrush* TesTst = new FSlateVectorImageBrush(Dir, FVector2f(16, 16), YapColor::White);

	TSharedPtr<FSlateImageBrush> ImageBrush = nullptr;
	
	// Attempt to load SVG
	FString IconPath = ProjectSettings->GetMoodKeyIconPath(MoodKey, "svg");
	ImageBrush = MakeShared<FSlateVectorImageBrush>(IconPath, FVector2f(16, 16), YapColor::White);

	if (!ImageBrush)
	{
		IconPath = ProjectSettings->GetMoodKeyIconPath(MoodKey, "png");
		ImageBrush = MakeShared<FSlateImageBrush>(IconPath, FVector2f(16, 16), YapColor::White);
	}
	
	// Found nothing
	if (!ImageBrush)
	{
		return;
	}

	MoodKeyIconBrushes.Add(MoodKey, ImageBrush);
}

TSharedPtr<FSlateImageBrush> UYapEditorSubsystem::GetMoodKeyIcon(FGameplayTag MoodKey)
{
	TSharedPtr<FSlateImageBrush>* Brush = MoodKeyIconBrushes.Find(MoodKey);

	if (Brush)
	{
		return *Brush;
	}

	return nullptr;
}

const FSlateBrush* UYapEditorSubsystem::GetMoodKeyBrush(FGameplayTag Name)
{
	TSharedPtr<FSlateImageBrush>* Brush = MoodKeyIconBrushes.Find(Name);

	return Brush ? Brush->Get() : FYapEditorStyle::GetImageBrush(YapBrushes.Icon_MoodKey_Missing);
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

	ProjectSettings->OnMoodTagsChanged.AddUObject(this, &ThisClass::UpdateMoodKeyBrushes);

	UpdateMoodKeyBrushes();

	if (IsValid(GUnrealEd))
	{
		InputTracker = MakeShared<FYapInputTracker>(this);
		auto& SlateApp = FSlateApplication::Get();
		SlateApp.RegisterInputPreProcessor(InputTracker);
	}

	SetupGameplayTagFiltering();
	
#if WITH_LIVE_CODING
	if (ILiveCodingModule* LiveCoding = FModuleManager::LoadModulePtr<ILiveCodingModule>(LIVE_CODING_MODULE_NAME))
	{
		OnPatchCompleteHandle = LiveCoding->GetOnPatchCompleteDelegate().AddUObject(this, &UYapEditorSubsystem::OnPatchComplete);
	}
#endif
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

	if (IsValid(GUnrealEd))
	{
		FSlateApplication::Get().UnregisterInputPreProcessor(InputTracker);
	}

	Super::Deinitialize();
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

void UYapEditorSubsystem::UpdateLiveCodingState(bool bNewState)
{
	if (bNewState == bLiveCodingInProgress)
	{
		return;
	}

	bLiveCodingInProgress = bNewState;

	UAssetEditorSubsystem* Subsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();

	if (bLiveCodingInProgress)
	{
		TArray<UObject*> OpenedAssetsTemp = Subsystem->GetAllEditedAssets();

		for (UObject* Obj : OpenedAssetsTemp)
		{
			OpenedAssets.Add(Obj);
		}
		
		Subsystem->CloseAllAssetEditors();
	}
	else
	{
		GEditor->GetTimerManager()->SetTimerForNextTick(this, &UYapEditorSubsystem::ReOpenAssets);
	}
}

void UYapEditorSubsystem::ReOpenAssets()
{
	UAssetEditorSubsystem* Subsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();

	if (IsValid(Subsystem))
	{
		for (TWeakObjectPtr<UObject> Asset : OpenedAssets)
		{
			if (Asset.IsValid())
			{
				Subsystem->OpenEditorForAsset(Asset.Get());
			}
		}
	}

	OpenedAssets.Empty();
}

void UYapEditorSubsystem::Tick(float DeltaTime)
{
#if WITH_LIVE_CODING
	if (ILiveCodingModule* LiveCoding = FModuleManager::LoadModulePtr<ILiveCodingModule>(LIVE_CODING_MODULE_NAME))
	{
		if (LiveCoding->IsCompiling())
		{
			UpdateLiveCodingState(true);
		}
		else
		{
			UpdateLiveCodingState(false);
		}
	}
#endif
}

TStatId UYapEditorSubsystem::UYapEditorSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UYapEditorSubsystem, STATGROUP_Tickables);
}

void UYapEditorSubsystem::OnPatchComplete()
{
	ReOpenAssets();
	
	bLiveCodingInProgress = false;
}

#undef LOCTEXT_NAMESPACE
