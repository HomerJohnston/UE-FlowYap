// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license.

#include "Yap/YapBit.h"

#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Yap/YapBitReplacement.h"
#include "Yap/YapCharacter.h"
#include "Yap/YapProjectSettings.h"
#include "Yap/YapStreamableManager.h"
#include "Yap/YapSubsystem.h"
#include "Yap/YapTextCalculator.h"

// --------------------------------------------------------------------------------------------
// PUBLIC API

FYapBit::FYapBit()
{
}

const UYapCharacter* FYapBit::GetSpeaker(EYapWarnings Warnings) const
{
	return GetCharacterAsset_Internal(SpeakerAsset, Speaker, Warnings);
}

const UYapCharacter* FYapBit::GetDirectedAt() const
{
	return GetCharacterAsset_Internal(DirectedAtAsset, DirectedAt, EYapWarnings::Ignore); // We ignore warnings for directed-at assets; they're always allowed to be normally unset
}

const FText& FYapBit::GetDialogueText(EYapMaturitySetting MaturitySetting) const
{
	ResolveMaturitySetting(MaturitySetting);
	check(MaturitySetting != EYapMaturitySetting::Unspecified);

	if (MaturitySetting == EYapMaturitySetting::Mature || SafeDialogueText.IsEmpty())
	{
		return MatureDialogueText;
	}

	return SafeDialogueText;
}

const FText& FYapBit::GetTitleText(EYapMaturitySetting MaturitySetting) const
{
	ResolveMaturitySetting(MaturitySetting);
	check(MaturitySetting != EYapMaturitySetting::Unspecified);
	
	if (MaturitySetting == EYapMaturitySetting::Mature || SafeTitleText.IsEmpty())
	{
		return MatureTitleText;
	}

	return SafeTitleText;
}

void FYapBit::ResolveMaturitySetting(EYapMaturitySetting& MaturitySetting) const
{
	if (MaturitySetting == EYapMaturitySetting::Unspecified)
	{
		if (IsValid(UYapSubsystem::Get()))
		{
			MaturitySetting = UYapSubsystem::GetMaturitySetting();
		}
		else
		{
			MaturitySetting = UYapProjectSettings::GetDefaultMaturitySetting();
		}	
	}

	check(MaturitySetting != EYapMaturitySetting::Unspecified);
}

