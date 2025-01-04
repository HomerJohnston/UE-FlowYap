#pragma once

#include "Nodes/FlowNode.h"
#include "Yap/YapFragment.h"
#include "Yap/Enums/YapDialogueSkippable.h"

#include "FlowNode_YapDialogue.generated.h"

class UYapCharacter;

// ------------------------------------------------------------------------------------------------
/**
 * Determines how a Talk node evaluates. Player Prompt nodes don't use this.
 */
UENUM()
enum class EYapDialogueTalkSequencing : uint8
{
	RunAll				UMETA(ToolTip = "The node will always try to run every fragment. The node will execute the Out pin after it finishes trying to run all fragments."), 
	RunUntilFailure		UMETA(ToolTip = "The node will attempt to run every fragment. If any one fails, the node will execute the Out pin."),
	SelectOne			UMETA(ToolTip = "The node will attempt to run every fragment. If any one passes, the node will execute the Out pin."),
	COUNT				UMETA(Hidden)
};

// ------------------------------------------------------------------------------------------------
/**
 * Node type. Freestyle talking or player prompt. Changes the execution flow of dialogue.
 */
UENUM()
enum class EYapDialogueNodeType : uint8
{
	Talk,
	PlayerPrompt,
	COUNT				UMETA(Hidden)
};

// ------------------------------------------------------------------------------------------------
/**
 * Emits Dialogue through UYapSubsystem.
 */
UCLASS(NotBlueprintable, meta = (DisplayName = "Dialogue", Keywords = "yap")) /*, ToolTip = "Emits Yap dialogue events"*/
class YAP_API UFlowNode_YapDialogue : public UFlowNode
{
	GENERATED_BODY()

#if WITH_EDITOR
	friend class SFlowGraphNode_YapDialogueWidget;
	friend class SFlowGraphNode_YapFragmentWidget;
	friend class SYapConditionDetailsViewWidget;
#endif

	friend class UYapSubsystem;

public:
	UFlowNode_YapDialogue();

	// ============================================================================================
	// SETTINGS
	// ============================================================================================
	
protected:
	/** What type of node we are. */
	UPROPERTY(BlueprintReadOnly)
	EYapDialogueNodeType DialogueNodeType;

	/** Maximum number of times we can successfully enter & exit this node. Any further attempts will trigger the Bypass output. */
	UPROPERTY(BlueprintReadOnly)
	int32 NodeActivationLimit;

	/** Controls how Talk nodes flow. See EYapDialogueTalkSequencing. */
	UPROPERTY(BlueprintReadOnly)
	EYapDialogueTalkSequencing TalkSequencing;

	/** Controls if dialogue can be interrupted. See EYapDialogueSkippable. */
	UPROPERTY(BlueprintReadOnly)
	EYapDialogueSkippable Skippable;

	/** Tags can be used to interact with this dialogue node during the game. Dialogue nodes can be looked up and/or modified by UYapSubsystem by their tag. */
	UPROPERTY(BlueprintReadOnly)
	FGameplayTag DialogueTag;

	/** Conditions which must be met for this dialogue to run. All conditions must pass (AND, not OR evaluation). If any conditions fail, Bypass output is triggered. */
	UPROPERTY(Instanced, BlueprintReadOnly, meta = (Yap_ReconstructNodeOnChange, Yap_ConditionProperty))
	TArray<TObjectPtr<UYapCondition>> Conditions;

	/** Actual dialogue contents. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FYapFragment> Fragments;

	// ============================================================================================
	// STATE
	// ============================================================================================
	
protected:
	/** How many times this node has been successfully ran. */
	UPROPERTY(Transient, BlueprintReadOnly)
	int32 NodeActivationCount = 0;

	/** Timer handle, used internally for fragment runs. */
	UPROPERTY(Transient)
	FTimerHandle FragmentTimerHandle;

#if WITH_EDITORONLY_DATA
	/** When was the current running fragment started? */ 
	double FragmentStartedTime = -1;

	/** When did the most recently ran fragment finish? */
	double FragmentEndedTime = -1;

	/** Helper to make it easier for slate widgets to determine if their fragment is running. */
	FYapFragment* RunningFragment = nullptr;

	/** Helper to make it easier for slate widgets to know which fragments have finished running. */
	TSet<FYapFragment*> FinishedFragments;
#endif

	static FName OutputPinName;
	static FName BypassPinName;
	
