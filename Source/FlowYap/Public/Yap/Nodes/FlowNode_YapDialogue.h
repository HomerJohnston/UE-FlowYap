#pragma once

#include "Nodes/FlowNode.h"
#include "Yap/FlowYapFragment.h"
#include "FlowNode_YapDialogue.generated.h"

class UFlowYapCharacter;

enum class EFlowYapInterruptible : uint8;

UENUM(BlueprintType)
enum class EFlowYapMultipleFragmentSequencing : uint8
{
	Sequential	,
	SelectOne	,
	Prompt		,
	COUNT		UMETA(Hidden)
};

UENUM(BlueprintType, meta = (Bitflags))
enum class ECcClimateZone : uint8
{
	NONE = 0 UMETA(Hidden),
	Spring = 0x00000000 << 0,
	Summer = 0x00000001 << 1,
	Fall = 0x00000002 << 2,
	Winter = 0x00000003 << 3,
	ALL = Spring | Summer | Fall | Winter UMETA(Hidden)
};

ENUM_RANGE_BY_VALUES(ECcClimateZone, ECcClimateZone::Spring, ECcClimateZone::Summer, ECcClimateZone::Fall, ECcClimateZone::Winter);
ENUM_RANGE_BY_COUNT(EFlowYapMultipleFragmentSequencing, EFlowYapMultipleFragmentSequencing::COUNT);

// TODO: you should NOT be able to set activation limits on any fragments which do not have unconnected nodes below them?

// TODO: make this NotBlueprintable, I have it blueprintable to make it easier to check details customizaitons for dev
/**
 * Emits a FlowYap Dialogue Fragment
 */
UCLASS(Blueprintable, meta = (DisplayName = "Dialogue", Keywords = "yap", ToolTip = "Select node(s) and tap SHIFT key to show all node contents"))
class FLOWYAP_API UFlowNode_YapDialogue : public UFlowNode
{
	GENERATED_BODY()

#if WITH_EDITOR
	friend class SFlowGraphNode_YapDialogueWidget;
	friend class SFlowGraphNode_YapFragmentWidget;
	friend class FDetailCustomization_FlowYapDialogueNode;
	friend class SFlowYapBitDetailsWidget;
	friend class FPropertyCustomization_FlowYapFragment;
#endif

	friend class UFlowYapSubsystem;

public:
	UFlowNode_YapDialogue();
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true, Bitmask, BitmaskEnum = "/Script/MyGame.ECcClimateZone"))
	ECcClimateZone SupportedClimateZones;
	
	// SETTINGS
protected:	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FFlowYapFragment> Fragments;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, AdvancedDisplay)
	bool bIsPlayerPrompt;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, AdvancedDisplay, meta = (ClampMin = 0, UIMin = 0, UIMax = 5))
	int32 NodeActivationLimit;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, AdvancedDisplay, meta = (EditCondition = "!bIsPlayerPrompt", EditConditionHides))
	EFlowYapMultipleFragmentSequencing FragmentSequencing;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, AdvancedDisplay)
	EFlowYapInterruptible Interruptible;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag DialogueTag;

	UPROPERTY(EditAnywhere, Instanced)
	TArray<TObjectPtr<UFlowYapCondition>> Conditions;
	
	// STATE
protected:
	UPROPERTY(VisibleAnywhere, Transient, BlueprintReadOnly)
	int32 NodeActivationCount = 0;

	UPROPERTY()
	FTimerHandle TimerHandle;

#if WITH_EDITOR
	TMap<uint8, TArray<FName>> FragmentPins;
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

	int16 FindFragmentIndex(FFlowYapFragment* Fragment) const;

	FFlowYapFragment* FindTaggedFragment(const FGameplayTag& Tag);
	
	// -------------------

	void InitializeInstance() override;

	void OnActivate() override;

	void ExecuteInput(const FName& PinName) override;

	bool GetInterruptible() const;
	
	const TArray<TObjectPtr<UFlowYapCondition>>& GetConditions() const { return Conditions; };

protected:
	void BroadcastPrompts();

protected:
	void RunPromptAsDialogue(uint8 Uint8);

	void RunFragmentsAsDialogue(uint8 StartIndex, EFlowYapMultipleFragmentSequencing SequencingMode, bool bSuccess);

	bool RunFragment(uint8 FragmentIndex, EFlowYapMultipleFragmentSequencing SequencingMode);

	void OnFragmentComplete(uint8 FragmentIndex, EFlowYapMultipleFragmentSequencing SequencingMode);

	void OnPaddingTimeComplete(uint8 FragmentIndex, EFlowYapMultipleFragmentSequencing SequencingMode);

	bool BypassPinRequired() const;
	
protected:
	bool TryBroadcastFragmentAsDialogue(FFlowYapFragment& Fragment);

#if WITH_EDITOR
public:
	const FFlowYapFragment* GetFragmentByIndex(int16 Index) const;
	
	FFlowYapFragment* GetFragmentByIndexMutable(int16 Index);
	
	TArray<FFlowYapFragment>& GetFragmentsMutable();
	
	void AddFragment();

	FText GetNodeTitle() const override;
	
	bool GetDynamicTitleColor(FLinearColor& OutColor) const override;

	bool CanUserAddInput() const override { return false; }

	bool CanUserAddOutput() const override { return false; }

	bool SupportsContextPins() const override;
	
	bool GetUsesMultipleInputs();
	
	bool GetUsesMultipleOutputs();

	EFlowYapMultipleFragmentSequencing GetMultipleFragmentSequencing() const;
	
	//virtual TArray<FFlowPin> GetContextInputs() override;

	virtual TArray<FFlowPin> GetContextOutputs() override;
	
	void SetIsPlayerPrompt(bool NewValue);

	void SetNodeActivationLimit(int32 NewValue);

	void CycleFragmentSequencingMode();
	
	void DeleteFragmentByIndex(int16 DeleteIndex);
	
	void InsertFragment(uint8 Index);
	
	void UpdateFragmentIndices();

	void SwapFragments(uint8 IndexA, uint8 IndexB);

protected:
	TOptional<uint8> RunningFragmentIndex;

	double FragmentStartedTime;
	double FragmentEndedTime;

public:
	TOptional<uint8> GetRunningFragmentIndex() const { return RunningFragmentIndex; }

	FString GetNodeDescription() const override;

	const FGameplayTag& GetDialogueTag() const { return DialogueTag; }
	
	void OnFilterGameplayTagChildren(const FString& String, TSharedPtr<FGameplayTagNode>& GameplayTagNode, bool& bArg) const;

#endif // WITH_EDITOR
};
