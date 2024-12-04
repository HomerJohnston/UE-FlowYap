#pragma once

#include "Nodes/FlowNode.h"
#include "Yap/YapFragment.h"
#include "FlowNode_YapDialogue.generated.h"

class UYapCharacter;

enum class EFlowYapInterruptible : uint8;

// Used for "Talk" nodes only. Prompt nodes don't use this.
UENUM(BlueprintType)
enum class EFlowYapMultipleFragmentSequencing : uint8
{
	RunAll,
	RunUntilFailure,
	SelectOne,
	COUNT		UMETA(Hidden)
};

// TODO: you should NOT be able to set activation limits on any fragments which do not have unconnected nodes below them?
// TODO: make sure this is NotBlueprintable for 1.0, I have it blueprintable to make it easier to check details customizaitons for dev
/**
 * Emits a FlowYap Dialogue Fragment
 */
UCLASS(NotBlueprintable, meta = (DisplayName = "Dialogue", Keywords = "yap")) /*, ToolTip = "Emits Yap dialogue events"*/
class YAP_API UFlowNode_YapDialogue : public UFlowNode
{
	GENERATED_BODY()

#if WITH_EDITOR
	friend class SFlowGraphNode_YapDialogueWidget;
	friend class SFlowGraphNode_YapFragmentWidget;
	friend class FDetailCustomization_FlowYapDialogueNode;
	friend class SFlowYapBitDetailsWidget;
	friend class FPropertyCustomization_FlowYapFragment;
	friend class FYapWidgetHelper;
#endif

	friend class UYapSubsystem;

public:
	UFlowNode_YapDialogue();

	// SETTINGS
protected:
	UPROPERTY(BlueprintReadOnly)
	bool bIsPlayerPrompt;

	UPROPERTY(BlueprintReadOnly)
	int32 NodeActivationLimit;

	UPROPERTY(BlueprintReadOnly)
	EFlowYapMultipleFragmentSequencing FragmentSequencing;

	UPROPERTY(BlueprintReadOnly)
	EFlowYapInterruptible Interruptible;

	UPROPERTY(BlueprintReadOnly)
	FGameplayTag DialogueTag;

	UPROPERTY(EditAnywhere, Instanced, BlueprintReadOnly, meta = (Yap_ReconstructNodeOnChange, Yap_ConditionProperty))
	TArray<TObjectPtr<UYapCondition>> Conditions;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FYapFragment> Fragments;
	
	// STATE
protected:
	UPROPERTY(Transient, BlueprintReadOnly)
	int32 NodeActivationCount = 0;

	UPROPERTY(Transient)
	FTimerHandle TimerHandle;
	
	// API
public:
	FText GetSpeakerName() const;

	const UTexture2D* GetDefaultSpeakerPortrait() const;

	const UTexture2D* GetSpeakerPortrait(const FName& RequestedMoodKey) const;

	FSlateBrush* GetSpeakerPortraitBrush(const FName& RequestedMoodKey) const;
	
	bool GetIsPlayerPrompt() const;

	int32 GetNodeActivationCount() const;

	int32 GetNodeActivationLimit() const;
	
	const TArray<FYapFragment>& GetFragments() const;

	uint8 GetNumFragments() const;

	int16 FindFragmentIndex(const FGuid& InFragmentGuid) const;

	FYapFragment* FindTaggedFragment(const FGameplayTag& Tag);
	
	// -------------------

	void InitializeInstance() override;

	void OnActivate() override;

	void ExecuteInput(const FName& PinName) override;

	void Cleanup() override;
	
	void OnPassThrough_Implementation() override;
	
	bool GetInterruptible() const;

	EFlowYapInterruptible GetInterruptibleSetting() const;
	
	const TArray<UYapCondition*>& GetConditions() const { return Conditions; }

#if WITH_EDITOR
	void InvalidateFragmentTags();

	TArray<TObjectPtr<UYapCondition>>& GetConditionsMutable() { return Conditions; }

#endif
	
	bool ActivationLimitsMet() const;

protected:
	void BroadcastPrompts();

protected:
	void RunPrompt(uint8 Uint8);
	
	void FindStartingFragment();

	bool RunFragment(uint8 FragmentIndex);

	void OnFragmentComplete(uint8 FragmentIndex);

	void OnPaddingTimeComplete(uint8 FragmentIndex);

	bool IsBypassPinRequired() const;


protected:
	bool TryBroadcastFragment(uint8 FragmentIndex);
	
	const FYapFragment& GetFragmentByIndex(uint8 Index) const;

#if WITH_EDITOR
public:
	
	FYapFragment& GetFragmentByIndexMutable(uint8 Index);
	
	TArray<FYapFragment>& GetFragmentsMutable();
	
	void RemoveFragment(int32 Index);

	FText GetNodeTitle() const override;
	
	bool GetDynamicTitleColor(FLinearColor& OutColor) const override;

	bool CanUserAddInput() const override { return false; }

	bool CanUserAddOutput() const override { return false; }

	bool SupportsContextPins() const override;
	
	bool GetUsesMultipleInputs();
	
	bool GetUsesMultipleOutputs();

	EFlowYapMultipleFragmentSequencing GetMultipleFragmentSequencing() const;
	
	TArray<FFlowPin> GetContextOutputs() const override;

	void SetIsPlayerPrompt(bool NewValue);

	void SetNodeActivationLimit(int32 NewValue);

	void CycleFragmentSequencingMode();
	
	void DeleteFragmentByIndex(int16 DeleteIndex);
	
	void AddFragment(int32 InsertionIndex = INDEX_NONE);
	
	void UpdateFragmentIndices();

	void SwapFragments(uint8 IndexA, uint8 IndexB);

	int32 GetFragmentIndex(const FGuid& Guid) const;
	
protected:
	TOptional<uint8> RunningFragmentIndex;

	double FragmentStartedTime;
	double FragmentEndedTime;

public:
	TOptional<uint8> GetRunningFragmentIndex() const { return RunningFragmentIndex; }

	FString GetNodeDescription() const override;

	const FGameplayTag& GetDialogueTag() const { return DialogueTag; }
	
	void OnFilterGameplayTagChildren(const FString& String, TSharedPtr<FGameplayTagNode>& GameplayTagNode, bool& bArg) const;
	
	void ForceReconstruction();

	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	
	virtual void PostEditImport() override;
	
	virtual bool CanRefreshContextPinsOnLoad() const { return true; }
	
	FText GetNodeToolTip() const override { return FText::GetEmpty(); };
#endif // WITH_EDITOR
};
