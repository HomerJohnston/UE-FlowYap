// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "Yap/YapProjectSettings.h"

#if WITH_EDITOR
#include "GameplayTagsManager.h"
#include "Interfaces/IPluginManager.h"
#include "Yap/YapGlobals.h"
#endif
#include "Yap/Enums/YapDialogueSkippable.h"
#include "Yap/Enums/YapMaturitySetting.h"
#include "Yap/Enums/YapMissingAudioErrorLevel.h"

#define LOCTEXT_NAMESPACE "Yap"

#if WITH_EDITOR
FName UYapProjectSettings::CategoryName = FName("Yap");
#endif

UYapProjectSettings::UYapProjectSettings()
{
#if WITH_EDITORONLY_DATA
	MoodTagIconPath.Path = "";

	UGameplayTagsManager& TagsManager = UGameplayTagsManager::Get();

	DialogueTagsParent = TagsManager.AddNativeGameplayTag("Yap.Dialogue");

	MoodTagsParent = TagsManager.AddNativeGameplayTag("Yap.Mood");

	TagContainers =
	{
		{ EYap_TagFilter::Prompts, &DialogueTagsParent }
	};
#endif
	
	DefaultTimeModeSetting = EYapTimeMode::AudioTime;

	DefaultMaturitySetting = EYapMaturitySetting::Mature;

#if WITH_EDITOR
	DefaultAssetAudioClasses = { USoundBase::StaticClass() };
#endif
	
	DefaultSkippableSetting = EYapDialogueSkippable::Skippable;

	MissingAudioErrorLevel = EYapMissingAudioErrorLevel::OK;

	MissingPortraitTexture = FSoftObjectPath("/Yap/T_Avatar_Missing.T_Avatar_Missing");
	
#if WITH_EDITOR
	UGameplayTagsManager::Get().OnGetCategoriesMetaFromPropertyHandle.AddUObject(this, &ThisClass::OnGetCategoriesMetaFromPropertyHandle);
#endif
}

#if WITH_EDITOR
FString UYapProjectSettings::GetMoodTagIconPath(FGameplayTag Key, FString FileExtension)
{
	int32 Index;

	FString KeyString = (Key.IsValid()) ?  Key.ToString() : "None";

	if (KeyString.FindLastChar('.', Index))
	{
		KeyString = KeyString.RightChop(Index + 1);
	}
	
	if (Get().MoodTagIconPath.Path == "")
	{		
		static FString ResourcesDir = Yap::GetPluginFolder();
		
		return Yap::GetResourcesFolder() / FString::Format(TEXT("DefaultMoodTags/{0}.{1}"), { KeyString, FileExtension });
	}
	
	return FPaths::ProjectDir() / FString::Format(TEXT("{0}/{1}.{2}}"), { Get().MoodTagIconPath.Path, KeyString, FileExtension });
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
}

void UYapProjectSettings::PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent)
{
	Super::PostEditChangeChainProperty(PropertyChangedEvent);

	// TODO better variable names please
	FName one = PropertyChangedEvent.PropertyChain.GetHead()->GetValue()->GetFName();
	FName two = PropertyChangedEvent.PropertyChain.GetTail()->GetValue()->GetFName();
	
	if (one == GET_MEMBER_NAME_CHECKED(ThisClass, MoodTagIconPath))
	{
		if (two == "Path")
		{
			FString ProjectDir = FPaths::ProjectDir();
	
			FString FullPathDir = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*ProjectDir);

			if (MoodTagIconPath.Path.StartsWith(FullPathDir))
			{
				MoodTagIconPath.Path = MoodTagIconPath.Path.RightChop(FullPathDir.Len());
			}
		}
	}
}
#endif

#if WITH_EDITOR
const FString& UYapProjectSettings::GetMoodTagIconPath()
{
	static FString CachedPath;

	// Recache the path if it was never calculated, or if the setting is set and the cached path is not equal to it
	if (CachedPath.IsEmpty() || (!Get().MoodTagIconPath.Path.IsEmpty() && CachedPath != Get().MoodTagIconPath.Path))
	{
		if (Get().MoodTagIconPath.Path == "")
		{
			CachedPath = Yap::GetResourcesFolder() / TEXT("DefaultMoodTags");
		}
		else
		{
			CachedPath = FPaths::ProjectDir() / Get().MoodTagIconPath.Path;
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
