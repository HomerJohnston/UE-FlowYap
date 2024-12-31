// Copyright Ghost Pepper Games Inc., all rights reserved.

#pragma once

#include "YapLog.h"
#include "YapTimeMode.h"
#include "GameplayTagContainer.h"
#include "Yap/Enums/YapDialogueSkippable.h"

#include "YapBit.generated.h"

enum class EYapDialogueSkippable : uint8;
class UYapCharacter;
class UFlowNode_YapDialogue;
struct FYapBitReplacement;

USTRUCT(BlueprintType)
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
	UPROPERTY(BlueprintReadOnly)
	TSoftObjectPtr<UYapCharacter> SpeakerAsset;

	/**  */
	UPROPERTY(BlueprintReadOnly)
	TSoftObjectPtr<UYapCharacter> DirectedAtAsset;
	/**  */
	UPROPERTY(BlueprintReadOnly)
	FText DialogueText;

	/**  */
	UPROPERTY(BlueprintReadOnly)
	FText DialogueTextSafe;

	/**  */
	UPROPERTY(BlueprintReadOnly)
	FText TitleText;

	/**  */
	UPROPERTY(BlueprintReadOnly)
	FText TitleTextSafe;
	
	/**  */
	UPROPERTY(BlueprintReadOnly)
	TSoftObjectPtr<UObject> DialogueAudioAsset;

	/**  */
	UPROPERTY(BlueprintReadOnly)
	TSoftObjectPtr<UObject> DialogueAudioAssetSafe;

	/**  */
	UPROPERTY(BlueprintReadOnly)
	FGameplayTag MoodKey;
	
	/**  */
	UPROPERTY(BlueprintReadOnly)
	bool bUseProjectDefaultTimeSettings = true;
	
	/**  */
	UPROPERTY(BlueprintReadOnly)
	EYapTimeMode TimeMode = EYapTimeMode::AudioTime;

	/**  */
	UPROPERTY(BlueprintReadOnly)
	EYapDialogueSkippable Skippable = EYapDialogueSkippable::Default;
	
	// --------------------------------------------------------------------------------------------
	// SERIALIZED STATE FROM EDITOR
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	double ManualTime = 0;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 CachedWordCount = 0;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 CachedWordCountSafe = 0;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	double CachedAudioTime = 0;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	double CachedAudioTimeSafe = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	double CachedSafeAudioTime = 0;
	
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
protected:
	const TSoftObjectPtr<UYapCharacter> GetSpeakerAsset() const { return SpeakerAsset; }
	
	const TSoftObjectPtr<UYapCharacter> GetDirectedAtAsset() const { return DirectedAtAsset; }

