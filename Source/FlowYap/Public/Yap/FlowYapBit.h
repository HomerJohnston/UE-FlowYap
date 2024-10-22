#pragma once

#include "FlowYapLog.h"
#include "FlowYapTimeMode.h"
#include "GameplayTagContainer.h"

#include "FlowYapBit.generated.h"

class UFlowYapCharacter;
struct FFlowYapBitReplacement;

UENUM(BlueprintType)
enum class EFlowYapInterruptible : uint8
{
	UseProjectDefaults,
	NotInterruptible,
	Interruptible,
	MAX						UMETA(Hidden)
};

USTRUCT(BlueprintType)
struct FLOWYAP_API FFlowYapBit
{
	GENERATED_BODY()

	FFlowYapBit();

	friend class FPropertyCustomization_FlowYapFragment;
	
	// --------------------------------------------------------------------------------------------
	// SETTINGS
protected:
	UPROPERTY(BlueprintReadOnly)
	TSoftObjectPtr<UFlowYapCharacter> Character;
	
	UPROPERTY(BlueprintReadOnly, meta=(MultiLine=true))
	FText DialogueText;

	UPROPERTY(BlueprintReadOnly, meta=(MultiLine=true))
	FText TitleText;
	
	UPROPERTY(BlueprintReadOnly)
	TSoftObjectPtr<UObject> DialogueAudioAsset;

	UPROPERTY(BlueprintReadOnly)
	FGameplayTag MoodKey;
	
	UPROPERTY(BlueprintReadOnly)
	bool bUseProjectDefaultTimeSettings = true;
	
	UPROPERTY(BlueprintReadOnly, meta = (EditCondition = "!bUseProjectDefaultTimeSettings"))
	EFlowYapTimeMode TimeMode = EFlowYapTimeMode::AudioTime;

	// TODO is this variable in use? Should I display it?
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EFlowYapInterruptible Interruptible = EFlowYapInterruptible::UseProjectDefaults;
	
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
	const TSoftObjectPtr<UFlowYapCharacter> GetCharacterAsset() const { return Character; }
	
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
			UE_LOG(FlowYap, Warning, TEXT("Synchronously loading dialogue audio asset. This should ONLY happen during editor time!"))
			return DialogueAudioAsset.LoadSynchronous();
		}

		return nullptr;
	}

	const FSlateBrush* GetSpeakerPortraitBrush() const;

#if WITH_EDITOR
	FGameplayTag GetMoodKeyLazyInit();

	bool HasAudioAsset() { return !DialogueAudioAsset.IsNull(); }
#endif

	FGameplayTag GetMoodKey() const;

	/** Gets the evaluated interruptible setting to be used for this bit (incorporating project default settings and fallbacks) */
	bool GetInterruptible() const;

	/** Gets the evaluated time mode to be used for this bit (incorporating project default settings and fallbacks) */
	EFlowYapTimeMode GetTimeMode() const;
	
	/** Gets the evaluated time duration to be used for this bit (incorporating project default settings and fallbacks) */
	double GetTime() const;
	

protected:
	double GetManualTime() const { return ManualTime; }

	double GetTextTime() const;

	double GetAudioTime() const { return CachedAudioTime; }

public:
	FFlowYapBit& operator=(const FFlowYapBitReplacement& Replacement);
	
	// --------------------------------------------------------------------------------------------
	// EDITOR API
#if WITH_EDITOR
public:
	TSoftObjectPtr<UFlowYapCharacter> GetCharacterMutable() const { return Character; }
	
	void SetCharacter(TSoftObjectPtr<UFlowYapCharacter> InCharacter) { Character = InCharacter; }
	
	void SetTitleText(const FText& InText) { TitleText = InText; }
	
	void SetDialogueText(const FText& InText);
	
	void SetDialogueAudioAsset(UObject* InAsset);
	
	bool HasDialogueAudioAsset() const { return !DialogueAudioAsset.IsNull(); }
	
	void SetMoodKey(const FGameplayTag& NewValue) { MoodKey = NewValue; };

	bool GetUseProjectDefaultTimeSettings() const { return bUseProjectDefaultTimeSettings; }
	
	void SetUseProjectDefaultSettings(bool NewValue) { bUseProjectDefaultTimeSettings = NewValue; }
	
	EFlowYapTimeMode GetBitTimeMode() const { return TimeMode; }

	void SetBitTimeMode(EFlowYapTimeMode NewValue) { TimeMode = NewValue; }

	void SetBitInterruptible(EFlowYapInterruptible NewValue) { Interruptible = NewValue; }

	EFlowYapInterruptible GetBitInterruptible() const { return Interruptible; }
	
	void SetManualTime(double NewValue) { ManualTime = NewValue; }
#endif
};