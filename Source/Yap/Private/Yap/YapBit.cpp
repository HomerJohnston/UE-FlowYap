// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license.

#include "Yap/YapBit.h"

#include "Yap/YapBitReplacement.h"
#include "Yap/YapCharacter.h"
#include "Yap/YapProjectSettings.h"
#include "Yap/YapStreamableManager.h"
#include "Yap/YapSubsystem.h"

#define LOCTEXT_NAMESPACE "Yap"

#define YAP_ASYNC_LOAD(ASSET, HANDLE)\
if (ASSET.IsPending())\
HANDLE = FYapStreamableManager::Get().RequestAsyncLoad(ASSET.ToSoftObjectPath());\

// --------------------------------------------------------------------------------------------
// PUBLIC API

#if WITH_EDITOR
void FYapText::Set(const FText& InText)
{
	Txt = InText;

	int32 NewWordCount = -1;

	if (Txt.IsEmptyOrWhitespace())
	{
		WordCnt = 0;
		return;
	}
	
	if (UYapProjectSettings::CacheFragmentWordCount())
	{
		const UYapBroker* Broker = UYapProjectSettings::GetEditorBrokerDefault();

		if (IsValid(Broker))
		{
			NewWordCount = Broker->CalculateWordCount(Txt);
		}
	}

	if (NewWordCount < 0)
	{
		UE_LOG(LogYap, Error, TEXT("Could not calculate word count!"));
	}

	WordCnt = NewWordCount;
}
#endif

FYapBit::FYapBit()
{
}

const UYapCharacter* FYapBit::GetSpeaker(EYapWarnings Warnings)
{
	return GetCharacter_Internal(SpeakerAsset, SpeakerHandle, Warnings);
}

const UYapCharacter* FYapBit::GetDirectedAt()
{
	return GetCharacter_Internal(DirectedAtAsset, DirectedAtHandle, EYapWarnings::Ignore); // We ignore warnings for directed-at assets; they're always allowed to be normally unset
}

const UYapCharacter* FYapBit::GetCharacter_Internal(const TSoftObjectPtr<UYapCharacter>& CharacterAsset, TSharedPtr<FStreamableHandle>& Handle, EYapWarnings Warnings)
{
	if (CharacterAsset.IsNull())
	{
		return nullptr;
	}

	if (CharacterAsset.IsValid())
	{
		return CharacterAsset.Get();
	}
	
	// If we're mid-game, force a sync load
	TWeakObjectPtr<UWorld> World = UYapSubsystem::GetStaticWorld();
	
	if (World.IsValid() && World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE)
	{
		if (Handle->IsLoadingInProgress())
		{
			UE_LOG(LogYap, Warning, TEXT("Interrupting async load to get yap character asset"));
		}
		
		Handle = FYapStreamableManager::Get().RequestSyncLoad(CharacterAsset.LoadSynchronous());
		UE_LOG(LogYap, Warning, TEXT("Synchronously loaded character: %s"), *CharacterAsset->GetName());
	}

	return CharacterAsset.Get();
}

const FText& FYapBit::GetDialogueText(EYapMaturitySetting MaturitySetting) const
{
	ResolveMaturitySetting(MaturitySetting);

	check(MaturitySetting != EYapMaturitySetting::Unspecified);
	
	if (MaturitySetting == EYapMaturitySetting::Mature || SafeDialogueText.Get().IsEmpty())
	{
		return MatureDialogueText.Get();
	}

	return SafeDialogueText.Get();
}

const FText& FYapBit::GetTitleText(EYapMaturitySetting MaturitySetting) const
{
	ResolveMaturitySetting(MaturitySetting);
	check(MaturitySetting != EYapMaturitySetting::Unspecified);
	
	if (MaturitySetting == EYapMaturitySetting::Mature || SafeTitleText.Get().IsEmpty())
	{
		return MatureTitleText.Get();
	}

	return SafeTitleText.Get();
}

void FYapBit::ResolveMaturitySetting(EYapMaturitySetting& MaturitySetting) const
{
	if (!bNeedsChildSafeData)
	{
		MaturitySetting = EYapMaturitySetting::Mature;
		return;
	}
	
	if (MaturitySetting == EYapMaturitySetting::Unspecified)
	{
		if (IsValid(UYapSubsystem::Get()))
		{
			MaturitySetting = UYapSubsystem::GetGameMaturitySetting();
		}
		else
		{
			UE_LOG(LogYap, Error, TEXT("UYapSubsystem was invalid in FYapBit::ResolveMaturitySetting. This should not happen! Please contact plugin author. Defaulting to mature."));
			MaturitySetting = EYapMaturitySetting::Mature;
		}
	}
}

