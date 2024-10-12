#include "Yap/FlowYapBit.h"

#include "Yap/FlowYapBitReplacement.h"
#include "Yap/FlowYapCharacter.h"
#include "Yap/FlowYapProjectSettings.h"
#include "Yap/FlowYapTextCalculator.h"

// --------------------------------------------------------------------------------------------
// PUBLIC API

FFlowYapBit::FFlowYapBit()
{
#if WITH_EDITOR
	const TArray<FGameplayTag>& Keys = UFlowYapProjectSettings::Get()->GetMoodKeys();

	if (Keys.Num() > 0)
	{
		MoodKey = Keys[0];
	}
#endif
}

#if WITH_EDITOR
const FSlateBrush* FFlowYapBit::GetSpeakerPortraitBrush() const
{
	if (Character.IsPending())
	{
		UE_LOG(FlowYap, Warning, TEXT("Synchronously loading portrait brushes. This should ONLY happen during editor time!"))
		Character.LoadSynchronous();
	}
	
	if (Character.IsValid())
	{
		return Character->GetPortraitBrush(MoodKey);
	}

	return nullptr;
}

FGameplayTag FFlowYapBit::GetMoodKeyLazyInit()
{
	if (!MoodKey.IsValid())
	{
		const TArray<FGameplayTag>& Keys = UFlowYapProjectSettings::Get()->GetMoodKeys();

		if (Keys.Num() > 0)
		{
			MoodKey = Keys[0];
		}
	}
	
	return MoodKey;
}
#endif

FGameplayTag FFlowYapBit::GetMoodKey() const
{
	return MoodKey;
}

bool FFlowYapBit::GetInterruptible() const
{
	if (Interruptible == EFlowYapInterruptible::UseProjectDefaults)
	{
		return UFlowYapProjectSettings::Get()->GetDialogueInterruptibleByDefault();
	}

	return Interruptible == EFlowYapInterruptible::Interruptible;
}

EFlowYapTimeMode FFlowYapBit::GetTimeMode() const
{
	return bUseProjectDefaultTimeSettings ? UFlowYapProjectSettings::Get()->GetDefaultTimeModeSetting() : TimeMode;
}

double FFlowYapBit::GetTime() const
{
	// TODO clamp minimums from project settings
	
	const UFlowYapProjectSettings* ProjectSettings = UFlowYapProjectSettings::Get();
	
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
	const UFlowYapProjectSettings* ProjectSettings = UFlowYapProjectSettings::Get();

	int32 TWPM = ProjectSettings->GetTextWordsPerMinute(); // TODO WPM needs to become a game setting, not a project setting!
	double SecondsPerWord = 60.0 / (double)TWPM;
			
	double Min = ProjectSettings->GetMinimumAutoTextTimeLength();
	return FMath::Max(CachedWordCount * SecondsPerWord, Min);
}


// --------------------------------------------------------------------------------------------
// Public


FFlowYapBit& FFlowYapBit::operator=(const FFlowYapBitReplacement& Replacement)
{
#define FLOWYAP_REPLACE(X) if (Replacement.X.IsSet()) {X = Replacement.X.GetValue(); }  

	FLOWYAP_REPLACE(Character);
	FLOWYAP_REPLACE(TitleText);
	FLOWYAP_REPLACE(DialogueText);
	FLOWYAP_REPLACE(DialogueAudioAsset);
	FLOWYAP_REPLACE(MoodKey);
	FLOWYAP_REPLACE(bUseProjectDefaultTimeSettings);
	FLOWYAP_REPLACE(TimeMode);
	FLOWYAP_REPLACE(ManualTime);
	FLOWYAP_REPLACE(CachedWordCount);
	FLOWYAP_REPLACE(CachedAudioTime);

#undef FLOWYAP_REPLACE
	
	return *this;
}

// --------------------------------------------------------------------------------------------
// EDITOR API

void FFlowYapBit::SetDialogueText(const FText& NewText)
{
	DialogueText = NewText;

	TSubclassOf<UFlowYapTextCalculator> TextCalculatorClass = UFlowYapProjectSettings::Get()->GetTextCalculator();

	CachedWordCount = TextCalculatorClass->GetDefaultObject<UFlowYapTextCalculator>()->CalculateWordCount(DialogueText);
}

void FFlowYapBit::SetDialogueAudioAsset(UObject* NewAudio)
{
	DialogueAudioAsset = NewAudio;

	TSubclassOf<UFlowYapAudioTimeCacher> AudioTimeCacheClass = UFlowYapProjectSettings::Get()->GetAudioTimeCacheClass();

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