public:
	const UYapCharacter* GetSpeaker() const;

	const UYapCharacter* GetDirectedAt() const;
	
	const FText& GetTitleText() const { return TitleText; }
	
	const FText& GetTitleTextSafe() const { return TitleTextSafe; }

	const FText& GetDialogueText() const { return DialogueText; }
	
	const FText& GetDialogueTextSafe() const { return DialogueTextSafe; }

	/**  */
	const FText& GetSpokenText(bool bUseChildSafeText) const;

	template<class T>
	const TSoftObjectPtr<T> GetDialogueAudioAsset_SoftPtr() const { return TSoftObjectPtr<T>(DialogueAudioAsset->GetPathName()); }

	template<class T>
	const T* GetDialogueAudioAsset() const
	{
		if (DialogueAudioAsset.IsValid())
		{
			return DialogueAudioAsset.Get();
		}

		if (DialogueAudioAsset.IsPending())
		{
			// TODO the main reason why I am doing this is because Epic's stupid property editor slate widget SObjectPropertyEntryBox can't display unloaded soft object ptr paths, it just displays "None"!
			UE_LOG(LogYap, Warning, TEXT("Synchronously loading dialogue audio asset. This should ONLY happen during editor time!"))
			return DialogueAudioAsset.LoadSynchronous();
		}

		return nullptr;
	}
	
	template<class T>
	const T* GetDialogueAudioAssetSafe() const
	{
		if (DialogueAudioAssetSafe.IsValid())
		{
			return DialogueAudioAssetSafe.Get();
		}

		if (DialogueAudioAssetSafe.IsPending())
		{
			// TODO the main reason why I am doing this is because Epic's stupid property editor slate widget SObjectPropertyEntryBox can't display unloaded soft object ptr paths, it just displays "None"!
			UE_LOG(LogYap, Warning, TEXT("Synchronously loading dialogue audio asset. This should ONLY happen during editor time!"))
			return DialogueAudioAssetSafe.LoadSynchronous();
		}

		return nullptr;
	}

	const FSlateBrush& GetSpeakerPortraitBrush() const;

	bool HasAudioAsset() { return !DialogueAudioAsset.IsNull(); }

	FGameplayTag GetMoodKey() const { return MoodKey; }

	/** Gets the evaluated skippable setting to be used for this bit (incorporating project default settings and fallbacks) */
	EYapDialogueSkippable GetSkippable() const { return Skippable; }

	/** Gets the evaluated time mode to be used for this bit (incorporating project default settings and fallbacks) */
	EYapTimeMode GetTimeMode() const;
	
	/** Gets the evaluated time duration to be used for this bit (incorporating project default settings and fallbacks) */
	double GetTime() const;
	
	void PreloadContent(UFlowNode_YapDialogue* OwningContext);
	
	void OnCharacterLoadComplete(TSoftObjectPtr<UYapCharacter>* CharacterAsset, TObjectPtr<UYapCharacter>* Character);
	
protected:
	double GetManualTime() const { return ManualTime; }

	double GetTextTime() const;

	double GetAudioTime() const { return CachedAudioTime; }

public:
	bool HasDialogueAudioAsset() const { return !DialogueAudioAsset.IsNull(); }
	
	bool HasDialogueAudioAssetSafe() const { return !DialogueAudioAssetSafe.IsNull(); }

public:
	FYapBit& operator=(const FYapBitReplacement& Replacement);

	// --------------------------------------------------------------------------------------------
	// EDITOR API
#if WITH_EDITOR
public:
	TSoftObjectPtr<UYapCharacter> GetCharacterMutable() const { return SpeakerAsset; }
	
	void SetCharacter(TSoftObjectPtr<UYapCharacter> InCharacter);

	void SetTitleText(const FText& InText) { TitleText = InText; }

	void SetTitleTextSafe(const FText& InText) { TitleTextSafe = InText; }
	
	void SetDialogueText(const FText& InText);

	void SetDialogueTextSafe(const FText& InText);
	
	void SetDialogueAudioAsset(UObject* NewAudio);
	
	void SetDialogueAudioAssetSafe(UObject* NewAudio);
	
	void SetMoodKey(const FGameplayTag& NewValue) { MoodKey = NewValue; };

	bool GetUseProjectDefaultTimeSettings() const { return bUseProjectDefaultTimeSettings; }
	
	void SetUseProjectDefaultSettings(bool NewValue) { bUseProjectDefaultTimeSettings = NewValue; }
	
	EYapTimeMode GetBitTimeMode() const { return TimeMode; }

	void SetBitTimeMode(EYapTimeMode NewValue) { TimeMode = NewValue; }
	
	void SetManualTime(double NewValue) { ManualTime = NewValue; }

private:
	void SetDialogueText_Internal(FText& Text, int32& WordCount, const FText& InText);

	void SetDialogueAudioAsset_Internal(TSoftObjectPtr<UObject>& AudioAsset, double& CachedTime, UObject* NewAudio);

	const UYapCharacter* GetCharacterAsset_Internal(TSoftObjectPtr<UYapCharacter> CharacterAsset, TObjectPtr<UYapCharacter>& CharacterPtr) const;
#endif
};