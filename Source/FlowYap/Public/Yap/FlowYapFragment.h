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
#endif
	
	// ==========================================
	// SETTINGS
protected:
	UPROPERTY(EditAnywhere)
	FFlowYapBit Bit;

	/** How many times is this fragment allowed to broadcast? This count persists only within this flow asset's lifespan (resets every Start). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = 0, UIMin = 0, UIMax = 5))
	int32 LocalActivationLimit = 0;

	/** How many times is this fragment allowed to broadcast? This count persists in the world scope (resets on level load). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = 0, UIMin = 0, UIMax = 5))
	int32 GlobalActivationLimit = 0;
	
	UPROPERTY(EditAnywhere)
	FGameplayTag FragmentTag;
	
	UPROPERTY(EditAnywhere)
	float PaddingToNextFragment = 0;

	UPROPERTY(EditAnywhere)
	TOptional<uint8> CommonPaddingSetting;
	
	// ==========================================
	// STATE
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	uint8 IndexInDialogue = 0; 

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 LocalActivationCount = 0;

	// ==========================================
	// API
public:
	uint8 GetIndexInDialogue() const { return IndexInDialogue; }
	
	int32 GetLocalActivationCount() const { return LocalActivationCount; }
	int32 GetLocalActivationLimit() const { return LocalActivationLimit; }
	bool IsLocalActivationLimitMet() const { if (LocalActivationLimit <= 0) return false; return (LocalActivationCount >= LocalActivationLimit); }
	
	int32 GetGlobalActivationCount(UFlowNode_YapDialogue* WorldContextObject) const;
	int32 GetGlobalActivationLimit() const { return GlobalActivationLimit; }
	bool IsGlobalActivationLimitMet(UFlowNode_YapDialogue* WorldContextObject) const;

	bool IsActivationLimitMet(UFlowNode_YapDialogue* Dialogue) const;
	
	const FFlowYapBit& GetBit() const { return Bit; }

	float GetPaddingToNextFragment() const;

	const TOptional<uint8>& GetCommonPaddingSetting() const { return CommonPaddingSetting; }
	
	void IncrementActivations();
	
#if WITH_EDITOR
public:
	FFlowYapBit& GetBitMutable() { return Bit; }
	
	TWeakObjectPtr<UFlowNode_YapDialogue> Owner;
	
	void SetIndexInDialogue(uint8 NewValue) { IndexInDialogue = NewValue; }

	FDelegateHandle FragmentTagFilterDelegateHandle;
	FDelegateHandle FragmentTagChildrenFilterDelegateHandle;
	
	void OnGetCategoriesMetaFromPropertyHandle(TSharedPtr<IPropertyHandle> PropertyHandle, FString& String) const;
	
	//void OnFilterGameplayTagChildren(const FString& String, TSharedPtr<FGameplayTagNode>& GameplayTagNode, bool& bArg) const;

	void SetPaddingToNextFragment(float NewValue) { PaddingToNextFragment = NewValue; }

	TOptional<uint8>& GetCommonPaddingSettingMutable() { return CommonPaddingSetting; }

#endif
};