#include "Yap/YapBit.h"

#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Yap/YapBitReplacement.h"
#include "Yap/YapCharacter.h"
#include "Yap/YapProjectSettings.h"
#include "Yap/YapStreamableManager.h"
#include "Yap/YapTextCalculator.h"

// --------------------------------------------------------------------------------------------
// PUBLIC API

FYapBit::FYapBit()
{
}

const UYapCharacter* FYapBit::GetCharacter(bool bSuppressWarnings) const
{
	if (IsValid(Character))
	{
		return Character;
	}
	
	if (CharacterAsset.IsNull())
	{
#if WITH_EDITOR
		if (!bSuppressWarnings && IsValid(GEditor->EditorWorld))
		{
			UE_LOG(LogYap, Error, TEXT("Fragment is missing a UYapCharacter!"));
		}
#endif
		return nullptr;
	}
	
	if (CharacterAsset.IsValid())
	{
		Character = CharacterAsset.Get();
		return Character;
	}

	Character = CharacterAsset.LoadSynchronous();

	if (!bSuppressWarnings)
	{
		UE_LOG(LogYap, Warning, TEXT("Synchronously loading character: %s"), *CharacterAsset->GetName());

#if WITH_EDITOR
		FNotificationInfo NotificationInfo(INVTEXT("Yap: Synchronously loading UYapCharacter."));
		NotificationInfo.ExpireDuration = 5.0f;
		NotificationInfo.Image = FAppStyle::GetBrush("Icons.WarningWithColor");
		NotificationInfo.SubText = FText::Format(INVTEXT("Loading: {0}\nThis may cause a hitch. This can happen if you try to play a dialogue asset immediately after loading a flow asset. You should try to load the flow asset before it is needed."), FText::FromString(CharacterAsset->GetName()));
		FSlateNotificationManager::Get().AddNotification(NotificationInfo);
#endif
	}
	
	if (!IsValid(Character))
	{
		UE_LOG(LogYap, Error, TEXT("Unknown error - could not load UYapCharacter: %s"), *CharacterAsset->GetName());
	}
	
	return Character;
}

#if WITH_EDITOR
const FSlateBrush& FYapBit::GetSpeakerPortraitBrush() const
{
	const UYapCharacter* Char = GetCharacter(true);

	if (IsValid(Char))
	{
		return Char->GetPortraitBrush(MoodKey);
	}

	return UYapProjectSettings::Get()->GetMissingPortraitBrush();
}
#endif

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

void FYapBit::PreloadContent(UFlowNode_YapDialogue* OwningContext)
{
	if (IsValid(Character))
	{
		return;
	}

	if (CharacterAsset.IsPending())
	{
		FYapStreamableManager::Get().RequestAsyncLoad(CharacterAsset.ToSoftObjectPath(), FStreamableDelegate::CreateUObject(OwningContext, &UFlowNode_YapDialogue::OnCharacterLoadComplete, this));
	}
}

void FYapBit::OnCharacterLoadComplete()
{
	Character = CharacterAsset.Get();
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

	FLOWYAP_REPLACE(CharacterAsset);
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
void FYapBit::SetCharacter(TSoftObjectPtr<UYapCharacter> InCharacter)
{
	CharacterAsset = InCharacter;
	Character = nullptr;
}
#endif

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
#endif

#if WITH_EDITOR
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