// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once

#include "YapLog.h"
#include "YapTimeMode.h"
#include "GameplayTagContainer.h"
#include "YapGlobals.h"
#include "Enums/YapMaturitySetting.h"
#include "Enums/YapWarnings.h"
#include "Yap/Enums/YapDialogueProgressionFlags.h"

#include "YapBit.generated.h"

struct FStreamableHandle;
class UYapCharacter;
class UFlowNode_YapDialogue;
struct FYapBitReplacement;

#define LOCTEXT_NAMESPACE "Yap"

USTRUCT(BlueprintType)
struct YAP_API FYapText
{
#if WITH_EDITOR
	friend class SFlowGraphNode_YapFragmentWidget;
#endif
	
	GENERATED_BODY()

private:
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess))
	FText Txt;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess))
	int32 WordCnt = 0;

public:
	void Set(const FText& InText);

	const FText& Get() const { return Txt; }

	int32 WordCount() const { return WordCnt; }

	void operator=(const FYapText& Other)
	{
		Txt = Other.Txt;
		WordCnt = Other.WordCnt;
	}
	
	void operator=(const FText& NewText)
	{
		Set(NewText);
	}
};

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
	UPROPERTY()
	TSoftObjectPtr<UYapCharacter> SpeakerAsset;

	/**  */
	UPROPERTY()
	TSoftObjectPtr<UYapCharacter> DirectedAtAsset;
	
	/**  */
	UPROPERTY()
	FYapText MatureTitleText;

	/**  */
	UPROPERTY()
	FYapText SafeTitleText;
	
	/**  */
	UPROPERTY()
	FYapText MatureDialogueText;

	/**  */
	UPROPERTY()
	FYapText SafeDialogueText;
	
	/**  */
	UPROPERTY()
	TSoftObjectPtr<UObject> MatureAudioAsset;

	/**  */
	UPROPERTY()
	TSoftObjectPtr<UObject> SafeAudioAsset;

	/**  */
	UPROPERTY()
	FGameplayTag MoodTag;
		
	/**  */
	UPROPERTY()
	EYapTimeMode TimeMode = EYapTimeMode::Default;

	/**  */
	UPROPERTY()
	TOptional<bool> Skippable;
	
	/**  */
	UPROPERTY()
	TOptional<bool> AutoAdvance;
	
	UPROPERTY()
	float ManualTime = 0;
	
	/** Indicates whether child-safe data is available in this bit or not */
	UPROPERTY()
	bool bNeedsChildSafeData = false;

#if WITH_EDITORONLY_DATA
	// This is currently not in use. Still considering if I should use it or not.
	UPROPERTY(EditAnywhere)
	bool bIgnoreChildSafeErrors = false;
#endif
	
	// --------------------------------------------------------------------------------------------
	// STATE
protected:
	TSharedPtr<FStreamableHandle> SpeakerHandle;
	
	TSharedPtr<FStreamableHandle> DirectedAtHandle;

	TSharedPtr<FStreamableHandle> AudioAssetHandle;

	// --------------------------------------------------------------------------------------------
	// PUBLIC API
public:
	const TSoftObjectPtr<UYapCharacter> GetSpeakerAsset() const { return SpeakerAsset; }
	
	const TSoftObjectPtr<UYapCharacter> GetDirectedAtAsset() const { return DirectedAtAsset; }

	const UYapCharacter* GetSpeaker(EYapWarnings Warnings = EYapWarnings::Show); // Non-const because of async loading handle

	const UYapCharacter* GetDirectedAt(); // Non-const because of async loading handle
	
private:
	const UYapCharacter* GetCharacter_Internal(const TSoftObjectPtr<UYapCharacter>& CharacterAsset, TSharedPtr<FStreamableHandle>& Handle, EYapWarnings Warnings = EYapWarnings::Show);

public:
	/** Pass in a maturity setting, or leave as Undetermined and it will ask the subsystem for the current maturity setting. */
	const FText& GetDialogueText(EYapMaturitySetting MaturitySetting) const;
	
	/** Pass in a maturity setting, or leave as Undetermined and it will ask the subsystem for the current maturity setting. */
	const FText& GetTitleText(EYapMaturitySetting MaturitySetting) const;

	template<class T>
	const TSoftObjectPtr<T> GetMatureDialogueAudioAsset_SoftPtr() const { return TSoftObjectPtr<T>(MatureAudioAsset->GetPathName()); }

	template<class T>
	const TSoftObjectPtr<T> GetSafeDialogueAudioAsset_SoftPtr() const { return TSoftObjectPtr<T>(SafeAudioAsset->GetPathName()); }
	
	template<class T>
	const T* GetAudioAsset(EYapMaturitySetting MaturitySetting = EYapMaturitySetting::Unspecified);

	/** If the maturity setting is unspecified, read it from either the Yap Subsystem or Project Defaults. */
	void ResolveMaturitySetting(EYapMaturitySetting& MaturitySetting) const;

	bool HasAudioAsset(EYapMaturitySetting MaturitySetting = EYapMaturitySetting::Unspecified) const;

	FGameplayTag GetMoodTag() const { return MoodTag; }

	/** Gets the evaluated skippable setting to be used for this bit (incorporating project default settings and fallbacks) */
	bool GetSkippable(const UFlowNode_YapDialogue* Owner) const;

	bool GetAutoAdvance(const UFlowNode_YapDialogue* Owner) const;
	
	/** Gets the evaluated time mode to be used for this bit (incorporating project default settings and fallbacks) */
	EYapTimeMode GetTimeMode(EYapMaturitySetting MaturitySetting = EYapMaturitySetting::Unspecified) const;

	bool IsTimeModeNone() const { return TimeMode == EYapTimeMode::None; }
	
	void PreloadContent(UFlowNode_YapDialogue* OwningContext);
	
	bool NeedsChildSafeData() const { return bNeedsChildSafeData; };

