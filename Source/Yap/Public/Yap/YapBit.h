// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once

#include "YapLog.h"
#include "YapTimeMode.h"
#include "GameplayTagContainer.h"
#include "Enums/YapMaturitySetting.h"
#include "Enums/YapWarnings.h"
#include "Yap/Enums/YapDialogueSkippable.h"

#include "YapBit.generated.h"

enum class EYapDialogueSkippable : uint8;
class UYapCharacter;
class UFlowNode_YapDialogue;
struct FYapBitReplacement;

USTRUCT()
struct YAP_API FYapBit
{
#if WITH_EDITOR
	friend class SFlowGraphNode_YapFragmentWidget;
#endif
	
	GENERATED_BODY()
	
	FYapBit();
	
	// --------------------------------------------------------------------------------------------
	// SETTINGS
protected:
	/**  */
	UPROPERTY()
	TSoftObjectPtr<UYapCharacter> SpeakerAsset;

	/**  */
	UPROPERTY()
	TSoftObjectPtr<UYapCharacter> DirectedAtAsset;
	
	/**  */
	UPROPERTY()
	FText MatureTitleText;

	/**  */
	UPROPERTY()
	FText SafeTitleText;
	
	/**  */
	UPROPERTY()
	FText MatureDialogueText;

	/**  */
	UPROPERTY()
	FText SafeDialogueText;
	
	/**  */
	UPROPERTY()
	TSoftObjectPtr<UObject> MatureDialogueAudioAsset;

	/**  */
	UPROPERTY()
	TSoftObjectPtr<UObject> SafeDialogueAudioAsset;

	/**  */
	UPROPERTY()
	FGameplayTag MoodKey;
		
	/**  */
	UPROPERTY()
	EYapTimeMode TimeMode = EYapTimeMode::Default;

	/**  */
	UPROPERTY()
	EYapDialogueSkippable Skippable = EYapDialogueSkippable::Default;
	
	// --------------------------------------------------------------------------------------------
	// SERIALIZED STATE FROM EDITOR
protected:
	UPROPERTY()
	float ManualTime = 0;
	
	UPROPERTY()
	int32 CachedMatureWordCount = 0;
	
	UPROPERTY()
	int32 CachedSafeWordCount = 0;
	
	UPROPERTY()
	TOptional<float> CachedMatureAudioTime;
	
	UPROPERTY()
	TOptional<float> CachedSafeAudioTime;

	
	// --------------------------------------------------------------------------------------------
	// STATE
protected:
	UPROPERTY(Transient)
	mutable TObjectPtr<UYapCharacter> Speaker;
	
	UPROPERTY(Transient)
	mutable TObjectPtr<UYapCharacter> DirectedAt;

	UPROPERTY(Transient)
	mutable TObjectPtr<UObject> DialogueAudio;
	
	UPROPERTY(Transient)
	mutable TObjectPtr<UObject> DialogueAudioSafe;

	// --------------------------------------------------------------------------------------------
	// PUBLIC API
public:
	const TSoftObjectPtr<UYapCharacter> GetSpeakerAsset() const { return SpeakerAsset; }
	
	const TSoftObjectPtr<UYapCharacter> GetDirectedAtAsset() const { return DirectedAtAsset; }

	const UYapCharacter* GetSpeaker(EYapWarnings Warnings = EYapWarnings::Show) const;

	const UYapCharacter* GetDirectedAt() const;

	/** Pass in a maturity setting, or leave as Undetermined and it will ask the subsystem for the current maturity setting. */
	const FText& GetDialogueText(EYapMaturitySetting/* = EYapMaturitySetting::Unspecified*/) const;

	/** Pass in a maturity setting, or leave as Undetermined and it will ask the subsystem for the current maturity setting. */
	const FText& GetTitleText(EYapMaturitySetting/* = EYapMaturitySetting::Unspecified*/) const;

	template<class T>
	const TSoftObjectPtr<T> GetDialogueAudioAsset_SoftPtr() const { return TSoftObjectPtr<T>(MatureDialogueAudioAsset->GetPathName()); }

	template<class T>
	const TSoftObjectPtr<T> GetSafeDialogueAudioAsset_SoftPtr() const { return TSoftObjectPtr<T>(SafeDialogueAudioAsset->GetPathName()); }
	
