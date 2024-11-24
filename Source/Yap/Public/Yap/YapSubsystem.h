#pragma once
#include "GameplayTagContainer.h"
#include "YapCharacterComponent.h"
#include "Yap/YapBitReplacement.h"
#include "YapSubsystem.generated.h"

class UFlowNode_YapDialogue;
struct FYapPromptHandle;
class UFlowAsset;
class IYapConversationHandlerInterface;
struct FYapBit;
class UYapTextCalculator;
class UYapCharacterComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFlowYapConversationEvent, FName, ConversationName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FFlowYapDialogueEvent, FName, ConversationName, const FYapBit&, DialogueInfo);

// ================================================================================================

USTRUCT(BlueprintType)
struct FYapActiveConversation
{
	GENERATED_BODY()

	FYapActiveConversation();

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

// ================================================================================================

UCLASS()
class YAP_API UYapSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

friend class UFlowNode_YapDialogue;
friend class UFlowNode_YapConversationStart;
friend class UFlowNode_YapConversationEnd;
friend struct FYapFragment;
friend struct FYapPromptHandle;
	
public:
	UYapSubsystem();
	
	// ------------------------------------------
	// STATE
protected:
	// TODO I should have a queue system???... otherwise I'll have odd race conditions on multiple conversation requests
	//UPROPERTY(Transient)
	//TArray<FName> ConversationQueue;

	/**  */
	UPROPERTY(Transient)
	FYapActiveConversation ActiveConversation;

	/**  */
	UPROPERTY(Transient)
	TArray<UObject*> Listeners;

	/** Stores the tag of a fragment and the owning dialogue node where that fragment can be found */
	UPROPERTY(Transient)
	TMap<FGameplayTag, UFlowNode_YapDialogue*> TaggedFragments;

	/** Stores overrides of bit replacements. Can only store one at a time, new assignments simply replace the old one. */
	UPROPERTY(Transient)
	TMap<FGameplayTag, FYapBitReplacement> BitReplacements;

	/**  */
	UPROPERTY(Transient)
	TSubclassOf<UYapTextCalculator> TextCalculatorClass;

	/**  */
	UPROPERTY(Transient)
	UClass* DialogueAudioAssetClass;

	/**  */
	UPROPERTY(Transient)
	TMap<FGameplayTag, TWeakObjectPtr<UYapCharacterComponent>> YapCharacterComponents;

	/**  */
	UPROPERTY(Transient)
	TSet<TObjectPtr<AActor>> RegisteredYapCharacterActors;

	UPROPERTY(Transient)
	TSubclassOf<UObject> ConversationHandlerClass;
	
	UPROPERTY(Transient)
	TObjectPtr<UObject> ConversationHandler;
	
	// ------------------------------------------
	// PUBLIC API
public:
	/** When a conversation starts */
	UFUNCTION(BlueprintCallable)
	void AddConversationHandler(UObject* NewListener);

	/**  */
	UFUNCTION(BlueprintCallable)
	void RemoveConversationHandler(UObject* RemovedListener);

	UFUNCTION(BlueprintCallable)
	UYapCharacterComponent* GetYapCharacter(const FGameplayTag& CharacterTag);
	
	// ------------------------------------------
	// FLOW YAP API - These are called by Yap classes
public:
	/**  */
	FYapFragment* FindTaggedFragment(const FGameplayTag& FragmentTag);

protected:
	/**  */
	void RegisterTaggedFragment(const FGameplayTag& FragmentTag, UFlowNode_YapDialogue* DialogueNode);

	/**  */
	bool StartConversation(UFlowAsset* OwningAsset, const FGameplayTag& ConversationName); // Called by ConversationStart node

	/**  */
	void EndCurrentConversation(); // Called by ConversationEnd node

	/**  */
	void BroadcastPrompt(UFlowNode_YapDialogue* Dialogue, uint8 FragmentIndex);

	/**  */
	void BroadcastDialogueStart(UFlowNode_YapDialogue* Dialogue, uint8 FragmentIndex); // Called by Dialogue node, 2nd output pin 

	/**  */
	void BroadcastDialogueEnd(const UFlowNode_YapDialogue* OwnerDialogue, uint8 FragmentIndex); // Called by Dialogue node, 1st output pin

	/**  */
	void RunPrompt(FYapPromptHandle& Handle);

public:
	/**  */
	void RegisterCharacterComponent(UYapCharacterComponent* YapCharacterComponent);

	/**  */
	void UnregisterCharacterComponent(UYapCharacterComponent* YapCharacterComponent);
	
public:
	/**  */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

protected:
	/**  */
	void OnConversationStarts_Internal(const FGameplayTag& Name);

	/**  */
	void OnConversationEnds_Internal(const FGameplayTag& Name);

	/**  */
	bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;
};