public:
	/** Gets the evaluated time duration to be used for this bit (incorporating project default settings and fallbacks) */
	TOptional<float> GetTime(EYapMaturitySetting MaturitySetting = EYapMaturitySetting::Unspecified);
	
protected:
	TOptional<float> GetManualTime() const { return ManualTime; }

	TOptional<float> GetTextTime(EYapMaturitySetting MaturitySetting = EYapMaturitySetting::Unspecified);

	TOptional<float> GetAudioTime(EYapMaturitySetting MaturitySetting = EYapMaturitySetting::Unspecified);

public:
	FYapBit& operator=(const FYapBitReplacement& Replacement);

	// --------------------------------------------------------------------------------------------
	// EDITOR API
#if WITH_EDITOR
public:
	void SetSpeaker(TSoftObjectPtr<UYapCharacter> InCharacter);
	void SetDirectedAt(TSoftObjectPtr<UYapCharacter> InDirectedAt);

	void SetMatureTitleText(const FText& NewText);
	void SetSafeTitleText(const FText& NewText);

	void SetMatureDialogueText(const FText& NewText);
	void SetSafeDialogueText(const FText& NewText);
	
	void SetMatureDialogueAudioAsset(UObject* NewAudio);
	void SetSafeDialogueAudioAsset(UObject* NewAudio);
	
	void SetMoodTag(const FGameplayTag& NewValue) { MoodTag = NewValue; };

	void SetTimeModeSetting(EYapTimeMode NewValue) { TimeMode = NewValue; }
	
	void SetManualTime(float NewValue) { ManualTime = NewValue; }

	TOptional<bool> GetSkippableSetting() const { return Skippable; }
	TOptional<bool>& GetSkippableSetting() { return Skippable; }
	TOptional<bool> GetAutoAdvanceSetting() const { return AutoAdvance; }
	TOptional<bool>& GetAutoAdvanceSetting() { return AutoAdvance; }
	
	EYapTimeMode GetTimeModeSetting() const { return TimeMode; }

	bool HasDialogueTextForMaturity(EYapMaturitySetting MaturitySetting) const;

private:
	void SetTextData_Internal(FYapText& TextToSet, const FText& NewText);

	void SetDialogueAudioAsset_Internal(TSoftObjectPtr<UObject>& AudioAsset, UObject* NewAudio);
	
	void RecacheSpeakingTime();

	void RecalculateTextWordCount(FText& Text, float& CachedTime);

	void RecalculateAudioTime(TSoftObjectPtr<UObject>& AudioAsset, TOptional<float>& CachedTime);
#endif
};

template <class T>
const T* FYapBit::GetAudioAsset(EYapMaturitySetting MaturitySetting)
{
	ResolveMaturitySetting(MaturitySetting);

	const TSoftObjectPtr<UObject>& Asset = MaturitySetting == EYapMaturitySetting::Mature ? MatureAudioAsset : SafeAudioAsset;
	
	// Asset is unset. Return nothing.
	if (Asset.IsNull())
	{
		return nullptr;
	}

#if WITH_EDITOR
	if (Asset.IsPending())
	{
		// This is a bit of a dumb hack.
		if (GEditor->PlayWorld && (GEditor->PlayWorld->WorldType == EWorldType::Game || GEditor->PlayWorld->WorldType == EWorldType::PIE))
		{
			UE_LOG(LogYap, Warning, TEXT("Synchronously loading dialogue audio asset."))

			Yap::PostNotificationInfo_Warning
			(
				LOCTEXT("SyncLoadAudioWarning_Title", "Sync Loading Audio"),
				LOCTEXT("SyncLoadAudioWarning_Description", "Synchronously loading dialogue audio asset.")
			);
		}
	}
#endif

	return Asset.LoadSynchronous();
}

#undef LOCTEXT_NAMESPACE