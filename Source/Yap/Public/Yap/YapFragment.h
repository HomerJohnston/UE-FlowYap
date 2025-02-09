// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once
#include "YapBit.h"
#include "GameplayTagContainer.h"
#include "Nodes/FlowPin.h"

#include "YapFragment.generated.h"

enum class EYapLoadContext : uint8;
class UYapCharacter;
class UYapCondition;
class UFlowNode_YapDialogue;
struct FFlowPin;
enum class EYapMaturitySetting : uint8;

USTRUCT(NotBlueprintType)
struct YAP_API FYapFragment
{
	GENERATED_BODY()

public:
	FYapFragment();
	
	bool CheckConditions() const;
	void ResetOptionalPins();
	void PreloadContent(EYapMaturitySetting MaturitySetting, EYapLoadContext LoadContext);

#if WITH_EDITOR
	friend class SFlowGraphNode_YapDialogueWidget;
	friend class SFlowGraphNode_YapFragmentWidget;
	friend class UFlowGraphNode_YapDialogue;
#endif
	
	// ==========================================
	// SETTINGS
protected:
	UPROPERTY()
	TArray<TObjectPtr<UYapCondition>> Conditions;
	
	/**  */
	UPROPERTY()
	TSoftObjectPtr<UYapCharacter> SpeakerAsset;

	/**  */
	UPROPERTY()
	TSoftObjectPtr<UYapCharacter> DirectedAtAsset;
	
	UPROPERTY()
	FYapBit MatureBit;

	UPROPERTY()
	FYapBit ChildSafeBit;
	
	/** How many times is this fragment allowed to broadcast? This count persists only within this flow asset's lifespan (resets every Start). */
	UPROPERTY()
	int32 ActivationLimit = 0;
	
	UPROPERTY()
	FGameplayTag FragmentTag;

	/** Padding is idle time to wait after the fragment finishes running. A value of -1 will use project defaults. */
	UPROPERTY()
	float PaddingToNextFragment = -1;
	
	/**  */
	UPROPERTY()
	TOptional<bool> Skippable;
	
	/**  */
	UPROPERTY()
	TOptional<bool> AutoAdvance;
	
	/** Indicates whether child-safe data is available in this bit or not */
	UPROPERTY()
	bool bEnableChildSafe = false;
	
	UPROPERTY()
	bool bShowOnStartPin = false;

	UPROPERTY()
	bool bShowOnEndPin = false;
	
	/**  */
	UPROPERTY()
	FGameplayTag MoodTag; // TODO - what about sending multiple mood tags? Or some other way to specify more arbitrary data? UPropertyBag???
		
	/**  */
	UPROPERTY()
	EYapTimeMode TimeMode;
	
	// ==========================================
	// STATE
protected:

	UPROPERTY(VisibleAnywhere, meta=(IgnoreForMemberInitializationTest))
	FGuid Guid;
	
	// TODO should this be serialized or transient
	UPROPERTY(Transient)
	uint8 IndexInDialogue = 0; 

	UPROPERTY(Transient)
	int32 ActivationCount = 0;

	UPROPERTY()
	FFlowPin PromptPin;

	UPROPERTY()
	FFlowPin StartPin;

	UPROPERTY()
	FFlowPin EndPin;

	// ASSET LOADING
protected:
	TSharedPtr<FStreamableHandle> SpeakerHandle;
	
	TSharedPtr<FStreamableHandle> DirectedAtHandle;
	
	// ==========================================
	// API
public:
	const UYapCharacter* GetSpeaker(EYapLoadContext LoadContext); // Non-const because of async loading handle

	const UYapCharacter* GetDirectedAt(EYapLoadContext LoadContext); // Non-const because of async loading handle
	
private:
	const UYapCharacter* GetCharacter_Internal(const TSoftObjectPtr<UYapCharacter>& CharacterAsset, TSharedPtr<FStreamableHandle>& Handle, EYapLoadContext LoadContext);

public:
	// TODO I don't think fragments should know where their position is!
	uint8 GetIndexInDialogue() const { return IndexInDialogue; }
	
	int32 GetActivationCount() const { return ActivationCount; }
	
	int32 GetActivationLimit() const { return ActivationLimit; }

	bool IsActivationLimitMet() const { if (ActivationLimit <= 0) return false; return (ActivationCount >= ActivationLimit); }

	const FText& GetDialogueText(EYapMaturitySetting MaturitySetting) const;
	
	const FText& GetTitleText(EYapMaturitySetting MaturitySetting) const;
	
