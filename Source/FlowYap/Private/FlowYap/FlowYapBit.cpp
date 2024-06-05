#include "FlowYap/FlowYapBit.h"

#include "FlowYap/FlowYapProjectSettings.h"
#include "FlowYap/FlowYapTextCalculator.h"

// --------------------------------------------------------------------------------------------
// PUBLIC API

bool FFlowYapBit::GetInterruptible() const
{
	if (bUseProjectDefaultTimeSettings)
	{
		const UFlowYapProjectSettings* ProjectSettings = GetDefault<UFlowYapProjectSettings>();

		return ProjectSettings->GetDefaultInterruptibleSetting();
	}

	return bInterruptible;	
}

double FFlowYapBit::GetTime() const
{
	const UFlowYapProjectSettings* ProjectSettings = GetDefault<UFlowYapProjectSettings>();
	
	EFlowYapTimeMode TimeMode = bUseProjectDefaultTimeSettings ? ProjectSettings->GetDefaultTimeModeSetting() : TimeMode;

	if (TimeMode == EFlowYapTimeMode::AudioLength && CachedAudioTime <= 0)
	{
		TimeMode = ProjectSettings->GetInvalidAudioFallbackTimeMode();
	}
	
	switch (TimeMode)
	{
	case EFlowYapTimeMode::ManualTime:
		{
			return ManualTime;
		}
	case EFlowYapTimeMode::AudioLength:
		{
			return CachedAudioTime;
		}
	case EFlowYapTimeMode::TextLength:
		{
			return GetTextTime();
		}
	default:
		{
			check(false)
			return -1.0;
		}
	}
}

// --------------------------------------------------------------------------------------------
// Protected

double FFlowYapBit::GetTextTime() const
{
	const UFlowYapProjectSettings* ProjectSettings = GetDefault<UFlowYapProjectSettings>();

	int32 TWPM = ProjectSettings->GetTextWordsPerMinute(); // TODO WPM needs to become a game setting, not a project setting!
	double SecondsPerWord = 60.0 / (double)TWPM;
			
	double Min = ProjectSettings->GetMinimumAutoTextTimeLength();
	return FMath::Max(CachedWordCount * SecondsPerWord, Min);
}

// --------------------------------------------------------------------------------------------
// EDITOR API

void FFlowYapBit::SetDialogueText(const FText& NewText)
{
	DialogueText = NewText;

	TSubclassOf<UFlowYapTextCalculator> TextCalculatorClass = GetDefault<UFlowYapProjectSettings>()->GetTextCalculator();

	CachedWordCount = TextCalculatorClass->GetDefaultObject<UFlowYapTextCalculator>()->CalculateWordCount(DialogueText);
}

void FFlowYapBit::SetDialogueAudioAsset(UObject* NewAudio)
{
	DialogueAudioAsset = NewAudio;

	TSubclassOf<UFlowYapAudioTimeCacher> AudioTimeCacheClass = GetDefault<UFlowYapProjectSettings>()->GetAudioTimeCacheClass();

	if (AudioTimeCacheClass == nullptr)
	{
		UE_LOG(FlowYap, Warning, TEXT("No audio time cache class found in project settings! Cannot set audio time!"));
		CachedAudioTime = -1.0;
		return;
	}
	
	UFlowYapAudioTimeCacher* CacherCDO = AudioTimeCacheClass->GetDefaultObject<UFlowYapAudioTimeCacher>();

	if (AudioTimeCacheClass == nullptr)
	{
		UE_LOG(FlowYap, Warning, TEXT("No audio time cache class found in project settings! Cannot set audio time!"));
		CachedAudioTime = -1.0;
		return;
	}
	
	CachedAudioTime = CacherCDO->GetAudioLengthInSeconds(DialogueAudioAsset);
}

EFlowYapTimeMode FFlowYapBit::GetTimeMode() const
{
	const UFlowYapProjectSettings* ProjectSettings = GetDefault<UFlowYapProjectSettings>();
	 
	EFlowYapTimeMode ActualTimeMode = bUseProjectDefaultTimeSettings ? ProjectSettings->GetDefaultTimeModeSetting() : TimeMode;

	if (ActualTimeMode == EFlowYapTimeMode::AudioLength && !HasDialogueAudioAsset())
	{
		ActualTimeMode = ProjectSettings->GetInvalidAudioFallbackTimeMode();
	}

	return ActualTimeMode;
}
