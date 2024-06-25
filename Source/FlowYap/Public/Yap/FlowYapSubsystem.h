#pragma once
#include "GameplayTagContainer.h"

#include "FlowYapSubsystem.generated.h"

class UFlowNode_YapDialogue;
struct FYapPromptHandle;
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
	FGameplayTag Conversation;

public:
	TDelegate<void(const FGameplayTag&)> OnConversationStarts;
	
	TDelegate<void(const FGameplayTag&)> OnConversationEnds;

public:
	bool StartConversation(UFlowAsset* InOwningAsset, const FGameplayTag& InName);

	bool EndConversation();

	bool IsConversationInProgress() const { return Conversation != FGameplayTag::EmptyTag; };

	const FGameplayTag& GetCurrentConversationName() const { return Conversation; }
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
friend struct FYapPromptHandle;
	
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
	// FLOW YAP API - These are called by Yap classes
protected:
	bool StartConversation(UFlowAsset* OwningAsset, const FGameplayTag& ConversationName); // Called by ConversationStart node

	void EndCurrentConversation(); // Called by ConversationEnd node

	void BroadcastPrompt(UFlowNode_YapDialogue* Dialogue, uint8 FragmentIndex);

	void BroadcastDialogueStart(UFlowNode_YapDialogue* Dialogue, uint8 FragmentIndex); // Called by Dialogue node, 2nd output pin 

	void BroadcastDialogueEnd(const UFlowNode_YapDialogue* OwnerDialogue, uint8 FragmentIndex); // Called by Dialogue node, 1st output pin

	int32 GetGlobalActivationCount(UFlowNode_YapDialogue* OwnerDialogue, uint8 FragmentIndex);

	void ActivatePrompt(FYapPromptHandle& Handle);

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

protected:
	void OnConversationStarts_Internal(const FGameplayTag& Name);

	void OnConversationEnds_Internal(const FGameplayTag& Name);

	bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;
};