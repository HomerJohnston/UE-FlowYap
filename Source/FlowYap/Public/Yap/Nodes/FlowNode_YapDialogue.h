#pragma once

#include "Nodes/FlowNode.h"
#include "Yap/FlowYapFragment.h"
#include "FlowNode_YapDialogue.generated.h"

class UFlowYapCharacter;

enum class EFlowYapInterruptible : uint8;

// Used for "Talk" nodes only. Prompt nodes don't use this.
UENUM(BlueprintType)
enum class EFlowYapMultipleFragmentSequencing : uint8
{
	Sequential,
	SelectOne,
	COUNT		UMETA(Hidden)
};

// TODO: you should NOT be able to set activation limits on any fragments which do not have unconnected nodes below them?
// TODO: make sure this is NotBlueprintable for 1.0, I have it blueprintable to make it easier to check details customizaitons for dev
/**
 * Emits a FlowYap Dialogue Fragment
 */
UCLASS(NotBlueprintable, meta = (DisplayName = "Dialogue", Keywords = "yap", ToolTip = "Emits Yap dialogue events"))
class FLOWYAP_API UFlowNode_YapDialogue : public UFlowNode
{
	GENERATED_BODY()

#if WITH_EDITOR
	friend class SFlowGraphNode_YapDialogueWidget;
	friend class SFlowGraphNode_YapFragmentWidget;
	friend class FDetailCustomization_FlowYapDialogueNode;
	friend class SFlowYapBitDetailsWidget;
	friend class FPropertyCustomization_FlowYapFragment;
	friend class FFlowYapWidgetHelper;
#endif

	friend class UFlowYapSubsystem;

public:
	UFlowNode_YapDialogue();

	// SETTINGS
protected:
	UPROPERTY(BlueprintReadOnly)
	bool bIsPlayerPrompt;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	int32 NodeActivationLimit;

	UPROPERTY(BlueprintReadOnly)
	EFlowYapMultipleFragmentSequencing FragmentSequencing;

	UPROPERTY(BlueprintReadOnly)
	EFlowYapInterruptible Interruptible;

	UPROPERTY(BlueprintReadOnly)
	FGameplayTag DialogueTag;

	UPROPERTY(EditAnywhere, Instanced, BlueprintReadOnly)
	TArray<TObjectPtr<UFlowYapCondition>> Conditions;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FFlowYapFragment> Fragments;
	
	// STATE
protected:
	UPROPERTY(Transient, BlueprintReadOnly)
	int32 NodeActivationCount = 0;

	UPROPERTY(Transient)
	FTimerHandle TimerHandle;

#if WITH_EDITORONLY_DATA
private:
	UPROPERTY(Transient, VisibleAnywhere, Instanced, SkipSerialization, meta = (AllowPrivateAccess))
	TObjectPtr<UFlowYapCondition> SelectedCondition;
#endif
	
	// API
public:
	FText GetSpeakerName() const;

	const UTexture2D* GetDefaultSpeakerPortrait() const;

	const UTexture2D* GetSpeakerPortrait(const FName& RequestedMoodKey) const;

	FSlateBrush* GetSpeakerPortraitBrush(const FName& RequestedMoodKey) const;
	
	bool GetIsPlayerPrompt() const;

	int32 GetNodeActivationCount() const;

	int32 GetNodeActivationLimit() const;
	
	const TArray<FFlowYapFragment>& GetFragments() const;

	uint8 GetNumFragments() const;

	int16 FindFragmentIndex(const FGuid& InFragmentGuid) const;

	FFlowYapFragment* FindTaggedFragment(const FGameplayTag& Tag);
	
	// -------------------

	void InitializeInstance() override;

	void OnActivate() override;

	void ExecuteInput(const FName& PinName) override;

	void Cleanup() override;
	
	void OnPassThrough_Implementation() override;
	
	bool GetInterruptible() const;

	EFlowYapInterruptible GetInterruptibleSetting() const;
	
	const TArray<TObjectPtr<UFlowYapCondition>>& GetConditions() const { return Conditions; }

protected:
	void BroadcastPrompts();

protected:
	void RunPrompt(uint8 Uint8);
	
	void RunFragments();

	bool RunFragment(uint8 FragmentIndex, bool bRunNext = false);

	void OnFragmentComplete(uint8 FragmentIndex, bool bRunNext);

	void OnPaddingTimeComplete(uint8 FragmentIndex, bool bRunNext);

	bool BypassPinRequired() const;
	
protected:
	bool TryBroadcastFragment(uint8 FragmentIndex);

#if WITH_EDITOR
public:
	const FFlowYapFragment& GetFragmentByIndex(uint8 Index) const;
	
	FFlowYapFragment& GetFragmentByIndexMutable(uint8 Index);
	
	TArray<FFlowYapFragment>& GetFragmentsMutable();
	
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

	bool ActivationLimitsMet() const;

	void ForceReconstruction();

	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	
#endif // WITH_EDITOR

	virtual void PostEditImport() override;
};
