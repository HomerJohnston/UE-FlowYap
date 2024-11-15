#pragma once
#include "FlowYapBit.h"
#include "GameplayTagContainer.h"

#include "FlowYapFragment.generated.h"

class UFlowYapCondition;
class UFlowNode_YapDialogue;
struct FFlowPin;

UENUM()
enum class EFlowYapFragmentPause : uint8
{
	None,
	Small,
	Medium,
	Large,
	Custom
};

USTRUCT(BlueprintType)
struct FLOWYAP_API FFlowYapFragment
{
	GENERATED_BODY()

public:
	FFlowYapFragment();
	
	~FFlowYapFragment();
	bool CheckConditions() const;
	void ResetOptionalPins();

#if WITH_EDITOR
	friend class SFlowGraphNode_YapDialogueWidget;
	friend class SFlowGraphNode_YapFragmentWidget;
	friend class SFlowYapBitDetailsWidget;
	friend class FPropertyCustomization_FlowYapFragment;
#endif
	
	// ==========================================
	// SETTINGS
protected:
	UPROPERTY(EditAnywhere, Instanced)
	TArray<TObjectPtr<UFlowYapCondition>> Conditions;
	
	UPROPERTY(EditAnywhere)
	FFlowYapBit Bit;

	/** How many times is this fragment allowed to broadcast? This count persists only within this flow asset's lifespan (resets every Start). */
	UPROPERTY(BlueprintReadOnly, meta = (ClampMin = 0, UIMin = 0, UIMax = 5))
	int32 ActivationLimit = 0;
	
	UPROPERTY()
	FGameplayTag FragmentTag;
	
	UPROPERTY(meta = (ClampMin = 0, UIMin = 0, UIMax = 5))
	float PaddingToNextFragment = 0;

	UPROPERTY()
	TOptional<uint8> CommonPaddingSetting;

	UPROPERTY(EditAnywhere)
	bool bShowOnEndPin = false;
	
	UPROPERTY(EditAnywhere)
	bool bShowOnStartPin = false;

	
	// ==========================================
	// STATE
protected:
	// TODO should this be serialized or transient
	UPROPERTY(BlueprintReadOnly)
	uint8 IndexInDialogue = 0; 

	UPROPERTY(BlueprintReadOnly, Transient)
	int32 ActivationCount = 0;

	UPROPERTY(VisibleAnywhere)
	FGuid Guid;
	
#if WITH_EDITORONLY_DATA
protected:
	bool bBitReplaced = false;
#endif

	// ==========================================
	// API
public:
	// TODO I don't think fragments should know where their position is!
	uint8 GetIndexInDialogue() const { return IndexInDialogue; }
	
	int32 GetActivationCount() const { return ActivationCount; }
	int32 GetActivationLimit() const { return ActivationLimit; }
	bool IsActivationLimitMet() const { if (ActivationLimit <= 0) return false; return (ActivationCount >= ActivationLimit); }
	
	const FFlowYapBit& GetBit() const { return Bit; }

	float GetPaddingToNextFragment() const;

	const TOptional<uint8>& GetCommonPaddingSetting() const { return CommonPaddingSetting; }
	
	void IncrementActivations();

	const FGameplayTag& GetFragmentTag() const { return FragmentTag; } 

	void ReplaceBit(const FFlowYapBitReplacement& ReplacementBit);

	const FGuid& GetGuid() const { return Guid; }

	FName GetStartPinName();

	FName GetEndPinName();
	
#if WITH_EDITOR
public:
	FFlowYapBit& GetBitMutable() { return Bit; }
	
	TWeakObjectPtr<UFlowNode_YapDialogue> Owner;
	
	void SetIndexInDialogue(uint8 NewValue) { IndexInDialogue = NewValue; }

	FDelegateHandle FragmentTagChildrenFilterDelegateHandle;
	
	static void OnGetCategoriesMetaFromPropertyHandle(TSharedPtr<IPropertyHandle> PropertyHandle, FString& String);
	
	void SetPaddingToNextFragment(float NewValue) { PaddingToNextFragment = NewValue; }

	TOptional<uint8>& GetCommonPaddingSettingMutable() { return CommonPaddingSetting; }

	bool GetBitReplaced() const;

	bool GetShowOnStartPin() const { return bShowOnStartPin; }
	
	bool GetShowOnEndPin() const { return bShowOnEndPin; }

	const TArray<TObjectPtr<UFlowYapCondition>>& GetConditions() const { return Conditions; }

	TArray<TObjectPtr<UFlowYapCondition>>& GetConditionsMutable() { return Conditions; }

	void ResetGUID() { Guid = FGuid::NewGuid(); };

	TArray<FFlowPin> GetOutputPins() const;

	FFlowPin GetPromptPin() const;
	
	FFlowPin GetEndPin() const;

	FFlowPin GetStartPin() const;

	void InvalidateFragmentTag();
#endif
};