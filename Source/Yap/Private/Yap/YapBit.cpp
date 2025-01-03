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

const UYapCharacter* FYapBit::GetSpeaker() const
{
	return GetCharacterAsset_Internal(SpeakerAsset, Speaker);
}

const UYapCharacter* FYapBit::GetDirectedAt() const
{
	return GetCharacterAsset_Internal(DirectedAtAsset, DirectedAt);
}

const FText& FYapBit::GetSpokenText(bool bUseChildSafeText) const
{
	if (!bUseChildSafeText)
	{
		return GetMatureDialogueText();
	}

	if (GetSafeDialogueText().IsEmpty())
	{
		return GetMatureDialogueText();
	}

	return GetSafeDialogueText();
}

#if WITH_EDITOR
const FSlateBrush& FYapBit::GetSpeakerPortraitBrush() const
{
	const UYapCharacter* Char = GetSpeaker();

	if (IsValid(Char))
	{
		return Char->GetPortraitBrush(MoodKey);
	}

	return UYapProjectSettings::Get()->GetMissingPortraitBrush();
}

const FSlateBrush& FYapBit::GetDirectedAtPortraitBrush() const
{
	const UYapCharacter* Char = GetDirectedAt();

	if (IsValid(Char))
	{
		return Char->GetPortraitBrush(FGameplayTag::EmptyTag);
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
	if (!IsValid(Speaker))
	{
		if (SpeakerAsset.IsPending())
		{
			FYapStreamableManager::Get().RequestAsyncLoad(SpeakerAsset.ToSoftObjectPath(), FStreamableDelegate::CreateUObject(OwningContext, &UFlowNode_YapDialogue::OnCharacterLoadComplete, this, &SpeakerAsset, &Speaker));
		}
	}

	if (!IsValid(DirectedAt))
	{
		if (DirectedAtAsset.IsPending())
		{
			FYapStreamableManager::Get().RequestAsyncLoad(DirectedAtAsset.ToSoftObjectPath(), FStreamableDelegate::CreateUObject(OwningContext, &UFlowNode_YapDialogue::OnCharacterLoadComplete, this, &DirectedAtAsset, &DirectedAt));
		}
	}
}

void FYapBit::OnCharacterLoadComplete(TSoftObjectPtr<UYapCharacter>* CharacterAsset, TObjectPtr<UYapCharacter>* Character)
{
	*Character = CharacterAsset->Get();
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

	FLOWYAP_REPLACE(SpeakerAsset);
	FLOWYAP_REPLACE(DirectedAtAsset);
	FLOWYAP_REPLACE(MatureTitleText);
	FLOWYAP_REPLACE(MatureDialogueText);
	FLOWYAP_REPLACE(SafeTitleText);
	FLOWYAP_REPLACE(SafeDialogueText);
	FLOWYAP_REPLACE(MatureDialogueAudioAsset);
	FLOWYAP_REPLACE(SafeDialogueAudioAsset);
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
void FYapBit::SetSpeaker(TSoftObjectPtr<UYapCharacter> InCharacter)
{
	SpeakerAsset = InCharacter;
	Speaker = nullptr;
}

void FYapBit::SetDirectedAt(TSoftObjectPtr<UYapCharacter> InDirectedAt)
{
	DirectedAtAsset = InDirectedAt;
	DirectedAt = nullptr;
}
#endif

#if WITH_EDITOR
void FYapBit::SetDialogueText(const FText& NewText)
{
	SetDialogueText_Internal(MatureDialogueText, CachedWordCount, NewText);
}

void FYapBit::SetDialogueTextSafe(const FText& NewText)
{
	SetDialogueText_Internal(SafeDialogueText, CachedWordCountSafe, NewText);
}

#endif

#if WITH_EDITOR
void FYapBit::SetDialogueAudioAsset(UObject* NewAudio)
{
	SetDialogueAudioAsset_Internal(MatureDialogueAudioAsset, CachedAudioTime, NewAudio);
}
#endif

#if WITH_EDITOR
void FYapBit::SetDialogueAudioAssetSafe(UObject* NewAudio)
{
	SetDialogueAudioAsset_Internal(SafeDialogueAudioAsset, CachedAudioTimeSafe, NewAudio);
}
#endif

#if WITH_EDITOR
void FYapBit::SetDialogueText_Internal(FText& Text, int32& WordCount, const FText& NewText)
{
	Text = NewText;

	if (UYapProjectSettings::Get()->CacheFragmentWordCount())
	{
		TSoftClassPtr<UYapTextCalculator> TextCalculatorClass = UYapProjectSettings::Get()->GetTextCalculator();
		WordCount = TextCalculatorClass.LoadSynchronous()->GetDefaultObject<UYapTextCalculator>()->CalculateWordCount(NewText);
	}
	else
	{
		WordCount = -1;
	}
}
#endif

#if WITH_EDITOR
void FYapBit::SetDialogueAudioAsset_Internal(TSoftObjectPtr<UObject>& AudioAsset, double& CachedTime, UObject* NewAudio)
{
	AudioAsset = NewAudio;

	TSoftClassPtr<UYapAudioTimeCacher> AudioTimeCacheClass = UYapProjectSettings::Get()->GetAudioTimeCacheClass();

	if (AudioTimeCacheClass == nullptr)
	{
		UE_LOG(LogYap, Warning, TEXT("No audio time cache class found in project settings! Cannot set audio time!"));
		CachedTime = -1.0;
		return;
	}
	
	if (AudioTimeCacheClass == nullptr)
	{
		UE_LOG(LogYap, Warning, TEXT("No audio time cache class found in project settings! Cannot set audio time!"));
		CachedTime = -1.0;
		return;
	}

	UYapAudioTimeCacher* CacherCDO = AudioTimeCacheClass.LoadSynchronous()->GetDefaultObject<UYapAudioTimeCacher>();

	CachedTime = CacherCDO->GetAudioLengthInSeconds(AudioAsset);
}
#endif

const UYapCharacter* FYapBit::GetCharacterAsset_Internal(TSoftObjectPtr<UYapCharacter> CharacterAsset, TObjectPtr<UYapCharacter>& CharacterPtr) const
{
	if (IsValid(CharacterPtr))
	{
		return CharacterPtr;
	}
	
	if (CharacterAsset.IsNull())
	{
#if WITH_EDITOR
		if (IsValid(GEditor->PlayWorld))
		{
			UE_LOG(LogYap, Error, TEXT("Fragment is missing a UYapCharacter!"));
		}
#endif
		return nullptr;
	}
	
	if (CharacterAsset.IsValid())
	{
		CharacterPtr = CharacterAsset.Get();
		return CharacterPtr;
	}

	if (GEngine->GetCurrentPlayWorld())
	{
		CharacterPtr = CharacterAsset.LoadSynchronous();

		UE_LOG(LogYap, Warning, TEXT("Synchronously loaded character: %s"), *CharacterAsset->GetName());

#if WITH_EDITOR
		FNotificationInfo NotificationInfo(FText::Format(INVTEXT("Yap - Synchronously loaded {0}."), FText::FromString(CharacterAsset->GetName())));
		NotificationInfo.ExpireDuration = 5.0f;
		NotificationInfo.Image = FAppStyle::GetBrush("Icons.WarningWithColor");
		NotificationInfo.SubText = FText::Format(INVTEXT("Loading: {0}\nThis may cause a hitch. This can happen if you try to play a dialogue asset immediately after loading a flow asset. You should try to load the flow asset before it is needed."), FText::FromString(CharacterAsset->GetName()));
		FSlateNotificationManager::Get().AddNotification(NotificationInfo);
#endif
	}

	// We get what we get. Nullptr in editor until the separate preload system finishes async loading it; sync loaded asset in play.
	return CharacterPtr;
}
