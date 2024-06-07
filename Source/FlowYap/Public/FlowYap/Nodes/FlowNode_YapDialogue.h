#pragma once

#include "Nodes/FlowNode.h"
#include "FlowYap/FlowYapFragment.h"
#include "FlowNode_YapDialogue.generated.h"

class UFlowYapCharacter;

enum class EFlowYapInterruptible : uint8;

UENUM(BlueprintType)
enum class EFlowYapMultipleFragmentSequencing : uint8
{
	Sequential	,
	Random		,
	COUNT		UMETA(Hidden)
};

ENUM_RANGE_BY_COUNT(EFlowYapMultipleFragmentSequencing, EFlowYapMultipleFragmentSequencing::COUNT);

// TODO: you should NOT be able to set activation limits on any fragments which do not have unconnected nodes below.

/**
 * Emits a FlowYap Dialogue Fragment
 */
UCLASS(NotBlueprintable, meta = (DisplayName = "Dialogue", Keywords = "yap", ToolTip = "Select node(s) and tap SHIFT key to show all node contents"))
class FLOWYAP_API UFlowNode_YapDialogue : public UFlowNode
{
	GENERATED_BODY()

friend class SFlowGraphNode_YapDialogueWidget;
	
public:
	UFlowNode_YapDialogue();

	// SETTINGS
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UFlowYapCharacter> Character;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FFlowYapFragment> Fragments;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName ConversationName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bIsPlayerPrompt;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = 0, UIMin = 0, UIMax = 5))
	int32 NodeActivationLimit;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EFlowYapMultipleFragmentSequencing MultipleFragmentSequencing;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EFlowYapInterruptible Interruptible;
	
	// STATE
protected:
	UPROPERTY(VisibleAnywhere, Transient, BlueprintReadOnly)
	int32 NodeActivationCount = 0;
	
	// API
public:
	void SetConversationName(FName Name);
	
	FText GetSpeakerName() const;

	FLinearColor GetSpeakerColor() const;

	const UTexture2D* GetDefaultSpeakerPortrait() const;

	const UTexture2D* GetSpeakerPortrait(const FName& RequestedPortraitKey) const;

	FSlateBrush* GetSpeakerPortraitBrush(const FName& RequestedPortraitKey) const;
	
	FText GetNodeTitle() const override;

	bool GetIsPlayerPrompt() const;

	int32 GetNodeActivationCount() const;

	int32 GetNodeActivationLimit() const;
	
	const TArray<FFlowYapFragment>& GetFragments();

	uint8 GetNumFragments() const;

	int16 FindFragmentIndex(FFlowYapFragment* Fragment) const;
	
	// -------------------

	void InitializeInstance() override;
	
	void OnActivate() override;
	
	void ExecuteInput(const FName& PinName) override;
	
#if WITH_EDITOR
public:
	TArray<FFlowYapFragment>& GetFragmentsMutable();
	
	void AddFragment();

	bool GetDynamicTitleColor(FLinearColor& OutColor) const override;

	bool CanUserAddInput() const override { return false; }

	bool CanUserAddOutput() const override { return false; }

	bool SupportsContextPins() const override;
	
	bool GetUsesMultipleInputs();
	
	bool GetUsesMultipleOutputs();

	EFlowYapMultipleFragmentSequencing GetMultipleFragmentSequencing() const;
	
	virtual TArray<FFlowPin> GetContextInputs() override;

	virtual TArray<FFlowPin> GetContextOutputs() override;
	
	void SetIsPlayerPrompt(bool NewValue);

	void SetNodeActivationLimit(int32 NewValue);

	void CycleFragmentSequencingMode();
	
	void DeleteFragmentByIndex(int16 DeleteIndex);
	
	void InsertFragment(uint8 Index);
	
	void UpdateFragmentIndices();

	void SwapFragments(uint8 IndexA, uint8 IndexB);
#endif
};
