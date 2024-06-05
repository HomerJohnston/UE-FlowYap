#pragma once
#include "FlowYapBit.h"

#include "FlowYapFragment.generated.h"

USTRUCT(BlueprintType)
struct FLOWYAP_API FFlowYapFragment
{
	GENERATED_BODY()

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

	// ==========================================
	// STATE
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	uint8 IndexInDialogue = 0; 
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 ActivationCount = 0;
	
	// ==========================================
	// API
public:
	uint8 GetIndexInDialogue() const { return IndexInDialogue; }
	
	int32 GetActivationCount() const { return ActivationCount; }
	
	int32 GetActivationLimit() const { return ActivationLimit; }

	const FFlowYapBit& GetBit() const { return Bit; }

#if WITH_EDITOR
	void SetIndexInDialogue(uint8 NewValue) { IndexInDialogue = NewValue; }
#endif
};