	const UObject* GetAudioAsset(EYapMaturitySetting MaturitySetting) const;
	
	const FYapBit& GetBit() const;

	const FYapBit& GetBit(EYapMaturitySetting MaturitySetting) const;

	const FYapBit& GetMatureBit() const { return MatureBit; }

	const FYapBit& GetChildSafeBit() const { return ChildSafeBit; }

	FYapBit& GetMatureBitMutable() { return MatureBit; }

	FYapBit& GetChildSafeBitMutable() { return ChildSafeBit; }

	TOptional<float> GetTime() const;

protected:
	TOptional<float> GetTime(EYapMaturitySetting MaturitySetting, EYapLoadContext LoadContext) const;

public:
	float GetPaddingToNextFragment() const;

	void IncrementActivations();

	const FGameplayTag& GetFragmentTag() const { return FragmentTag; } 

	// TODO - want to design a better system for this.
	//void ReplaceBit(EYapMaturitySetting MaturitySetting, const FYapBitReplacement& ReplacementBit);

	const FGuid& GetGuid() const { return Guid; }

	bool UsesStartPin() const { return bShowOnStartPin; }

	bool UsesEndPin() const { return bShowOnEndPin; }

	const TArray<UYapCondition*>& GetConditions() const { return Conditions; }

	const TSoftObjectPtr<UYapCharacter>& GetSpeakerAsset() const { return SpeakerAsset; }
	
	const TSoftObjectPtr<UYapCharacter>& GetDirectedAtAsset() const { return DirectedAtAsset; }

	FFlowPin GetPromptPin() const;

	FFlowPin GetEndPin() const;

	FFlowPin GetStartPin() const;;

	void ResolveMaturitySetting(EYapMaturitySetting& MaturitySetting) const;
	
	TOptional<bool> GetSkippableSetting() const { return Skippable; }
	
	TOptional<bool>& GetSkippableSetting() { return Skippable; }
	
	TOptional<bool> GetAutoAdvanceSetting() const { return AutoAdvance; }
	
	TOptional<bool>& GetAutoAdvanceSetting() { return AutoAdvance; }
	
	/** Gets the evaluated skippable setting to be used for this fragment (incorporating project default settings and fallbacks) */
	bool GetSkippable(bool Default) const;

	bool GetAutoAdvance(bool Default) const;
	
	/** Gets the evaluated time mode to be used for this bit (incorporating project default settings and fallbacks) */
	EYapTimeMode GetTimeMode() const;
	
	EYapTimeMode GetTimeMode(EYapMaturitySetting MaturitySetting) const;

	FGameplayTag GetMoodTag() const { return MoodTag; }

	bool IsTimeModeNone() const;

	bool HasAudio() const;


#if WITH_EDITOR
public:
	FYapBit& GetBitMutable(EYapMaturitySetting MaturitySetting);
		
	void SetIndexInDialogue(uint8 NewValue) { IndexInDialogue = NewValue; }

	FDelegateHandle FragmentTagChildrenFilterDelegateHandle;
	
	static void OnGetCategoriesMetaFromPropertyHandle(TSharedPtr<IPropertyHandle> PropertyHandle, FString& String);
	
	void SetPaddingToNextFragment(float NewValue) { PaddingToNextFragment = NewValue; }

	TArray<TObjectPtr<UYapCondition>>& GetConditionsMutable() { return Conditions; }

	void ResetGUID() { Guid = FGuid::NewGuid(); }
	
	FName GetPromptPinName() const { return GetPromptPin().PinName; }

	FName GetEndPinName() const { return GetEndPin().PinName; }

	FName GetStartPinName() const { return GetStartPin().PinName; }

	void ResetEndPin() { bShowOnEndPin = false; }

	void ResetStartPin() { bShowOnStartPin = false; }
	
	void InvalidateFragmentTag(UFlowNode_YapDialogue* OwnerNode);

	void SetMoodTag(const FGameplayTag& NewValue) { MoodTag = NewValue; };

	void SetTimeModeSetting(EYapTimeMode NewValue) { TimeMode = NewValue; }
	
	EYapTimeMode GetTimeModeSetting() const { return TimeMode; }
	// TODO implement this
	bool GetBitReplaced() const { return false; };
#endif

	
	// --------------------------------------------------------------------------------------------
	// EDITOR API
#if WITH_EDITOR
public:
	void SetSpeaker(TSoftObjectPtr<UYapCharacter> InCharacter);
	
	void SetDirectedAt(TSoftObjectPtr<UYapCharacter> InDirectedAt);
#endif
};
