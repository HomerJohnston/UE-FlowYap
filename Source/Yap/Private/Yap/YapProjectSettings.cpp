// Copyright Ghost Pepper Games, Inc. All Rights Reserved.

#define LOCTEXT_NAMESPACE "FlowYap"
#include "Yap/YapProjectSettings.h"

#include "GameplayTagsManager.h"
#include "Interfaces/IPluginManager.h"
#include "Yap/YapConversationBrokerBase.h"
#include "Yap/YapTextCalculator.h"

FName UYapProjectSettings::CategoryName = FName("Yap");

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

	DialogueAssetClass = nullptr;
	
	AudioTimeCacherClass = nullptr; // You *must* create your own class and set it to calculate audio time!

	TextCalculatorClass = UYapTextCalculator::StaticClass();

	ConversationBrokerClass = nullptr; // You *must* create your own conversation handler class!
	
	FragmentPaddingSliderMax = 5.0;

#if WITH_EDITOR
	UGameplayTagsManager::Get().OnGetCategoriesMetaFromPropertyHandle.AddUObject(this, &ThisClass::OnGetCategoriesMetaFromPropertyHandle);
#endif
}

#if WITH_EDITOR
FString UYapProjectSettings::GetPortraitIconPath(FGameplayTag Key) const
{
	int32 Index;

	FString KeyString = (Key.IsValid()) ?  Key.ToString() : "None";

	if (KeyString.FindLastChar('.', Index))
	{
		KeyString = KeyString.RightChop(Index + 1);
	}
	
	if (MoodKeyIconPath.Path == "")
	{
		TSharedPtr<IPlugin> YapPlugin = IPluginManager::Get().FindPlugin(TEXT("Yap"));
		TSharedPtr<IPlugin> FlowYapPlugin = IPluginManager::Get().FindPlugin(TEXT("FlowYap"));
		
		static FString PluginDir = IPluginManager::Get().FindPlugin(TEXT("Yap"))->GetBaseDir();
		
		return PluginDir / FString::Format(TEXT("Resources/DefaultMoodKeys/{0}.png"), { KeyString });
	}
	
	return FPaths::ProjectDir() / FString::Format(TEXT("{0}/{1}.png"), { MoodKeyIconPath.Path, KeyString });
}

FGameplayTagContainer UYapProjectSettings::GetMoodTags() const
{
	return UGameplayTagsManager::Get().RequestGameplayTagChildren(MoodTagsParent);
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

bool UYapProjectSettings::GetHideTitleTextOnNPCDialogueNodes() const
{
	return bHideTitleTextOnNPCDialogueNodes;
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