bool FYapBit::HasAudioAsset(EYapMaturitySetting MaturitySetting) const
{
	if (MaturitySetting == EYapMaturitySetting::Mature)
	{
		return !MatureAudioAsset.IsNull();
	}

	if (MaturitySetting == EYapMaturitySetting::ChildSafe)
	{
		return !SafeAudioAsset.IsNull();
	}

	if (MaturitySetting == EYapMaturitySetting::Unspecified)
	{
		return !MatureAudioAsset.IsNull() || !SafeAudioAsset.IsNull();
	}

	return false;
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

	if (EffectiveTimeMode == EYapTimeMode::AudioTime)
	{
		const TSoftObjectPtr<UObject>& Asset = (MaturitySetting == EYapMaturitySetting::Mature) ? MatureAudioAsset : SafeAudioAsset;

		if (Asset.IsNull())
		{
			EffectiveTimeMode = EYapTimeMode::TextTime;
		}
	}

	return EffectiveTimeMode;
}

TOptional<float> FYapBit::GetTime(EYapMaturitySetting MaturitySetting) const
{
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
			return GetAudioTime(MaturitySetting);
		}
		case EYapTimeMode::TextTime:
		{
			return GetTextTime(MaturitySetting);
		}
		default:
		{
			return TOptional<float>();
		}
	}
}

void FYapBit::PreloadContent(UFlowNode_YapDialogue* OwningContext)
{
	YAP_ASYNC_LOAD(SpeakerAsset, SpeakerHandle);
	
	YAP_ASYNC_LOAD(DirectedAtAsset, DirectedAtHandle);
	
	UWorld* World = OwningContext->GetWorld();

	if (World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE))
	{
		EYapMaturitySetting MaturitySetting = UYapSubsystem::GetGameMaturitySetting();

		switch (MaturitySetting)
		{
			case EYapMaturitySetting::Mature:
			{
				YAP_ASYNC_LOAD(MatureAudioAsset, AudioAssetHandle);
				break;
			}
			case EYapMaturitySetting::ChildSafe:
			{
				if (bNeedsChildSafeData)
				{
					YAP_ASYNC_LOAD(SafeAudioAsset, AudioAssetHandle);
				}
				else
				{
					YAP_ASYNC_LOAD(MatureAudioAsset, AudioAssetHandle);
				}
				break;
			}
			default:
			{
				UE_LOG(LogYap, Warning, TEXT("Failed to preload audio, invalid maturity setting!"));
				break;
			}
		}

		// TODO I need some way for Yap to act upon the user changing their maturity setting. Broker needs an "OnMaturitySettingChanged" delegate
	}
}

// --------------------------------------------------------------------------------------------
// Protected

TOptional<float> FYapBit::GetTextTime(EYapMaturitySetting MaturitySetting) const
{
	ResolveMaturitySetting(MaturitySetting);

	int32 TWPM = UYapProjectSettings::GetTextWordsPerMinute();

	int32 WordCount = (MaturitySetting == EYapMaturitySetting::Mature) ? MatureDialogueText.WordCount() : SafeDialogueText.WordCount();

	double SecondsPerWord = 60.0 / (double)TWPM;
	
	double Min = UYapProjectSettings::GetMinimumAutoTextTimeLength();
	
	return FMath::Max(WordCount * SecondsPerWord, Min);
}

TOptional<float> FYapBit::GetAudioTime(EYapMaturitySetting MaturitySetting) const
{
	if (AudioAssetHandle.IsValid() && AudioAssetHandle->IsLoadingInProgress())
	{
		UE_LOG(LogYap, Warning, TEXT("Loading in progress...")); 
		return NullOpt;
	}
	
	const UObject* AudioAsset = GetAudioAsset<UObject>(MaturitySetting);

	if (!AudioAsset)
	{
		return NullOpt;
	}
	
	UYapBroker* Broker = nullptr;

#if WITH_EDITOR
	if (GEditor && GEditor->IsPlayingSessionInEditor())
	{
		Broker = UYapSubsystem::GetBroker();
	}
	else
	{
		// This is running at editor time only
		const TSoftClassPtr<UYapBroker>& BrokerClass = UYapProjectSettings::GetBrokerClass();
	
		if (BrokerClass.IsNull())
		{
			UE_LOG(LogYap, Warning, TEXT("No broker found in project settings! Cannot determine audio time!")); 
			return NullOpt;
		}
		
		if (!BrokerClass.IsNull())
		{
			Broker = BrokerClass.LoadSynchronous()->GetDefaultObject<UYapBroker>();
		}
	}
#else
	Broker = UYapSubsystem::GetBroker();
#endif

	if (!Broker)
	{
		UE_LOG(LogYap, Warning, TEXT("No broker found! Cannot determine audio time!")); 
		return NullOpt;
	}
		
	return Broker->GetAudioAssetDuration(AudioAsset);
}

// --------------------------------------------------------------------------------------------
// Public


