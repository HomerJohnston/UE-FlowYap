#pragma once
#include "FlowYapBit.h"
#include "GameplayTagContainer.h"

#include "FlowYapFragment.generated.h"

class UFlowNode_YapDialogue;

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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = 0, UIMin = 0, UIMax = 5))
	int32 ActivationLimit = 0;

	UPROPERTY(EditAnywhere)
	FGameplayTag FragmentTag;
	
	// ==========================================
	// STATE
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	uint8 IndexInDialogue = 0; 

	// TODO move activation count and limit to the parent dialogue so that they persist naturally when swapping fragments
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 ActivationCount = 0;
	
	// ==========================================
	// API
public:
	uint8 GetIndexInDialogue() const { return IndexInDialogue; }
	
	int32 GetActivationCount() const { return ActivationCount; }
	
	int32 GetActivationLimit() const { return ActivationLimit; }

	const FFlowYapBit& GetBit() const { return Bit; }

	bool TryActivate();
	
#if WITH_EDITOR
	FFlowYapBit& GetBitMutable() { return Bit; }
	
	TWeakObjectPtr<UFlowNode_YapDialogue> Owner;
	
	void SetIndexInDialogue(uint8 NewValue) { IndexInDialogue = NewValue; }

	FDelegateHandle FragmentTagFilterDelegateHandle;
	FDelegateHandle FragmentTagChildrenFilterDelegateHandle;
	
	void OnGetCategoriesMetaFromPropertyHandle(TSharedPtr<IPropertyHandle> PropertyHandle, FString& String) const;
	
	void OnFilterGameplayTagChildren(const FString& String, TSharedPtr<FGameplayTagNode>& GameplayTagNode, bool& bArg) const;

#endif
};