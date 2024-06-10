#pragma once

#include "FlowYapSubsystem.generated.h"

class IFlowYapConversationListener;
struct FFlowYapBit;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFlowYapConversationEvent, FName, ConversationName);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FFlowYapDialogueEvent, FName, ConversationName, const FFlowYapBit&, DialogueInfo);

// ================================================================================================

USTRUCT(BlueprintType)
struct FTestDelegateContainer
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite, BlueprintAssignable)
	FFlowYapConversationEvent Test1;
	
	UPROPERTY(BlueprintReadWrite, BlueprintAssignable)
	FFlowYapConversationEvent Test2;
};

USTRUCT(BlueprintType)
struct FFlowYapActiveConversation
{
	GENERATED_BODY()

	FFlowYapActiveConversation();
protected:
	FName ConversationName;

public:
	TDelegate<void(FName)> OnConversationStarts;
	
	TDelegate<void(FName)> OnConversationEnds;

public:
	bool TryStartConversation(FName InName);

	bool EndConversation();

	bool IsConversationInProgress() const { return ConversationName != NAME_None; };

	FName GetCurrentConversationName() const { return ConversationName; }
};

// ================================================================================================

UCLASS()
class FLOWYAP_API UFlowYapSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

friend class UFlowNode_YapDialogue;
friend class UFlowNode_YapConversationStart;
friend class UFlowNode_YapConversationEnd;
	
public:
	UFlowYapSubsystem();
	
	// ------------------------------------------
	// STATE
protected:
	UPROPERTY()
	TArray<FName> ConversationQueue;

	UPROPERTY()
	FFlowYapActiveConversation Conversation;

	UPROPERTY()
	TArray<UObject*> Listeners;
	
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
	void StartConversation(FName ConversationName); // Called by ConversationStart node

	void EndCurrentConversation(); // Called by ConversationEnd node

	void DialogueStart(FName ConversationName, const FFlowYapBit& DialogueBit); // Called by Dialogue node, 2nd output pin 

	void DialogueEnd(FName ConversationName, const FFlowYapBit& DialogueBit); // Called by Dialogue node, 1st output pin
	
	void DialogueInterrupt(FName ConversationName, const FFlowYapBit& DialogueBit); // Called by Dialogue node, 3rd output pin
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

protected:
	void OnConversationStarts_Internal(FName Name);

	void OnConversationEnds_Internal(FName Name);

	// TEST TEST TEST
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FTestDelegateContainer TestContainer;

	UFUNCTION(BlueprintCallable)
	FTestDelegateContainer& GetTestContainer() { return TestContainer; };
};