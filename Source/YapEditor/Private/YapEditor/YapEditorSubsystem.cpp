// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "YapEditor/YapEditorSubsystem.h"

#include "GameplayTagsEditorModule.h"
#include "GameplayTagsManager.h"
#include "ILiveCodingModule.h"
#include "YapEditor/YapColors.h"
#include "UnrealEdGlobals.h"
#include "Editor/UnrealEdEngine.h"
#include "Yap/YapCharacter.h"
#include "YapEditor/YapInputTracker.h"
#include "Yap/YapProjectSettings.h"
#include "YapEditor/YapEditorStyle.h"
#include "Engine/Texture2D.h"
#include "YapEditor/YapDeveloperSettings.h"

#define LOCTEXT_NAMESPACE "YapEditor"

bool UYapEditorSubsystem::bLiveCodingInProgress = true;
TArray<TWeakObjectPtr<UObject>> UYapEditorSubsystem::OpenedAssets = {};


void UYapEditorSubsystem::UpdateMoodTagBrushesIfRequired()
{
	FGameplayTagContainer ProjectMoodTags = UYapProjectSettings::GetMoodTags();
	
	if (ProjectMoodTags.Num() != CachedMoodTags.Num() || !CachedMoodTags.HasAllExact(ProjectMoodTags))
	{
		UpdateMoodTagBrushes();
	}

	CachedMoodTags = ProjectMoodTags;
}

void UYapEditorSubsystem::UpdateMoodTagBrushes()
{
	FGameplayTagContainer ProjectMoodTags = UYapProjectSettings::GetMoodTags();
	
	MoodTagIconBrushes.Empty(ProjectMoodTags.Num() + 1);

	for (const FGameplayTag& MoodTag : ProjectMoodTags)
	{
		BuildIcon(MoodTag);
	}

	BuildIcon(FGameplayTag::EmptyTag);

	CachedMoodTags = ProjectMoodTags;
}

void UYapEditorSubsystem::BuildIcon(const FGameplayTag& MoodTag)
{
	TSharedPtr<FSlateImageBrush> ImageBrush = nullptr;
	
	// Attempt to load SVG
	FString IconPath = UYapProjectSettings::GetMoodTagIconPath(MoodTag, "svg");
	ImageBrush = MakeShared<FSlateVectorImageBrush>(IconPath, FVector2f(16, 16), YapColor::White);

	// Attempt to load PNG
	if (!ImageBrush)
	{
		IconPath = UYapProjectSettings::GetMoodTagIconPath(MoodTag, "png");
		ImageBrush = MakeShared<FSlateImageBrush>(IconPath, FVector2f(16, 16), YapColor::White);
	}
	
	// Found nothing
	if (!ImageBrush)
	{
		return;
	}

	MoodTagIconBrushes.Add(MoodTag, ImageBrush);
}

TSharedPtr<FSlateImageBrush> UYapEditorSubsystem::GetMoodTagIcon(FGameplayTag MoodTag)
{
	TSharedPtr<FSlateImageBrush>* Brush = MoodTagIconBrushes.Find(MoodTag);

	if (Brush)
	{
		return *Brush;
	}

	return nullptr;
}

const FSlateBrush* UYapEditorSubsystem::GetMoodTagBrush(FGameplayTag Name)
{
	TSharedPtr<FSlateImageBrush>* Brush = MoodTagIconBrushes.Find(Name);

	return Brush ? Brush->Get() : FYapEditorStyle::GetImageBrush(YapBrushes.Icon_MoodTag_Missing);
}

const FSlateBrush* UYapEditorSubsystem::GetCharacterPortraitBrush(const UYapCharacter* Character, const FGameplayTag& MoodTag)
{
	if (!IsValid(Character))
	{
		return nullptr;
	}
	
	const UTexture2D* Texture = Character->GetPortraitTexture(MoodTag);

	// Load the default missing portrait
	if (!IsValid(Texture))
	{
		if (!IsValid(Get()->MissingPortraitTexture))
		{
			Get()->MissingPortraitTexture = UYapProjectSettings::GetMissingPortraitTextureAsset().LoadSynchronous();
		}

		Texture = Get()->MissingPortraitTexture;
	}
	
	FSlateBrush* PortraitBrush = Get()->CharacterPortraitBrushes.Find(Texture);

	if (PortraitBrush)
	{
		// TODO: somehow check if it is out of date
		return PortraitBrush;
	}

	FSlateBrush NewPortraitBrush;

	NewPortraitBrush.ImageSize = FVector2D(128, 128);
	NewPortraitBrush.SetResourceObject(const_cast<UTexture2D*>(Texture)); // TODO make sure this is safe and that there isn't a better system
	NewPortraitBrush.SetUVRegion(FBox2D(FVector2D(0,0), FVector2D(1,1)));
	NewPortraitBrush.DrawAs = ESlateBrushDrawType::Box;
	NewPortraitBrush.Margin = 0;

	return &Get()->CharacterPortraitBrushes.Add(Texture, NewPortraitBrush);
}

void UYapEditorSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	if (UGameplayTagsManager::Get().FindTagSource("YapGameplayTags.ini") == nullptr)
	{
		IGameplayTagsEditorModule::Get().AddNewGameplayTagSource("YapGameplayTags.ini");
	}

	// TODO -- this does NOT work! Keep it hidden until I can figure out why. For some reason FSlateSVGRasterizer will NOT attempt to load a file that didn't exist on startup?
	// Also see: FDetailCustomization_YapProjectSetting for hidden/disabled button to run UpdateMoodTagBrushes().
	//UGameplayTagsManager::Get().OnEditorRefreshGameplayTagTree.AddUObject(this, &UYapEditorSubsystem::UpdateMoodTagBrushesIfRequired);
	UpdateMoodTagBrushes();

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

void UYapEditorSubsystem::Deinitialize()
{
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

	if (IsMoodTagProperty(PropertyHandle))
	{
		MetaString = GetDefault<UYapProjectSettings>()->GetMoodTagsParent().ToString();
		return;
	}
}

bool UYapEditorSubsystem::IsMoodTagProperty(TSharedPtr<IPropertyHandle> PropertyHandle) const
{
	TArray<FName> PropertyNames
	{
		"MoodTag",
		"MoodTags",
		"MoodTags2"
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

	if (UYapDeveloperSettings::GetCloseAndReopenAssetsOnLiveCoding())
	{
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

bool UYapEditorSubsystem::GetMoodTagsDirty()
{
	//return Get()->bMoodTagsDirty;
	FGameplayTagContainer ProjectMoodTags = UYapProjectSettings::GetMoodTags();
	
	if (ProjectMoodTags.Num() != Get()->CachedMoodTags.Num() || !Get()->CachedMoodTags.HasAllExact(ProjectMoodTags))
	{
		return true;
	}

	return false;
}

TStatId UYapEditorSubsystem::UYapEditorSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UYapEditorSubsystem, STATGROUP_Tickables);
}

void UYapEditorSubsystem::OnPatchComplete()
{
	if (UYapDeveloperSettings::GetCloseAndReopenAssetsOnLiveCoding())
	{
		ReOpenAssets();
	}
	
	bLiveCodingInProgress = false;
}

#undef LOCTEXT_NAMESPACE
