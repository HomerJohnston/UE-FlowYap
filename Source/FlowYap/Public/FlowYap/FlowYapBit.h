#pragma once

#include "FlowYapTimeMode.h"

#include "FlowYapBit.generated.h"

USTRUCT()
struct FFlowYapBit
{
	GENERATED_BODY()

	// --------------------------------------------------------------------------------------------
	// SETTINGS
protected:
	UPROPERTY(EditAnywhere, meta=(MultiLine=true))
	FText TitleText;
	
	UPROPERTY(EditAnywhere, meta=(MultiLine=true))
	FText DialogueText;

	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<UObject> DialogueAudioAsset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName PortraitKey = NAME_None;
	
	UPROPERTY(EditAnywhere)
	bool bUseProjectDefaultTimeSettings = true;
	
	UPROPERTY(EditAnywhere, meta = (EditCondition = "!bUseProjectDefaultTimeSettings"))
	EFlowYapTimeMode TimeMode = EFlowYapTimeMode::AudioLength;

	UPROPERTY(EditAnywhere)
	bool bInterruptible = true;
	
	// --------------------------------------------------------------------------------------------
	// SERIALIZED STATE FROM EDITOR
protected:
	UPROPERTY(VisibleAnywhere, meta = (Units = "s"))
	double ManualTime = 0;
	
	UPROPERTY(VisibleAnywhere)
	int32 CachedWordCount = 0;
	
	UPROPERTY(VisibleAnywhere, meta = (Units = "s"))
	double CachedAudioTime = 0;
	
	// --------------------------------------------------------------------------------------------
	// PUBLIC API
public:
	const FText& GetTitleText() const { return TitleText; }

	const FText& GetDialogueText() const { return DialogueText; }

	template<class T>
	const TSoftObjectPtr<T> GetDialogueAudioAsset() const { return TSoftObjectPtr<T>(DialogueAudioAsset->GetPathName()); }; // TODO make sure this works

	FName GetPortraitKey() const { return PortraitKey; }
	
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
	
	// --------------------------------------------------------------------------------------------
	// EDITOR API
#if WITH_EDITOR
public:
	void SetTitleText(const FText& InText) { TitleText = InText; }

	void SetDialogueText(const FText& InText);

	void SetDialogueAudioAsset(UObject* InAsset);

	bool HasDialogueAudioAsset() const { return !DialogueAudioAsset.IsNull(); }
	
	void SetPortraitKey(const FName& NewValue) { PortraitKey = NewValue; };

	bool GetUseProjectDefaultTimeSettings() const { return bUseProjectDefaultTimeSettings; }
	
	void SetUseProjectDefaultSettings(bool NewValue) { bUseProjectDefaultTimeSettings = NewValue; }
	
	EFlowYapTimeMode GetBitTimeMode() const { return TimeMode; }

	void SetBitTimeMode(EFlowYapTimeMode NewValue) { TimeMode = NewValue; }

	void SetBitInterruptible(bool NewValue) { bInterruptible = NewValue; }
	
	void SetManualTime(double NewValue) { ManualTime = NewValue; }
#endif
};