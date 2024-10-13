// Copyright Ghost Pepper Games, Inc. All Rights Reserved.

#define LOCTEXT_NAMESPACE "FlowYap"
#include "Yap/FlowYapProjectSettings.h"

#include "GameplayTagsManager.h"
#include "Yap/FlowYapEngineUtils.h"
#include "Yap/FlowYapTextCalculator.h"

UFlowYapProjectSettings::UFlowYapProjectSettings()
{
	MoodKeyIconPath.Path = "";

	DefaultTimeModeSetting = EFlowYapTimeMode::AudioTime;

	bDefaultInterruptibleSetting = true;
	
	DialogueWidthAdjustment = 0;

	TextWordsPerMinute = 120;
	
	MinimumAutoTextTimeLength = 2.5;

	MinimumAutoAudioTimeLength = 1.0;

	TextCalculator = UFlowYapTextCalculator::StaticClass();

	AudioTimeCacher = nullptr; // You *must* create your own class and set it to calculate audio time!
	
	DialogueAssetClass = USoundBase::StaticClass();

	ConditionTagsParent = UGameplayTagsManager::Get().AddNativeGameplayTag("Yap.Condition");

	DialogueTagsParent = UGameplayTagsManager::Get().AddNativeGameplayTag("Yap.Dialogue");

	MoodTagsParent = UGameplayTagsManager::Get().AddNativeGameplayTag("Yap.Mood");

	TagContainers =
	{
		{ EFlowYap_TagFilter::Conditions, &ConditionTagsParent },
		{ EFlowYap_TagFilter::Prompts, &DialogueTagsParent }
	};

	CommonFragmentPaddings = { 0.0, 0.1, 0.3, 0.6, 1.0 }; // TODO sort on post edit change properties

	FragmentPaddingSliderMax = 5.0;

#if WITH_EDITOR
	UGameplayTagsManager::Get().OnGetCategoriesMetaFromPropertyHandle.AddUObject(this, &ThisClass::OnGetCategoriesMetaFromPropertyHandle);
#endif
}

#if WITH_EDITOR
FString UFlowYapProjectSettings::GetPortraitIconPath(FGameplayTag Key) const
{
	if (MoodKeyIconPath.Path == "")
	{
		return FPaths::ProjectDir() / FString::Format(TEXT("Resources/Yap/{1}.png"), { MoodKeyIconPath.Path, Key.ToString() });
	}
	
	return FPaths::ProjectDir() / FString::Format(TEXT("{0}/{1}.png"), { MoodKeyIconPath.Path, Key.ToString() });
}

const TArray<FGameplayTag>& UFlowYapProjectSettings::GetMoodKeys() const
{
	return MoodKeys;
}

void UFlowYapProjectSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	FName PropertyName = PropertyChangedEvent.Property->GetFName();

	if (PropertyName == GET_MEMBER_NAME_CHECKED(ThisClass, MoodKeys))
	{
		OnMoodKeysChanged.Broadcast();
	}
}

void UFlowYapProjectSettings::PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent)
{
	Super::PostEditChangeChainProperty(PropertyChangedEvent);

	FName PropertyName = PropertyChangedEvent.Property->GetFName();
	FName MemberPropertyName = PropertyChangedEvent.MemberProperty->GetFName();

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

UClass* UFlowYapProjectSettings::GetDialogueAssetClass() const
{	
	return DialogueAssetClass;
}

int32 UFlowYapProjectSettings::GetDialogueWidthAdjustment() const
{
	return DialogueWidthAdjustment;
}

bool UFlowYapProjectSettings::GetHideTitleTextOnNPCDialogueNodes() const
{
	return bHideTitleTextOnNPCDialogueNodes;
}

int32 UFlowYapProjectSettings::GetTextWordsPerMinute() const
{
	return TextWordsPerMinute;
}

double UFlowYapProjectSettings::GetMinimumAutoTextTimeLength() const
{
	return MinimumAutoTextTimeLength;
}

double UFlowYapProjectSettings::GetMinimumAutoAudioTimeLength() const
{
	return MinimumAutoAudioTimeLength;
}

double UFlowYapProjectSettings::GetMinimumFragmentTime()
{
	return MinimumFragmentTime;
}

void UFlowYapProjectSettings::RegisterTagFilter(UObject* ClassSource, FName PropertyName, EFlowYap_TagFilter Filter)
{
	TMap<UClass*, EFlowYap_TagFilter>& ClassFiltersForProperty = Get()->TagFilterSubscriptions.FindOrAdd(PropertyName);

	ClassFiltersForProperty.Add(ClassSource->GetClass(), Filter);
}

void UFlowYapProjectSettings::OnGetCategoriesMetaFromPropertyHandle(TSharedPtr<IPropertyHandle> PropertyHandle, FString& MetaString) const
{
	if (!PropertyHandle)
	{
		return;
	}
	
	const TMap<UClass*, EFlowYap_TagFilter>* ClassFilters = TagFilterSubscriptions.Find(PropertyHandle->GetProperty()->GetFName());

	if (!ClassFilters)
	{
		return;
	}

	TArray<UObject*> OuterObjects;
	PropertyHandle->GetOuterObjects(OuterObjects);

	for (const UObject* PropertyOuter : OuterObjects)
	{
		const EFlowYap_TagFilter* Filter = ClassFilters->Find(PropertyOuter->GetClass());

		if (!Filter)
		{
			continue;
		}

		MetaString = TagContainers[*Filter]->ToString();
	}
}

FString UFlowYapProjectSettings::GetTrimmedGameplayTagString(EFlowYap_TagFilter Filter, const FGameplayTag& PropertyTag)
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