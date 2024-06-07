#include "FlowYap/FlowYapBit.h"

#include "FlowYap/FlowYapProjectSettings.h"
#include "FlowYap/FlowYapTextCalculator.h"

// --------------------------------------------------------------------------------------------
// PUBLIC API

#if WITH_EDITOR
FName FFlowYapBit::GetPortraitKey()
{
	if (PortraitKey == NAME_None)
	{
		const TArray<FName>& Keys = GetDefault<UFlowYapProjectSettings>()->GetPortraitKeys();

		if (Keys.Num() > 0)
		{
			PortraitKey = Keys[0];
		}
	}
	
	return PortraitKey;
}
#else
FName FFlowYapBit::GetPortraitKey() const
{
	return PortraitKey;
}
#endif

bool FFlowYapBit::GetInterruptible() const
{
	if (Interruptible == EFlowYapInterruptible::UseProjectDefaults)
	{
		return GetDefault<UFlowYapProjectSettings>()->GetDialogueInterruptibleByDefault();
	}

	return Interruptible == EFlowYapInterruptible::Interruptible;
}

EFlowYapTimeMode FFlowYapBit::GetTimeMode() const
{
	return bUseProjectDefaultTimeSettings ? GetDefault<UFlowYapProjectSettings>()->GetDefaultTimeModeSetting() : TimeMode;
}

double FFlowYapBit::GetTime() const
{
	const UFlowYapProjectSettings* ProjectSettings = GetDefault<UFlowYapProjectSettings>();
	
	EFlowYapTimeMode ActualTimeMode = bUseProjectDefaultTimeSettings ? ProjectSettings->GetDefaultTimeModeSetting() : TimeMode;

	if (ActualTimeMode == EFlowYapTimeMode::AudioTime && (!HasDialogueAudioAsset() || CachedAudioTime <= 0))
	{
		ActualTimeMode = EFlowYapTimeMode::TextTime;
	}

	switch (ActualTimeMode)
	{
	case EFlowYapTimeMode::ManualTime:
		{
			return ManualTime;
		}
	case EFlowYapTimeMode::AudioTime:
		{
			return CachedAudioTime;
		}
	case EFlowYapTimeMode::TextTime:
		{
			return GetTextTime();
		}
	default:
		{
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
