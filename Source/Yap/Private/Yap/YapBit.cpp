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

#define LOCTEXT_NAMESPACE "Yap"

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
			MaturitySetting = UYapSubsystem::GetGameMaturitySetting();
		}
		else
		{
			UE_LOG(LogYap, Error, TEXT("UYapSubsystem was invalid in FYapBit::ResolveMaturitySetting. This should not happen! Please contact plugin author. Using project default maturity settings."));
			MaturitySetting = UYapProjectSettings::GetDefaultMaturitySetting();
		}
	}
}

bool FYapBit::GetSkippable(const UFlowNode_YapDialogue* Owner) const
{
	return Skippable.Get(Owner->GetSkippable());
}

bool FYapBit::GetAutoAdvance(const UFlowNode_YapDialogue* Owner) const
{
	if (TimeMode == EYapTimeMode::None || (TimeMode == EYapTimeMode::Default && UYapProjectSettings::GetDefaultTimeModeSetting() == EYapTimeMode::None))
	{
		return false;
	}
	
	return AutoAdvance.Get(Owner->GetAutoAdvance());
}

EYapTimeMode FYapBit::GetTimeMode(EYapMaturitySetting MaturitySetting) const
{
	ResolveMaturitySetting(MaturitySetting);

	EYapTimeMode EffectiveTimeMode = TimeMode;

	if (EffectiveTimeMode == EYapTimeMode::Default)
	{
		EffectiveTimeMode = UYapProjectSettings::GetDefaultTimeModeSetting();
	}

	const TSoftObjectPtr<UObject>& AudioAsset = (MaturitySetting == EYapMaturitySetting::Mature) ? MatureAudioAsset : SafeAudioAsset;
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

#define YAP_ASYNC_LOAD_AND_SET(ASSET, TARGET)\
	if (!IsValid(TARGET) && ASSET.IsPending())\
	{\
		FYapStreamableManager::Get().RequestAsyncLoad(ASSET.ToSoftObjectPath(), FStreamableDelegate::CreateWeakLambda(OwningContext, [this] ()\
		{\
			TARGET = ASSET.Get();\
		}));\
	}

void FYapBit::PreloadContent(UFlowNode_YapDialogue* OwningContext)
{
	YAP_ASYNC_LOAD_AND_SET(SpeakerAsset, Speaker);
	YAP_ASYNC_LOAD_AND_SET(DirectedAtAsset, DirectedAt);
	YAP_ASYNC_LOAD_AND_SET(MatureAudioAsset, MatureAudio);
	YAP_ASYNC_LOAD_AND_SET(SafeAudioAsset, SafeAudio);
}

// --------------------------------------------------------------------------------------------
// Protected

float FYapBit::GetTextTime(EYapMaturitySetting MaturitySetting) const
{
	if (MaturitySetting == EYapMaturitySetting::Unspecified)
	{
		MaturitySetting = UYapProjectSettings::GetDefaultMaturitySetting();
	}

	int32 TWPM = UYapProjectSettings::GetTextWordsPerMinute(); // TODO WPM needs to become a game setting, not a project setting!

	int32 WordCount = (MaturitySetting == EYapMaturitySetting::Mature) ? CachedMatureWordCount : CachedSafeWordCount;

	double SecondsPerWord = 60.0 / (double)TWPM;
	
	double Min = UYapProjectSettings::GetMinimumAutoTextTimeLength();
	
	return FMath::Max(WordCount * SecondsPerWord, Min);
}

TOptional<float> FYapBit::GetAudioTime(EYapMaturitySetting MaturitySetting) const
{
	if (MaturitySetting == EYapMaturitySetting::Unspecified)
	{
		MaturitySetting = UYapProjectSettings::GetDefaultMaturitySetting();
	}
	
	TOptional<float> CachedTime = (MaturitySetting == EYapMaturitySetting::Mature) ? CachedMatureAudioTime : CachedSafeAudioTime;
	TSoftObjectPtr<UObject> Asset = (MaturitySetting == EYapMaturitySetting::Mature) ? MatureAudioAsset : SafeAudioAsset;

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
	FLOWYAP_REPLACE(MatureAudioAsset);
	FLOWYAP_REPLACE(SafeAudioAsset);
	FLOWYAP_REPLACE(MoodTag);
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
void FYapBit::SetTextData(FText* TextToSet, const FText& NewText)
{
	SetDialogueText_Internal(TextToSet, NewText);
}

void FYapBit::RecalculateText()
{
	RecalculateText(&MatureDialogueText);
	RecalculateText(&SafeDialogueText);
}

void FYapBit::RecalculateText(FText* TextToCalculate)
{
	int32 WordCount = -1;

	if (UYapProjectSettings::CacheFragmentWordCount())
	{
		const UYapBroker* Broker = UYapProjectSettings::GetEditorBrokerDefault();

		if (IsValid(Broker))
		{
			WordCount = Broker->CalculateWordCount(*TextToCalculate);
		}
	}

	if (WordCount < 0)
	{
		UE_LOG(LogYap, Error, TEXT("Could not calculate word count!"));
	}

	if (TextToCalculate == &MatureDialogueText)
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
void FYapBit::SetMatureDialogueAudioAsset(UObject* NewAudio)
{
	SetDialogueAudioAsset_Internal(MatureAudioAsset, CachedMatureAudioTime, NewAudio);
}
#endif

#if WITH_EDITOR
void FYapBit::SetSafeDialogueAudioAsset(UObject* NewAudio)
{
	SetDialogueAudioAsset_Internal(SafeAudioAsset, CachedSafeAudioTime, NewAudio);
}
#endif

#if WITH_EDITOR
void FYapBit::SetDialogueText_Internal(FText* TextToSet, const FText& NewText)
{
	*TextToSet = NewText;

	RecalculateText(TextToSet);
}
#endif

#if WITH_EDITOR
void FYapBit::SetDialogueAudioAsset_Internal(TSoftObjectPtr<UObject>& AudioAsset, TOptional<float>& CachedTime, UObject* NewAudio)
{
	AudioAsset = NewAudio;

	const TSoftClassPtr<UYapBroker>& BrokerClass = UYapProjectSettings::GetBrokerClass();
	
	if (BrokerClass.IsNull())
	{
		// TODO I need to handle the case of assigning lots of audio assets without time assigned - can I parse through all assets and fix up? Or log warnings during play so devs can go and fix them?
		UE_LOG(LogYap, Warning, TEXT("No audio time cache class found in project settings! Cannot set audio time!")); 
		CachedTime.Reset();
		return;
	}
	
	UYapBroker* BrokerCDO = BrokerClass.LoadSynchronous()->GetDefaultObject<UYapBroker>();

	float NewCachedTime = BrokerCDO->GetAudioAssetDuration(NewAudio);

	if (NewCachedTime > 0)
	{
		CachedTime = NewCachedTime;
	}
	else
	{
		CachedTime.Reset();
	}
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
		FNotificationInfo NotificationInfo(FText::Format(LOCTEXT("SyncLoadingWarning_Title", "Yap - Synchronously loaded {0}."), FText::FromString(CharacterAsset->GetName())));
		NotificationInfo.ExpireDuration = 5.0f;
		NotificationInfo.Image = FAppStyle::GetBrush("Icons.WarningWithColor");
		NotificationInfo.SubText = FText::Format(LOCTEXT("SyncLoadingWarning_Description", "Loading asset: {0}\nThis may cause a hitch. This can happen if you try to play a dialogue asset immediately after loading a flow asset. You should try to load the flow asset before it is needed."), FText::FromString(CharacterAsset->GetName()));
		FSlateNotificationManager::Get().AddNotification(NotificationInfo);
#endif
	}

	// We get what we get. Nullptr in editor until the separate preload system finishes async loading it; sync loaded asset in play.
	return CharacterPtr;
}

#undef LOCTEXT_NAMESPACE