FYapBit& FYapBit::operator=(const FYapBitReplacement& Replacement)
{
#define FLOWYAP_REPLACE(X) if (Replacement.X.IsSet()) {X = Replacement.X.GetValue(); }  

	FLOWYAP_REPLACE(SpeakerAsset);
	FLOWYAP_REPLACE(DirectedAtAsset);
	FLOWYAP_REPLACE(MatureTitleText);

	if (Replacement.bOverrideMatureDialogueText)
	{
		MatureDialogueText = Replacement.MatureDialogueText;
	}
	
	//FLOWYAP_REPLACE(MatureDialogueText);
	FLOWYAP_REPLACE(SafeTitleText);
	FLOWYAP_REPLACE(SafeDialogueText);
	FLOWYAP_REPLACE(MatureAudioAsset);
	FLOWYAP_REPLACE(SafeAudioAsset);
	FLOWYAP_REPLACE(MoodTag);
	FLOWYAP_REPLACE(TimeMode);
	FLOWYAP_REPLACE(ManualTime);
	
#undef FLOWYAP_REPLACE

	return *this;
}

// --------------------------------------------------------------------------------------------
// EDITOR API

#if WITH_EDITOR
void FYapBit::SetSpeaker(TSoftObjectPtr<UYapCharacter> InCharacter)
{
	SpeakerAsset = InCharacter;
	SpeakerHandle = nullptr;
}

void FYapBit::SetDirectedAt(TSoftObjectPtr<UYapCharacter> InDirectedAt)
{
	DirectedAtAsset = InDirectedAt;
	DirectedAtHandle = nullptr;
}

void FYapBit::SetMatureTitleText(const FText& NewText)
{
	MatureTitleText.Set(NewText);	
}
void FYapBit::SetSafeTitleText(const FText& NewText)
{
	SafeTitleText.Set(NewText);
}

void FYapBit::SetMatureDialogueText(const FText& NewText)
{
	MatureDialogueText.Set(NewText);
}

void FYapBit::SetSafeDialogueText(const FText& NewText)
{
	SafeDialogueText.Set(NewText);
}
#endif

#if WITH_EDITOR
bool FYapBit::HasDialogueTextForMaturity(EYapMaturitySetting MaturitySetting) const
{
	if (MaturitySetting == EYapMaturitySetting::Mature)
	{
		return !MatureDialogueText.Get().IsEmpty();
	}

	if (MaturitySetting == EYapMaturitySetting::ChildSafe)
	{
		return !SafeDialogueText.Get().IsEmpty();
	}

	return false;
}

void FYapBit::RecacheSpeakingTime()
{
	//RecalculateTextWordCount(MatureDialogueText, CachedMatureWordCount);
	//RecalculateTextWordCount(SafeDialogueText, CachedSafeWordCount);
}

void FYapBit::RecalculateAudioTime(TSoftObjectPtr<UObject>& AudioAsset, TOptional<float>& CachedTime)
{
	const TSoftClassPtr<UYapBroker>& BrokerClass = UYapProjectSettings::GetBrokerClass();
	
	if (BrokerClass.IsNull())
	{
		// TODO I need to handle the case of assigning lots of audio assets without time assigned - can I parse through all assets and fix up? Or log warnings during play so devs can go and fix them?
		UE_LOG(LogYap, Warning, TEXT("No audio time cache class found in project settings! Cannot set audio time!")); 
		CachedTime.Reset();
		return;
	}
	
	UYapBroker* BrokerCDO = BrokerClass.LoadSynchronous()->GetDefaultObject<UYapBroker>();

	float NewCachedTime = BrokerCDO->GetAudioAssetDuration(AudioAsset.LoadSynchronous());

	if (NewCachedTime > 0)
	{
		CachedTime = NewCachedTime;
	}
	else
	{
		CachedTime.Reset();
	}
}

void FYapBit::RecalculateTextWordCount(FText& Text, float& CachedWordCount)
{
	int32 WordCount = -1;

	if (UYapProjectSettings::CacheFragmentWordCount())
	{
		const UYapBroker* Broker = UYapProjectSettings::GetEditorBrokerDefault();

		if (IsValid(Broker))
		{
			WordCount = Broker->CalculateWordCount(Text);
		}
	}

	if (WordCount < 0)
	{
		UE_LOG(LogYap, Error, TEXT("Could not calculate word count!"));
	}

	CachedWordCount = WordCount;
}

#endif

#if WITH_EDITOR
void FYapBit::SetMatureDialogueAudioAsset(UObject* NewAudio)
{
	SetDialogueAudioAsset_Internal(MatureAudioAsset, NewAudio);
}
#endif

#if WITH_EDITOR
void FYapBit::SetSafeDialogueAudioAsset(UObject* NewAudio)
{
	SetDialogueAudioAsset_Internal(SafeAudioAsset, NewAudio);
}
#endif

#if WITH_EDITOR
void FYapBit::SetDialogueAudioAsset_Internal(TSoftObjectPtr<UObject>& AudioAsset, UObject* NewAudio)
{
	AudioAsset = NewAudio;
}
#endif

#undef LOCTEXT_NAMESPACE