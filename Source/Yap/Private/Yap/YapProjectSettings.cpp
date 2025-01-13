// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "Yap/YapProjectSettings.h"

#include "GameplayTagsManager.h"
#include "Interfaces/IPluginManager.h"
#include "Yap/YapGlobals.h"
#include "Yap/YapTextCalculator.h"
#include "Yap/Enums/YapMaturitySetting.h"

#define LOCTEXT_NAMESPACE "Yap"

#if WITH_EDITOR
FName UYapProjectSettings::CategoryName = FName("Yap");
#endif

UYapProjectSettings::UYapProjectSettings()
{

#if WITH_EDITORONLY_DATA
	MoodKeyIconPath.Path = "";

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

	bDefaultSkippableSetting = true;

	TextCalculatorClass = UYapTextCalculator::StaticClass();

	ConversationBrokerClass = nullptr; // You *must* create your own conversation handler class!
	
	FragmentPaddingSliderMax = 5.0;

	DefaultMaturitySetting = EYapMaturitySetting::ChildSafe;
	
#if WITH_EDITOR
	UGameplayTagsManager::Get().OnGetCategoriesMetaFromPropertyHandle.AddUObject(this, &ThisClass::OnGetCategoriesMetaFromPropertyHandle);
#endif
}

#if WITH_EDITOR
FString UYapProjectSettings::GetMoodKeyIconPath(FGameplayTag Key, FString FileExtension) const
{
	int32 Index;

	FString KeyString = (Key.IsValid()) ?  Key.ToString() : "None";

	if (KeyString.FindLastChar('.', Index))
	{
		KeyString = KeyString.RightChop(Index + 1);
	}
	
	if (MoodKeyIconPath.Path == "")
	{		
		static FString ResourcesDir = Yap::GetPluginFolder();
		
		return Yap::GetResourcesFolder() / FString::Format(TEXT("DefaultMoodKeys/{0}.{1}"), { KeyString, FileExtension });
	}
	
	return FPaths::ProjectDir() / FString::Format(TEXT("{0}/{1}.{2}}"), { MoodKeyIconPath.Path, KeyString, FileExtension });
}

FGameplayTagContainer UYapProjectSettings::GetMoodTags()
{
	return UGameplayTagsManager::Get().RequestGameplayTagChildren(Get()->MoodTagsParent);
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
	
	if (one == GET_MEMBER_NAME_CHECKED(ThisClass, MoodKeyIconPath))
	{
		if (two == "Path")
		{
			MoodKeyIconPath.Path = MoodKeyIconPath.Path.RightChop(FullPathDir.Len());
		}
	}
}
#endif

int32 UYapProjectSettings::GetTextWordsPerMinute() const
{
	return TextWordsPerMinute;
}

double UYapProjectSettings::GetMinimumAutoTextTimeLength() const
{
	return MinimumAutoTextTimeLength;
}

double UYapProjectSettings::GetMinimumAutoAudioTimeLength() const
{
	return MinimumAutoAudioTimeLength;
}

double UYapProjectSettings::GetMinimumFragmentTime()
{
	return MinimumFragmentTime;
}

#if WITH_EDITOR
const FString& UYapProjectSettings::GetMoodKeyIconPath() const
{
	static FString CachedPath;

	// Recache the path if it was never calculated, or if the setting is set and the cached path is not equal to it
	if (CachedPath.IsEmpty() || (!MoodKeyIconPath.Path.IsEmpty() && CachedPath != MoodKeyIconPath.Path))
	{
		if (MoodKeyIconPath.Path == "")
		{
			CachedPath = Yap::GetResourcesFolder() / TEXT("DefaultMoodKeys");
		}
		else
		{
			CachedPath = FPaths::ProjectDir() / MoodKeyIconPath.Path;
		}	
	}
	
	return CachedPath;
}

void UYapProjectSettings::RegisterTagFilter(UObject* ClassSource, FName PropertyName, EYap_TagFilter Filter)
{
	TMap<UClass*, EYap_TagFilter>& ClassFiltersForProperty = Get()->TagFilterSubscriptions.FindOrAdd(PropertyName);

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

FString UYapProjectSettings::GetTrimmedGameplayTagString(EYap_TagFilter Filter, const FGameplayTag& PropertyTag)
{
	const FGameplayTag& ParentContainer = *Get()->TagContainers[Filter];
	
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
