#pragma once
#include "FlowYapBit.h"
#include "GameplayTagContainer.h"

#include "FlowYapFragment.generated.h"

class UFlowNode_YapDialogue;

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

#if WITH_EDITOR
	friend class SFlowGraphNode_YapDialogueWidget;
	friend class SFlowGraphNode_YapFragmentWidget;
	friend class SFlowYapBitDetailsWidget;
	friend class FPropertyCustomization_FlowYapFragment;
#endif
	
	// ==========================================
	// SETTINGS
protected:
	UPROPERTY(EditAnywhere)
	FFlowYapBit Bit;

	/** How many times is this fragment allowed to broadcast? This count persists only within this flow asset's lifespan (resets every Start). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = 0, UIMin = 0, UIMax = 5))
	int32 ActivationLimit = 0;
	
	UPROPERTY(EditAnywhere)
	FGameplayTag FragmentTag;
	
	UPROPERTY(EditAnywhere)
	float PaddingToNextFragment = 0;

	UPROPERTY(EditAnywhere)
	TOptional<uint8> CommonPaddingSetting;

	UPROPERTY(EditAnywhere)
	bool bShowOnStartPin = false;

	UPROPERTY(EditAnywhere)
	bool bShowOnEndPin = false;

	// ==========================================
	// STATE
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	uint8 IndexInDialogue = 0; 

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 ActivationCount = 0;

	UPROPERTY()
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

	const FGuid& GetGuid() { return Guid; }
	
#if WITH_EDITOR
public:
	FFlowYapBit& GetBitMutable() { return Bit; }
	
	TWeakObjectPtr<UFlowNode_YapDialogue> Owner;
	
	void SetIndexInDialogue(uint8 NewValue) { IndexInDialogue = NewValue; }

	FDelegateHandle FragmentTagFilterDelegateHandle;
	FDelegateHandle FragmentTagChildrenFilterDelegateHandle;
	
	void OnGetCategoriesMetaFromPropertyHandle(TSharedPtr<IPropertyHandle> PropertyHandle, FString& String) const;
	
	void SetPaddingToNextFragment(float NewValue) { PaddingToNextFragment = NewValue; }

	TOptional<uint8>& GetCommonPaddingSettingMutable() { return CommonPaddingSetting; }

	bool GetBitReplaced() const;

	bool GetShowOnStartPin() const { return bShowOnStartPin; }
	
	bool GetShowOnEndPin() const { return bShowOnEndPin; }
#endif
};