#pragma once
#include "GameplayTagContainer.h"

#include "FlowYapSubsystem.generated.h"

class UFlowAsset;
class IFlowYapConversationListener;
struct FFlowYapBit;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFlowYapConversationEvent, FName, ConversationName);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FFlowYapDialogueEvent, FName, ConversationName, const FFlowYapBit&, DialogueInfo);

// ================================================================================================

USTRUCT(BlueprintType)
struct FFlowYapActiveConversation
{
	GENERATED_BODY()

	FFlowYapActiveConversation();

public:
	UPROPERTY(Transient)
	UFlowAsset* FlowAsset;

	UPROPERTY(Transient)
	FName Name;

public:
	TDelegate<void(FName)> OnConversationStarts;
	
	TDelegate<void(FName)> OnConversationEnds;

public:
	bool StartConversation(UFlowAsset* InOwningAsset, FName InName);

	bool EndConversation();

	bool IsConversationInProgress() const { return Name != NAME_None; };

	FName GetCurrentConversationName() const { return Name; }
};

// ================================================================================================

USTRUCT()
struct FYapFragmentActivationCount
{
	GENERATED_BODY()
	
	TMap<uint8, int32> Counts;
};

UCLASS()
class FLOWYAP_API UFlowYapSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

friend class UFlowNode_YapDialogue;
friend class UFlowNode_YapConversationStart;
friend class UFlowNode_YapConversationEnd;
friend struct FFlowYapFragment;
	
public:
	UFlowYapSubsystem();
	
	// ------------------------------------------
	// STATE
protected:
	// TODO I should have a queue system???... otherwise I'll have odd race conditions on multiple conversation requests
	//UPROPERTY(Transient)
	//TArray<FName> ConversationQueue;

	UPROPERTY(Transient)
	FFlowYapActiveConversation ActiveConversation;

	UPROPERTY(Transient)
	TArray<UObject*> Listeners;

	/**  */
	UPROPERTY(Transient)
	TMap<FGuid, FYapFragmentActivationCount> GlobalFragmentActivationCounts;

	// ------------------------------------------
	// PUBLIC API
public:
	UFUNCTION(BlueprintCallable)
	void AddConversationListener(UObject* NewListener);

	UFUNCTION(BlueprintCallable)
	void RemoveConversationListener(UObject* RemovedListener);

	// ------------------------------------------
	// FLOW YAP API - These are called by the flow node
protected:
	bool StartConversation(UFlowAsset* OwningAsset, FName ConversationName); // Called by ConversationStart node

	void EndCurrentConversation(); // Called by ConversationEnd node

	void BroadcastFragmentStart(UFlowNode_YapDialogue* Dialogue, uint8 FragmentIndex); // Called by Dialogue node, 2nd output pin 

	void BroadcastFragmentEnd(const UFlowNode_YapDialogue* OwnerDialogue, uint8 FragmentIndex); // Called by Dialogue node, 1st output pin

	int32 GetGlobalActivationCount(UFlowNode_YapDialogue* OwnerDialogue, uint8 FragmentIndex);

public:
	bool FragmentGlobalActivationLimitMet(UFlowNode_YapDialogue* Dialogue, uint8 FragmentIndex) const;
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

protected:
	void OnConversationStarts_Internal(FName Name);

	void OnConversationEnds_Internal(FName Name);

	bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;
};