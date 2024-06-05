#pragma once

#include "Nodes/FlowNode.h"
#include "FlowYap/FlowYapFragment.h"
#include "FlowNode_YapDialogue.generated.h"

class UFlowYapCharacter;

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
UCLASS(NotBlueprintable, meta = (DisplayName = "Dialogue", Keywords = "yap", ToolTip = "Select and press CTRL to show all node contents"))
class FLOWYAP_API UFlowNode_YapDialogue : public UFlowNode
{
	GENERATED_BODY()
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
	
	// STATE
protected:
	UPROPERTY(VisibleAnywhere, Transient, BlueprintReadOnly)
	int32 NodeActivationCount = 0;
	
#if WITH_EDITORONLY_DATA
protected:
	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	//bool bMultipleInputs;
	
	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	//bool bMultipleOutputs;
#endif
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
	
	TArray<FFlowYapFragment>& GetFragments();

	uint8 GetNumFragments() const;

	int16 FindFragmentIndex(FFlowYapFragment* Fragment) const;
	
	// -------------------

	void InitializeInstance() override;
	
	void OnActivate() override;
	
	void ExecuteInput(const FName& PinName) override;
	
	void AddFragment();

#if WITH_EDITOR
public:
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
#endif
};
