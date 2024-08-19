#pragma once

#include "FlowYapTimeMode.h"

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
	
	// --------------------------------------------------------------------------------------------
	// SETTINGS
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UFlowYapCharacter> Character;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(MultiLine=true))
	FText TitleText;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(MultiLine=true))
	FText DialogueText;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UObject> DialogueAudioAsset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName MoodKey = NAME_None;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bUseProjectDefaultTimeSettings = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "!bUseProjectDefaultTimeSettings"))
	EFlowYapTimeMode TimeMode = EFlowYapTimeMode::AudioTime;

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

	template<class T>
	const TSoftObjectPtr<T> GetDialogueAudioAsset() const { return TSoftObjectPtr<T>(DialogueAudioAsset->GetPathName()); }

	const FSlateBrush* GetSpeakerPortraitBrush() const;; // TODO make sure this works

#if WITH_EDITOR
	FName GetMoodKeyLazyInit();
#endif

	FName GetMoodKey() const;

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
	
	void SetMoodKey(const FName& NewValue) { MoodKey = NewValue; };

	bool GetUseProjectDefaultTimeSettings() const { return bUseProjectDefaultTimeSettings; }
	
	void SetUseProjectDefaultSettings(bool NewValue) { bUseProjectDefaultTimeSettings = NewValue; }
	
	EFlowYapTimeMode GetBitTimeMode() const { return TimeMode; }

	void SetBitTimeMode(EFlowYapTimeMode NewValue) { TimeMode = NewValue; }

	void SetBitInterruptible(EFlowYapInterruptible NewValue) { Interruptible = NewValue; }

	EFlowYapInterruptible GetBitInterruptible() const { return Interruptible; }
	
	void SetManualTime(double NewValue) { ManualTime = NewValue; }
#endif
};