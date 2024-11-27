#include "Yap/YapBit.h"

#include "Yap/YapBitReplacement.h"
#include "Yap/YapCharacter.h"
#include "Yap/YapProjectSettings.h"
#include "Yap/YapTextCalculator.h"

// --------------------------------------------------------------------------------------------
// PUBLIC API

FYapBit::FYapBit()
{
#if WITH_EDITOR
	FGameplayTagContainer Keys = UYapProjectSettings::Get()->GetMoodTags();

	if (Keys.Num() > 0)
	{
		MoodKey = Keys.First();
	}
#endif
}

#if WITH_EDITOR
const FSlateBrush* FYapBit::GetSpeakerPortraitBrush() const
{
	if (Character.IsPending())
	{
		UE_LOG(LogYap, Warning, TEXT("Synchronously loading portrait brushes. This should ONLY happen during editor time!"))
		Character.LoadSynchronous();
	}
	
	if (Character.IsValid())
	{
		return &Character->GetPortraitBrush(MoodKey);
	}

	return nullptr;
}

FGameplayTag FYapBit::GetMoodKeyLazyInit()
{
	if (!MoodKey.IsValid())
	{
		FGameplayTagContainer Keys = UYapProjectSettings::Get()->GetMoodTags();

		if (Keys.Num() > 0)
		{
			MoodKey = Keys.First();
		}
	}
	
	return MoodKey;
}
#endif

FGameplayTag FYapBit::GetMoodKey() const
{
	return MoodKey;
}

bool FYapBit::GetInterruptible() const
{
	if (Interruptible == EFlowYapInterruptible::UseProjectDefaults)
	{
		return UYapProjectSettings::Get()->GetDialogueInterruptibleByDefault();
	}

	return Interruptible == EFlowYapInterruptible::Interruptible;
}

EYapTimeMode FYapBit::GetTimeMode() const
{
	return bUseProjectDefaultTimeSettings ? UYapProjectSettings::Get()->GetDefaultTimeModeSetting() : TimeMode;
}

double FYapBit::GetTime() const
{
	// TODO clamp minimums from project settings
	
	const UYapProjectSettings* ProjectSettings = UYapProjectSettings::Get();
	
	EYapTimeMode ActualTimeMode = bUseProjectDefaultTimeSettings ? ProjectSettings->GetDefaultTimeModeSetting() : TimeMode;

	if (ActualTimeMode == EYapTimeMode::AudioTime && (!HasDialogueAudioAsset() || CachedAudioTime <= 0))
	{
		ActualTimeMode = EYapTimeMode::TextTime;
	}

	switch (ActualTimeMode)
	{
	case EYapTimeMode::ManualTime:
		{
			return ManualTime;
		}
	case EYapTimeMode::AudioTime:
		{
			return CachedAudioTime;
		}
	case EYapTimeMode::TextTime:
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

double FYapBit::GetTextTime() const
{
	const UYapProjectSettings* ProjectSettings = UYapProjectSettings::Get();

	int32 TWPM = ProjectSettings->GetTextWordsPerMinute(); // TODO WPM needs to become a game setting, not a project setting!
	double SecondsPerWord = 60.0 / (double)TWPM;
			
	double Min = ProjectSettings->GetMinimumAutoTextTimeLength();
	return FMath::Max(CachedWordCount * SecondsPerWord, Min);
}


// --------------------------------------------------------------------------------------------
// Public


FYapBit& FYapBit::operator=(const FYapBitReplacement& Replacement)
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

#if WITH_EDITOR
void FYapBit::SetDialogueText(const FText& NewText)
{
	DialogueText = NewText;

	if (UYapProjectSettings::Get()->CacheFragmentWordCount())
	{
		TSoftClassPtr<UYapTextCalculator> TextCalculatorClass = UYapProjectSettings::Get()->GetTextCalculator();
		CachedWordCount = TextCalculatorClass.LoadSynchronous()->GetDefaultObject<UYapTextCalculator>()->CalculateWordCount(DialogueText);
	}
	else
	{
		CachedWordCount = -1;
	}
}

void FYapBit::SetDialogueAudioAsset(UObject* NewAudio)
{
	DialogueAudioAsset = NewAudio;

	TSoftClassPtr<UYapAudioTimeCacher> AudioTimeCacheClass = UYapProjectSettings::Get()->GetAudioTimeCacheClass();

	if (AudioTimeCacheClass == nullptr)
	{
		UE_LOG(LogYap, Warning, TEXT("No audio time cache class found in project settings! Cannot set audio time!"));
		CachedAudioTime = -1.0;
		return;
	}
	
	if (AudioTimeCacheClass == nullptr)
	{
		UE_LOG(LogYap, Warning, TEXT("No audio time cache class found in project settings! Cannot set audio time!"));
		CachedAudioTime = -1.0;
		return;
	}

	UYapAudioTimeCacher* CacherCDO = AudioTimeCacheClass.LoadSynchronous()->GetDefaultObject<UYapAudioTimeCacher>();

	CachedAudioTime = CacherCDO->GetAudioLengthInSeconds(DialogueAudioAsset);
}
#endif