// Copyright Ghost Pepper Games, Inc. All Rights Reserved.

#include "FlowYap/FlowYapProjectSettings.h"

#include "GameplayTagsManager.h"
#include "FlowYap/FlowYapEngineUtils.h"

#include "FlowYap/FlowYapTextCalculator.h"
#include "FlowYap/Enums/FlowYapErrorLevel.h"

#define LOCTEXT_NAMESPACE "FlowYap"

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

	ConditionContainer = UGameplayTagsManager::Get().AddNativeGameplayTag(TEXT("Yap.Condition"));
	
	UGameplayTagsManager::Get().OnGetCategoriesMetaFromPropertyHandle.AddUObject(this, &ThisClass::OnGetCategoriesMetaFromPropertyHandle);
}

#if WITH_EDITOR
FString UFlowYapProjectSettings::GetPortraitIconPath(FName Key) const
{
	if (MoodKeyIconPath.Path == "")
	{
		return FFlowYapEngineUtils::GetFlowYapPluginDir() / FString::Format(TEXT("Resources/MoodKeys/{1}.png"), { MoodKeyIconPath.Path, Key.ToString() });
	}
	
	return FPaths::ProjectDir() / FString::Format(TEXT("{0}/{1}.png"), { MoodKeyIconPath.Path, Key.ToString() });
}

const TArray<FName>& UFlowYapProjectSettings::GetMoodKeys() const
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

void UFlowYapProjectSettings::RegisterConditionContainerUser(UObject* Object, FName PropertyName)
{
	if (Object->IsTemplate())
	{
		RegisterConditionContainerUser(Object->GetClass(), PropertyName);
	}
}

void UFlowYapProjectSettings::RegisterConditionContainerUser(UClass* Class, FName PropertyName)
{
	if (!PropertyContainerUsers.Contains(Class))
	{
		PropertyContainerUsers.Add(Class, PropertyName);
	}
}

void UFlowYapProjectSettings::OnGetCategoriesMetaFromPropertyHandle(TSharedPtr<IPropertyHandle> PropertyHandle, FString& MetaString) const
{
	TArray<UObject*> OuterObjects;
	PropertyHandle->GetOuterObjects(OuterObjects);

	for (const UObject* PropertyOuter : OuterObjects)
	{
		const FName* FoundProperty = PropertyContainerUsers.FindPair(PropertyOuter->GetClass(), PropertyHandle->GetProperty()->GetFName());

		if (FoundProperty)
		{
			MetaString = GetConditionContainer().ToString();
		}
	}
}
#endif

#undef LOCTEXT_NAMESPACE