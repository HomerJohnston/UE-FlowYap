// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "Yap/YapProjectSettings.h"

#include "GameplayTagsManager.h"
#include "Interfaces/IPluginManager.h"
#include "Yap/YapGlobals.h"
#include "Yap/Enums/YapDialogueSkippable.h"
#include "Yap/Enums/YapMaturitySetting.h"

#define LOCTEXT_NAMESPACE "Yap"

#if WITH_EDITOR
FName UYapProjectSettings::CategoryName = FName("Yap");
#endif

UYapProjectSettings::UYapProjectSettings()
{

#if WITH_EDITORONLY_DATA
	OverrideMoodTagIconPath.Path = "";

	UGameplayTagsManager& TagsManager = UGameplayTagsManager::Get();

	ConditionTagsParent = TagsManager.AddNativeGameplayTag("Yap.Condition");

	DialogueTagsParent = TagsManager.AddNativeGameplayTag("Yap.Dialogue");

	MoodTagsParent = TagsManager.AddNativeGameplayTag("Yap.Mood");

	TagContainers =
	{
		{ EYap_TagFilter::Conditions, &ConditionTagsParent },
		{ EYap_TagFilter::Prompts, &DialogueTagsParent }
	};
#endif
	
	DefaultTimeModeSetting = EYapTimeMode::AudioTime;

	DefaultMaturitySetting = EYapMaturitySetting::ChildSafe;

	DefaultAssetAudioClasses = { USoundBase::StaticClass() };

	DefaultSkippableSetting = EYapDialogueSkippable::Skippable;
	
#if WITH_EDITOR
	UGameplayTagsManager::Get().OnGetCategoriesMetaFromPropertyHandle.AddUObject(this, &ThisClass::OnGetCategoriesMetaFromPropertyHandle);
#endif
}

#if WITH_EDITOR
FString UYapProjectSettings::GetMoodKeyIconPath(FGameplayTag Key, FString FileExtension)
{
	int32 Index;

	FString KeyString = (Key.IsValid()) ?  Key.ToString() : "None";

	if (KeyString.FindLastChar('.', Index))
	{
		KeyString = KeyString.RightChop(Index + 1);
	}
	
	if (Get().OverrideMoodTagIconPath.Path == "")
	{		
		static FString ResourcesDir = Yap::GetPluginFolder();
		
		return Yap::GetResourcesFolder() / FString::Format(TEXT("DefaultMoodKeys/{0}.{1}"), { KeyString, FileExtension });
	}
	
	return FPaths::ProjectDir() / FString::Format(TEXT("{0}/{1}.{2}}"), { Get().OverrideMoodTagIconPath.Path, KeyString, FileExtension });
}

FGameplayTagContainer UYapProjectSettings::GetMoodTags()
{
	return UGameplayTagsManager::Get().RequestGameplayTagChildren(Get().MoodTagsParent);
}

const TArray<TSoftClassPtr<UObject>>& UYapProjectSettings::GetAudioAssetClasses()
{
	if (Get().OverrideAudioAssetClasses.Num() > 0)
	{
		return Get().OverrideAudioAssetClasses;
	}

	return Get().DefaultAssetAudioClasses;
}

void UYapProjectSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	FName PropertyName = PropertyChangedEvent.Property->GetFName();

	if (PropertyName == GET_MEMBER_NAME_CHECKED(ThisClass, MoodTagsParent))
	{
		OnMoodTagsChanged.Broadcast();
	}
}

void UYapProjectSettings::PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent)
{
	Super::PostEditChangeChainProperty(PropertyChangedEvent);

	// TODO better variable names please
	FName one = PropertyChangedEvent.PropertyChain.GetHead()->GetValue()->GetFName();
	FName two = PropertyChangedEvent.PropertyChain.GetTail()->GetValue()->GetFName();
	
	FString ProjectDir = FPaths::ProjectDir();
	
	FString FullPathDir = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*ProjectDir);
	
	if (one == GET_MEMBER_NAME_CHECKED(ThisClass, OverrideMoodTagIconPath))
	{
		if (two == "Path")
		{
			OverrideMoodTagIconPath.Path = OverrideMoodTagIconPath.Path.RightChop(FullPathDir.Len());
		}
	}
}
#endif

#if WITH_EDITOR
const UTexture2D* UYapProjectSettings::GetMissingPortraitTexture()
{
	UYapProjectSettings& Settings = Get();
	
	if (Settings.MissingPortraitTexture_Loaded && Settings.MissingPortraitTexture == Settings.MissingPortraitTexture_Loaded)
	{
		return Settings.MissingPortraitTexture_Loaded;
	}

	// TODO async loading
	Settings.MissingPortraitTexture_Loaded = Settings.MissingPortraitTexture.LoadSynchronous();
	
	return Settings.MissingPortraitTexture_Loaded;
}

const FString& UYapProjectSettings::GetMoodKeyIconPath()
{
	static FString CachedPath;

	// Recache the path if it was never calculated, or if the setting is set and the cached path is not equal to it
	if (CachedPath.IsEmpty() || (!Get().OverrideMoodTagIconPath.Path.IsEmpty() && CachedPath != Get().OverrideMoodTagIconPath.Path))
	{
		if (Get().OverrideMoodTagIconPath.Path == "")
		{
			CachedPath = Yap::GetResourcesFolder() / TEXT("DefaultMoodKeys");
		}
		else
		{
			CachedPath = FPaths::ProjectDir() / Get().OverrideMoodTagIconPath.Path;
		}	
	}
	
	return CachedPath;
}

void UYapProjectSettings::RegisterTagFilter(UObject* ClassSource, FName PropertyName, EYap_TagFilter Filter)
{
	TMap<UClass*, EYap_TagFilter>& ClassFiltersForProperty = Get().TagFilterSubscriptions.FindOrAdd(PropertyName);

	ClassFiltersForProperty.Add(ClassSource->GetClass(), Filter);
}

void UYapProjectSettings::OnGetCategoriesMetaFromPropertyHandle(TSharedPtr<IPropertyHandle> PropertyHandle, FString& MetaString) const
{
	if (!PropertyHandle)
	{
		return;
	}
	
	const TMap<UClass*, EYap_TagFilter>* ClassFilters = TagFilterSubscriptions.Find(PropertyHandle->GetProperty()->GetFName());

	if (!ClassFilters)
	{
		return;
	}

	TArray<UObject*> OuterObjects;
	PropertyHandle->GetOuterObjects(OuterObjects);

	for (const UObject* PropertyOuter : OuterObjects)
	{
		const EYap_TagFilter* Filter = ClassFilters->Find(PropertyOuter->GetClass());

		if (!Filter)
		{
			continue;
		}

		MetaString = TagContainers[*Filter]->ToString();
	}
}

// TODO someone posted a nicer way to do this in Slackers without this... something about simple name? using the node?? can't remember
FString UYapProjectSettings::GetTrimmedGameplayTagString(EYap_TagFilter Filter, const FGameplayTag& PropertyTag)
{
	const FGameplayTag& ParentContainer = *Get().TagContainers[Filter];
	
	if (ParentContainer.IsValid() && ParentContainer != FGameplayTag::EmptyTag && PropertyTag.MatchesTag(ParentContainer))
	{
		return PropertyTag.ToString().RightChop(ParentContainer.ToString().Len() + 1);
	}

	if (PropertyTag == FGameplayTag::EmptyTag)
	{
		return "";
	}
	
	return PropertyTag.ToString();
}
#endif

#undef LOCTEXT_NAMESPACE