	template<class T>
	const T* GetAudioAsset(EYapMaturitySetting MaturitySetting = EYapMaturitySetting::Unspecified) const
	{
		ResolveMaturitySetting(MaturitySetting);
		
		if (MatureDialogueAudioAsset.IsValid())
		{
			return MatureDialogueAudioAsset.Get();
		}

		if (MatureDialogueAudioAsset.IsPending())
		{
			// TODO the main reason why I am doing this is because Epic's stupid property editor slate widget SObjectPropertyEntryBox can't display unloaded soft object ptr paths, it just displays "None"!
			UE_LOG(LogYap, Warning, TEXT("Synchronously loading dialogue audio asset. This should ONLY happen during editor time!"))
			return MatureDialogueAudioAsset.LoadSynchronous();
		}

		return nullptr;
	}
	
	/** If the maturity setting is unspecified, read it from either the Yap Subsystem or Project Defaults. */
	void ResolveMaturitySetting(EYapMaturitySetting& MaturitySetting) const;
	
	template<class T>
	const T* GetDialogueAudioAssetSafe() const
	{
		if (SafeDialogueAudioAsset.IsValid())
		{
			return SafeDialogueAudioAsset.Get();
		}

		if (SafeDialogueAudioAsset.IsPending())
		{
			// TODO the main reason why I am doing this is because Epic's stupid property editor slate widget SObjectPropertyEntryBox can't display unloaded soft object ptr paths, it just displays "None"!
			UE_LOG(LogYap, Warning, TEXT("Synchronously loading dialogue audio asset. This should ONLY happen during editor time!"))
			return SafeDialogueAudioAsset.LoadSynchronous();
		}

		return nullptr;
	}

	const FSlateBrush& GetSpeakerPortraitBrush() const;

	const FSlateBrush& GetDirectedAtPortraitBrush() const;

	bool HasAudioAsset() { return !MatureDialogueAudioAsset.IsNull(); }

	FGameplayTag GetMoodKey() const { return MoodKey; }

	/** Gets the evaluated skippable setting to be used for this bit (incorporating project default settings and fallbacks) */
	EYapDialogueSkippable GetSkippable(EYapMaturitySetting MaturitySetting = EYapMaturitySetting::Unspecified) const;

	/** Gets the evaluated time mode to be used for this bit (incorporating project default settings and fallbacks) */
	EYapTimeMode GetTimeMode(EYapMaturitySetting MaturitySetting = EYapMaturitySetting::Unspecified) const;
	
	/** Gets the evaluated time duration to be used for this bit (incorporating project default settings and fallbacks) */
	TOptional<float> GetTime(EYapMaturitySetting MaturitySetting = EYapMaturitySetting::Unspecified) const;
	
	void PreloadContent(UFlowNode_YapDialogue* OwningContext);
	
	void OnCharacterLoadComplete(TSoftObjectPtr<UYapCharacter>* CharacterAsset, TObjectPtr<UYapCharacter>* Character);

protected:
	TOptional<float> GetManualTime() const { return ManualTime; }

	float GetTextTime() const;

	TOptional<float> GetAudioTime() const;

public:
	FYapBit& operator=(const FYapBitReplacement& Replacement);

	// --------------------------------------------------------------------------------------------
	// EDITOR API
#if WITH_EDITOR
public:
	void SetSpeaker(TSoftObjectPtr<UYapCharacter> InCharacter);

	void SetDirectedAt(TSoftObjectPtr<UYapCharacter> InDirectedAt);

	void SetTitleText(FText* TextToSet, const FText& NewText);

	void SetDialogueText(FText* TextToSet, const FText& NewText);
	
	void SetDialogueAudioAsset(UObject* NewAudio);
	
	void SetDialogueAudioAssetSafe(UObject* NewAudio);
	
	void SetMoodKey(const FGameplayTag& NewValue) { MoodKey = NewValue; };

	EYapTimeMode GetBitTimeMode() const { return TimeMode; }

	void SetBitTimeMode(EYapTimeMode NewValue) { TimeMode = NewValue; }
	
	void SetManualTime(float NewValue) { ManualTime = NewValue; }

private:
	void SetDialogueText_Internal(FText* Text, const FText& InText);

	void SetDialogueAudioAsset_Internal(TSoftObjectPtr<UObject>& AudioAsset, TOptional<float>& CachedTime, UObject* NewAudio);

	const UYapCharacter* GetCharacterAsset_Internal(TSoftObjectPtr<UYapCharacter> CharacterAsset, TObjectPtr<UYapCharacter>& CharacterPtr, EYapWarnings Warnings = EYapWarnings::Show) const;
#endif
};