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
	TSoftObjectPtr<UObject> MatureAudioAsset;

	/**  */
	UPROPERTY()
	TSoftObjectPtr<UObject> SafeAudioAsset;

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

	/** Indicates whether child-safe data is available in this bit or not */
	UPROPERTY()
	bool bNeedsChildSafeData = false;

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere)
	bool bIgnoreChildSafeErrors = false;
#endif
	
	// --------------------------------------------------------------------------------------------
	// STATE
protected:
	UPROPERTY(Transient)
	mutable TObjectPtr<UYapCharacter> Speaker;
	
	UPROPERTY(Transient)
	mutable TObjectPtr<UYapCharacter> DirectedAt;

	UPROPERTY(Transient)
	mutable TObjectPtr<UObject> MatureAudio;
	
	UPROPERTY(Transient)
	mutable TObjectPtr<UObject> SafeAudio;

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
	const TSoftObjectPtr<T> GetMatureDialogueAudioAsset_SoftPtr() const { return TSoftObjectPtr<T>(MatureAudioAsset->GetPathName()); }

	template<class T>
	const TSoftObjectPtr<T> GetSafeDialogueAudioAsset_SoftPtr() const { return TSoftObjectPtr<T>(SafeAudioAsset->GetPathName()); }
	
	template<class T>
	const T* GetAudioAsset(EYapMaturitySetting MaturitySetting = EYapMaturitySetting::Unspecified) const;

	/** If the maturity setting is unspecified, read it from either the Yap Subsystem or Project Defaults. */
	void ResolveMaturitySetting(EYapMaturitySetting& MaturitySetting) const;
	
	const FSlateBrush& GetSpeakerPortraitBrush() const;

	const FSlateBrush& GetDirectedAtPortraitBrush() const;

	bool HasAudioAsset() { return !MatureAudioAsset.IsNull(); }

	FGameplayTag GetMoodKey() const { return MoodKey; }

	/** Gets the evaluated skippable setting to be used for this bit (incorporating project default settings and fallbacks) */
	bool GetSkippable(const UFlowNode_YapDialogue* Owner) const;

	EYapDialogueSkippable GetSkippableSetting() const { return Skippable; }
	
	/** Gets the evaluated time mode to be used for this bit (incorporating project default settings and fallbacks) */
	EYapTimeMode GetTimeMode(EYapMaturitySetting MaturitySetting = EYapMaturitySetting::Unspecified) const;
	
	/** Gets the evaluated time duration to be used for this bit (incorporating project default settings and fallbacks) */
	TOptional<float> GetTime(EYapMaturitySetting MaturitySetting = EYapMaturitySetting::Unspecified) const;
	
	void PreloadContent(UFlowNode_YapDialogue* OwningContext);
	
	void OnCharacterLoadComplete(TSoftObjectPtr<UYapCharacter>* CharacterAsset, TObjectPtr<UYapCharacter>* Character);

	bool NeedsChildSafeData() const { return bNeedsChildSafeData; };

protected:
	TOptional<float> GetManualTime() const { return ManualTime; }

	float GetTextTime(EYapMaturitySetting MaturitySetting = EYapMaturitySetting::Unspecified) const;

	TOptional<float> GetAudioTime(EYapMaturitySetting MaturitySetting = EYapMaturitySetting::Unspecified) const;

public:
	FYapBit& operator=(const FYapBitReplacement& Replacement);

	// --------------------------------------------------------------------------------------------
	// EDITOR API
#if WITH_EDITOR
public:
	void SetSpeaker(TSoftObjectPtr<UYapCharacter> InCharacter);

	void SetDirectedAt(TSoftObjectPtr<UYapCharacter> InDirectedAt);

	void SetTitleText(FText* TextToSet, const FText& NewText);

	void SetTextData(FText* TextToSet, const FText& NewText);
	
	void SetMatureDialogueAudioAsset(UObject* NewAudio);
	
	void SetSafeDialogueAudioAsset(UObject* NewAudio);
	
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

template <class T>
const T* FYapBit::GetAudioAsset(EYapMaturitySetting MaturitySetting) const
{
	ResolveMaturitySetting(MaturitySetting);

	const TSoftObjectPtr<UObject>& PreferredAsset = MaturitySetting == EYapMaturitySetting::Mature ? MatureAudioAsset : SafeAudioAsset;
	const TSoftObjectPtr<UObject>& SecondaryAsset = MaturitySetting == EYapMaturitySetting::Mature ? SafeAudioAsset : MatureAudioAsset;

	if (PreferredAsset.IsNull() && SecondaryAsset.IsNull())
	{
		return nullptr;
	}
		
	TObjectPtr<UObject>& PreferredAudio = MaturitySetting == EYapMaturitySetting::Mature ? MatureAudio : SafeAudio; 
	TObjectPtr<UObject>& SecondaryAudio = MaturitySetting == EYapMaturitySetting::Mature ? SafeAudio : MatureAudio; 

	if (PreferredAsset.IsValid())
	{
		return PreferredAsset.Get();
	}

	if (!PreferredAsset.IsNull())
	{
#if WITH_EDITOR
		if (GEditor->PlayWorld)
		{
			UE_LOG(LogYap, Warning, TEXT("Synchronously loading dialogue audio asset."))
		}
#endif
		PreferredAudio = PreferredAsset.LoadSynchronous();
		return  PreferredAudio;
	}

	// Don't allow falling back to child-safe data if it is 
	if (MaturitySetting == EYapMaturitySetting::Mature && !bNeedsChildSafeData)
	{
		return nullptr;
	}
	
	if (SecondaryAsset.IsValid())
	{
		return SecondaryAsset.Get();
	}

#if WITH_EDITOR
	if (GEditor->PlayWorld)
	{
		UE_LOG(LogYap, Warning, TEXT("Synchronously loading dialogue audio asset."))
	}
#endif
	SecondaryAudio = SecondaryAsset.LoadSynchronous();
	return SecondaryAudio;
}