#if WITH_EDITOR
const FSlateBrush& FYapBit::GetSpeakerPortraitBrush() const
{
	const UYapCharacter* Char = GetSpeaker(EYapWarnings::Ignore);

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

EYapDialogueSkippable FYapBit::GetSkippable(EYapMaturitySetting MaturitySetting) const
{
	ResolveMaturitySetting(MaturitySetting);

	if (GetTime(MaturitySetting).IsSet())
	{
		return Skippable;
	}

	return EYapDialogueSkippable::Skippable;
}

EYapTimeMode FYapBit::GetTimeMode(EYapMaturitySetting MaturitySetting) const
{
	ResolveMaturitySetting(MaturitySetting);

	EYapTimeMode EffectiveTimeMode = TimeMode;

	if (EffectiveTimeMode == EYapTimeMode::Default)
	{
		EffectiveTimeMode = UYapProjectSettings::Get()->GetDefaultTimeModeSetting();
	}

	const TSoftObjectPtr<UObject>& AudioAsset = (MaturitySetting == EYapMaturitySetting::Mature) ? MatureDialogueAudioAsset : SafeDialogueAudioAsset;
	const TOptional<float>& AudioTime = (MaturitySetting == EYapMaturitySetting::Mature) ? CachedMatureAudioTime : CachedSafeAudioTime;
	
	if (EffectiveTimeMode == EYapTimeMode::AudioTime && (AudioAsset.IsNull() || !AudioTime.IsSet()))
	{
		EffectiveTimeMode = EYapTimeMode::TextTime;
	}

	return EffectiveTimeMode;
}

TOptional<float> FYapBit::GetTime(EYapMaturitySetting MaturitySetting) const
{
	ResolveMaturitySetting(MaturitySetting);

	// TODO clamp minimums from project settings
	
	EYapTimeMode EffectiveTimeMode = GetTimeMode(MaturitySetting);

	switch (EffectiveTimeMode)
	{
		case EYapTimeMode::ManualTime:
		{
			return ManualTime;
		}
		case EYapTimeMode::AudioTime:
		{
			return GetAudioTime();
		}
		case EYapTimeMode::TextTime:
		{
			return GetTextTime();
		}
		default:
		{
			return TOptional<float>();
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

float FYapBit::GetTextTime() const
{
	const UYapProjectSettings* ProjectSettings = UYapProjectSettings::Get();

	int32 TWPM = ProjectSettings->GetTextWordsPerMinute(); // TODO WPM needs to become a game setting, not a project setting!
	double SecondsPerWord = 60.0 / (double)TWPM;
			
	double Min = ProjectSettings->GetMinimumAutoTextTimeLength();
	return FMath::Max(CachedMatureWordCount * SecondsPerWord, Min);
}

TOptional<float> FYapBit::GetAudioTime() const
{
	EYapMaturitySetting MaturitySetting = UYapProjectSettings::GetDefaultMaturitySetting();
	
	TOptional<float> CachedTime = (MaturitySetting == EYapMaturitySetting::Mature) ? CachedMatureAudioTime : CachedSafeAudioTime;
	TSoftObjectPtr<UObject> Asset = (MaturitySetting == EYapMaturitySetting::Mature) ? MatureDialogueAudioAsset : SafeDialogueAudioAsset;

	if (Asset.IsNull() || !CachedTime.IsSet())
	{
		return TOptional<float>();
	}

	return CachedTime;
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
	FLOWYAP_REPLACE(TimeMode);
	FLOWYAP_REPLACE(ManualTime);
	FLOWYAP_REPLACE(CachedMatureWordCount);
	FLOWYAP_REPLACE(CachedSafeWordCount);
	FLOWYAP_REPLACE(CachedMatureAudioTime);
	FLOWYAP_REPLACE(CachedSafeAudioTime);

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

void FYapBit::SetTitleText(FText* TextToSet, const FText& NewText)
{
	*TextToSet = NewText;
}
#endif

#if WITH_EDITOR
void FYapBit::SetDialogueText(FText* TextToSet, const FText& NewText)
{
	SetDialogueText_Internal(TextToSet, NewText);
}

#endif

#if WITH_EDITOR
void FYapBit::SetDialogueAudioAsset(UObject* NewAudio)
{
	SetDialogueAudioAsset_Internal(MatureDialogueAudioAsset, CachedMatureAudioTime, NewAudio);
}
#endif

#if WITH_EDITOR
void FYapBit::SetDialogueAudioAssetSafe(UObject* NewAudio)
{
	SetDialogueAudioAsset_Internal(SafeDialogueAudioAsset, CachedSafeAudioTime, NewAudio);
}
#endif

#if WITH_EDITOR
void FYapBit::SetDialogueText_Internal(FText* TextToSet, const FText& NewText)
{
	*TextToSet = NewText;

	int32 WordCount = -1;

	if (UYapProjectSettings::Get()->CacheFragmentWordCount())
	{
		TSoftClassPtr<UYapTextCalculator> TextCalculatorClass = UYapProjectSettings::Get()->GetTextCalculator();
		WordCount = TextCalculatorClass.LoadSynchronous()->GetDefaultObject<UYapTextCalculator>()->CalculateWordCount(NewText);
	}

	if (TextToSet == &MatureDialogueText)
	{
		CachedMatureWordCount = WordCount;
	}
	else
	{
		CachedSafeWordCount = WordCount;
	}
}
#endif

#if WITH_EDITOR
void FYapBit::SetDialogueAudioAsset_Internal(TSoftObjectPtr<UObject>& AudioAsset, TOptional<float>& CachedTime, UObject* NewAudio)
{
	AudioAsset = NewAudio;

	TSoftClassPtr<UYapAudioTimeCacher> AudioTimeCacheClass = UYapProjectSettings::Get()->GetAudioTimeCacheClass();

	if (AudioTimeCacheClass == nullptr)
	{
		UE_LOG(LogYap, Warning, TEXT("No audio time cache class found in project settings! Cannot set audio time!"));
		CachedTime.Reset();
		return;
	}
	
	if (AudioTimeCacheClass == nullptr)
	{
		UE_LOG(LogYap, Warning, TEXT("No audio time cache class found in project settings! Cannot set audio time!"));
		CachedTime.Reset();
		return;
	}

	UYapAudioTimeCacher* CacherCDO = AudioTimeCacheClass.LoadSynchronous()->GetDefaultObject<UYapAudioTimeCacher>();

	CachedTime = CacherCDO->GetAudioLengthInSeconds(AudioAsset);
}
#endif

const UYapCharacter* FYapBit::GetCharacterAsset_Internal(TSoftObjectPtr<UYapCharacter> CharacterAsset, TObjectPtr<UYapCharacter>& CharacterPtr, EYapWarnings Warnings) const
{
	if (IsValid(CharacterPtr))
	{
		return CharacterPtr;
	}
	
	if (CharacterAsset.IsNull())
	{
#if WITH_EDITOR
		if (IsValid(GEditor->PlayWorld) && Warnings >= EYapWarnings::Show)
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
