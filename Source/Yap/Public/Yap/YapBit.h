// Copyright Ghost Pepper Games Inc., all rights reserved.

#pragma once

#include "YapLog.h"
#include "YapTimeMode.h"
#include "GameplayTagContainer.h"
#include "YapBit.generated.h"

class UYapCharacter;
struct FYapBitReplacement;

USTRUCT(BlueprintType)
struct YAP_API FYapBit
{
	GENERATED_BODY()

	FYapBit();
	
	// --------------------------------------------------------------------------------------------
	// SETTINGS
protected:
	UPROPERTY(BlueprintReadOnly)
	TSoftObjectPtr<UYapCharacter> Character;
	
	UPROPERTY(BlueprintReadOnly, meta=(MultiLine=true))
	FText DialogueText;

	UPROPERTY(BlueprintReadOnly, meta=(MultiLine=true))
	FText TitleText;
	
	UPROPERTY(BlueprintReadOnly)
	TSoftObjectPtr<UObject> DialogueAudioAsset;

	UPROPERTY(BlueprintReadOnly, meta = (Yap))
	FGameplayTag MoodKey;
	
	UPROPERTY(BlueprintReadOnly)
	bool bUseProjectDefaultTimeSettings = true;
	
	UPROPERTY(BlueprintReadOnly, meta = (EditCondition = "!bUseProjectDefaultTimeSettings"))
	EYapTimeMode TimeMode = EYapTimeMode::AudioTime;

	// --------------------------------------------------------------------------------------------
	// SERIALIZED STATE FROM EDITOR
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	double ManualTime = 0;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 CachedWordCount = 0;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	double CachedAudioTime = 0;
	
	// --------------------------------------------------------------------------------------------
	// PUBLIC API
public:
	const TSoftObjectPtr<UYapCharacter> GetCharacterAsset() const { return Character; }
	
	const FText& GetTitleText() const { return TitleText; }

	const FText& GetDialogueText() const { return DialogueText; }

	FText& GetDialogueTextMutable() { return DialogueText; }
	
	FText& GetTitleTextMutable() { return TitleText; }

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

	const FSlateBrush& GetSpeakerPortraitBrush() const;

	bool HasAudioAsset() { return !DialogueAudioAsset.IsNull(); }

	FGameplayTag GetMoodKey(bool bReturnDefault = true) const;

	/** Gets the evaluated interruptible setting to be used for this bit (incorporating project default settings and fallbacks) */
	bool GetInterruptible() const;

	/** Gets the evaluated time mode to be used for this bit (incorporating project default settings and fallbacks) */
	EYapTimeMode GetTimeMode() const;
	
	/** Gets the evaluated time duration to be used for this bit (incorporating project default settings and fallbacks) */
	double GetTime() const;
	

protected:
	double GetManualTime() const { return ManualTime; }

	double GetTextTime() const;

	double GetAudioTime() const { return CachedAudioTime; }

public:
	bool HasDialogueAudioAsset() const { return !DialogueAudioAsset.IsNull(); }

public:
	FYapBit& operator=(const FYapBitReplacement& Replacement);


	// --------------------------------------------------------------------------------------------
	// EDITOR API
#if WITH_EDITOR
public:
	TSoftObjectPtr<UYapCharacter> GetCharacterMutable() const { return Character; }
	
	void SetCharacter(TSoftObjectPtr<UYapCharacter> InCharacter) { Character = InCharacter; }
	
	void SetTitleText(const FText& InText) { TitleText = InText; }
	
	void SetDialogueText(const FText& InText);
	
	void SetDialogueAudioAsset(UObject* InAsset);
	
	void SetMoodKey(const FGameplayTag& NewValue) { MoodKey = NewValue; };

	bool GetUseProjectDefaultTimeSettings() const { return bUseProjectDefaultTimeSettings; }
	
	void SetUseProjectDefaultSettings(bool NewValue) { bUseProjectDefaultTimeSettings = NewValue; }
	
	EYapTimeMode GetBitTimeMode() const { return TimeMode; }

	void SetBitTimeMode(EYapTimeMode NewValue) { TimeMode = NewValue; }
	
	void SetManualTime(double NewValue) { ManualTime = NewValue; }
#endif

};