	// ============================================================================================
	// PUBLIC API
	// ============================================================================================

public:
	/** Is this dialogue a Talk node or a Player Prompt node? */
	bool IsPlayerPrompt() const { return DialogueNodeType == EYapDialogueNodeType::PlayerPrompt; }

	/** Does this node use title text? */ // TODO 
	bool UsesTitleText() const;

	/** How many times has this dialogue node successfully ran? */
	int32 GetNodeActivationCount() const { return NodeActivationCount; }

	/** How many times is this dialogue node allowed to successfully run? */
	int32 GetNodeActivationLimit() const { return NodeActivationLimit; }

	/** Dialogue fragments getter. */
	const TArray<FYapFragment>& GetFragments() const { return Fragments; }

	/** Simple helper function. */
	uint8 GetNumFragments() const { return Fragments.Num(); }

	/** Is dialogue from this node skippable by default? */
	bool GetSkippable() const;

	// TODO this sucks can I register the fragments some other way instead
	/** Finds the first fragment on this dialogue containing a tag. */
	FYapFragment* FindTaggedFragment(const FGameplayTag& Tag);
	
protected:
	bool ActivationLimitsMet() const;
	
#if WITH_EDITOR
private:
	EYapDialogueSkippable GetSkippableSetting() const;
	
	void InvalidateFragmentTags();

	const TArray<UYapCondition*>& GetConditions() const { return Conditions; }
	
	TArray<UYapCondition*>& GetConditionsMutable() { return MutableView(Conditions); }

	void ToggleNodeType();
#endif
	
	// ============================================================================================
	// OVERRIDES
	// ============================================================================================

protected:
	/** UFlowNodeBase override */
	void InitializeInstance() override;

	/** UFlowNodeBase override */
	void ExecuteInput(const FName& PinName) override;

	/** UFlowNode override */
	void OnPassThrough_Implementation() override;

	// ============================================================================================
	// INTERNAL API
	// ============================================================================================
	
protected:
	void BroadcastPrompts();

	void RunPrompt(uint8 Uint8);
	
	void FindStartingFragment();

	bool RunFragment(uint8 FragmentIndex);

	void WhenFragmentComplete(uint8 FragmentIndex);

	void WhenPaddingTimeComplete(uint8 FragmentIndex);

	bool IsBypassPinRequired() const;

	int16 FindFragmentIndex(const FGuid& InFragmentGuid) const;

public:
	void OnCharacterLoadComplete(FYapBit* Bit, TSoftObjectPtr<UYapCharacter>* CharacterAsset, TObjectPtr<UYapCharacter>* Character);
	
protected:
	bool TryBroadcastFragment(uint8 FragmentIndex);
	
	const FYapFragment& GetFragmentByIndex(uint8 Index) const;

#if WITH_EDITOR
private:
	
	FYapFragment& GetFragmentByIndexMutable(uint8 Index);
	
	TArray<FYapFragment>& GetFragmentsMutable();
	
	void RemoveFragment(int32 Index);

	FText GetNodeTitle() const override;
	
	bool CanUserAddInput() const override { return false; }

	bool CanUserAddOutput() const override { return false; }

	bool SupportsContextPins() const override;
	
	bool GetUsesMultipleInputs();
	
	bool GetUsesMultipleOutputs();

	EYapDialogueTalkSequencing GetMultipleFragmentSequencing() const;
	
	TArray<FFlowPin> GetContextOutputs() const override;

	void SetNodeActivationLimit(int32 NewValue);

	void CycleFragmentSequencingMode();
	
	void DeleteFragmentByIndex(int16 DeleteIndex);
	
	void AddFragment(int32 InsertionIndex = INDEX_NONE);
	
	void UpdateFragmentIndices();

	void SwapFragments(uint8 IndexA, uint8 IndexB);
	
public:
	const FYapFragment* GetRunningFragment() const { return RunningFragment; }

	const TSet<FYapFragment*> GetFinishedFragments() const { return FinishedFragments; }
	
	FString GetNodeDescription() const override;

	const FGameplayTag& GetDialogueTag() const { return DialogueTag; }
	
	void OnFilterGameplayTagChildren(const FString& String, TSharedPtr<FGameplayTagNode>& GameplayTagNode, bool& bArg) const;
	
	void ForceReconstruction();

	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	
	virtual void PostEditImport() override;
	
	virtual bool CanRefreshContextPinsOnLoad() const { return true; }
	
	FText GetNodeToolTip() const override { return FText::GetEmpty(); };

	void PostLoad() override;

	void PreloadContent() override;
#endif // WITH_EDITOR
};
