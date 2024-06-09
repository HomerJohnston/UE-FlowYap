// Copyright Ghost Pepper Games, Inc. All Rights Reserved.

#include "FlowYap/FlowYapProjectSettings.h"

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

	AudioTimeCacher = nullptr; // UFlowYapAudioTimeCacher::StaticClass(); // You must create your own class and calculate time in it!
	
	DialogueAssetClass = USoundBase::StaticClass();
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
#endif

#undef LOCTEXT_NAMESPACE