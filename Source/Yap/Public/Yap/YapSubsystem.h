// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once
#include "GameplayTagContainer.h"
#include "YapCharacterComponent.h"
#include "YapBroker.h"
#include "YapPromptHandle.h"
#include "Enums/YapMaturitySetting.h"
#include "Yap/YapDialogueHandle.h"
#include "Yap/YapBitReplacement.h"

#include "YapSubsystem.generated.h"

class UYapConversationHandler;
class UYapBroker;
class UFlowNode_YapDialogue;
struct FYapPromptHandle;
class UFlowAsset;
class IYapConversationHandler;
struct FYapBit;
class UYapCharacterComponent;
enum class EYapMaturitySetting : uint8;

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
	TOptional<FGameplayTag> Conversation;

public:
	TDelegate<void(const FGameplayTag&)> OnConversationOpens;
	
	TDelegate<void(const FGameplayTag&)> OnConversationCloses;

public:
	bool OpenConversation(UFlowAsset* InOwningAsset, const FGameplayTag& InName);

	bool CloseConversation();

	bool IsConversationInProgress() const { return Conversation.IsSet(); };

	const FGameplayTag& GetCurrentConversationName() const { return Conversation.Get(FGameplayTag::EmptyTag); }
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
friend class UFlowNode_YapConversation_Open;
friend class UFlowNode_YapConversation_Close;
friend struct FYapFragment;
friend struct FYapPromptHandle;
	
public:
	UYapSubsystem();
	
	// ------------------------------------------
	// STATE
	
protected:
	static TWeakObjectPtr<UWorld> World;
	
	// TODO I should have a queue system???... otherwise I'll have odd race conditions if there are multiple conversation requests!
	//UPROPERTY(Transient)
	//TArray<FName> ConversationQueue;
	
	/** The current active conversation. */
	UPROPERTY(Transient)
	FYapActiveConversation ActiveConversation;

	/** All registered conversation handlers. It is assumed developers will only have one or two of these at a time, no need for fast lookup. */
	UPROPERTY(Transient)
	TArray<UObject*> ConversationHandlers;

	/** The broker object. Active only during play. Editor work uses the CDO instead. */
	UPROPERTY(Transient)
	TObjectPtr<UYapBroker> Broker;
	
	/** Stores the tag of a fragment and the owning dialogue node where that fragment can be found */
	UPROPERTY(Transient)
	TMap<FGameplayTag, UFlowNode_YapDialogue*> TaggedFragments;

	/** Stores overrides of bit replacements. Currently, can only store one at a time per fragment; new assignments simply replace the old one. */
	UPROPERTY(Transient)
	TMap<FGameplayTag, FYapBitReplacement> BitReplacements;

	/** All registered character components. */
	UPROPERTY(Transient)
	TMap<FGameplayTag, TWeakObjectPtr<UYapCharacterComponent>> YapCharacterComponents;

	/** Helper to ensure that multiple character components are never registered for the same actor. */
	UPROPERTY(Transient)
	TSet<TObjectPtr<AActor>> RegisteredYapCharacterActors;

	// Running dialogue instances. Since only one fragment of a dialogue node can be running at any time, we don't need handles to map to individual fragments.
	UPROPERTY(Transient)
	TMap<FYapDialogueHandleRef, TWeakObjectPtr<UFlowNode_YapDialogue>> GuidDialogueMap;

	static bool bGetGameMaturitySettingWarningIssued;

	static FYapDialogueHandle InvalidHandle;
	// ------------------------------------------
	// PUBLIC API - Your game should use these
	
public:
	/** Register a conversation handler. Conversation handlers will receive yap dialogue events. Must implement IYapConversationHandler either in C++ or BP. */
	UFUNCTION(BlueprintCallable, Category = "Yap")
	void RegisterConversationHandler(UObject* NewHandler);

	/** Unregister a conversation handler. */
	UFUNCTION(BlueprintCallable, Category = "Yap")
	void UnregisterConversationHandler(UObject* RemovedHandler);
	
	/** Given a character identity tag, find the character component in the world. */
	UFUNCTION(BlueprintCallable, Category = "Yap")
	UYapCharacterComponent* FindCharacterComponent(FGameplayTag CharacterTag);

	// TODO I want a custom K2 node for this with IsValid outputs automatically
	//UFUNCTION(BlueprintCallable, Category = "Yap")
	//AActor* FindCharacterActor(FGameplayTag CharacterTag);
	
	// ------------------------------------------
	// YAP API - These are called by Yap classes
	
public:
	static const TWeakObjectPtr<UWorld> GetStaticWorld()
	{
		return World;
	}

	static UYapSubsystem* Get()
	{
		if (World.IsValid())
		{
			return World->GetSubsystem<UYapSubsystem>();
		}

		return nullptr;
	}
	
public:
	static UYapBroker* GetBroker();
	
	static EYapMaturitySetting GetCurrentMaturitySetting();

	/**  */
	FYapFragment* FindTaggedFragment(const FGameplayTag& FragmentTag);

protected:  // TODO should some of these be public?
	/**  */
	void RegisterTaggedFragment(const FGameplayTag& FragmentTag, UFlowNode_YapDialogue* DialogueNode);

	/**  */
	bool OpenConversation(UFlowAsset* OwningAsset, const FGameplayTag& ConversationName); // Called by Open Conversation node

	/**  */
	void CloseConversation(); // Called by Close Conversation node

	/**  */
	FYapPromptHandle BroadcastPrompt(UFlowNode_YapDialogue* Dialogue, uint8 FragmentIndex);

	/**  */
	void OnFinishedBroadcastingPrompts();
	
	/**  */
	void BroadcastDialogueStart(UFlowNode_YapDialogue* Dialogue, uint8 FragmentIndex); // Called by Dialogue node, 2nd output pin 

	/**  */
	void BroadcastDialogueEnd(const UFlowNode_YapDialogue* Dialogue, uint8 FragmentIndex); // Called by Dialogue node, 1st output pin

	/**  */
	void BroadcastPaddingTimeOver(const UFlowNode_YapDialogue* Dialogue, uint8 FragmentIndex);

public:
	// TODO should I make a ref struct for FYapPromptHandle too?
	/** The prompt handle will call this function, passing in itself. */
	static bool RunPrompt(const FYapPromptHandle& Handle);

	/**  */
	static bool SkipDialogue(const FYapDialogueHandleRef& Handle);

	static FYapDialogueHandle& GetDialogueHandle(FYapDialogueHandleRef HandleRef);

public:
	/**  */
	void RegisterCharacterComponent(UYapCharacterComponent* YapCharacterComponent);

	/**  */
	void UnregisterCharacterComponent(UYapCharacterComponent* YapCharacterComponent);
	
public:
	/**  */
	void Initialize(FSubsystemCollectionBase& Collection) override;

	/**  */
	void Deinitialize() override;

	/**  */
	void OnWorldBeginPlay(UWorld& InWorld) override;
	
protected:
	/**  */
	void OnConversationOpens_Internal(const FGameplayTag& ConversationName);

	/**  */
	void OnConversationCloses_Internal(const FGameplayTag& Name);

	/**  */
	bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

	// Thanks to Blue Man for template help
	template<auto TFunction, auto TExecFunction, typename... TArgs>
	void BroadcastConversationHandlerFunc(TArgs&&... Args)
	{
		bool bHandled = false;
	
		for (int i = 0; i < ConversationHandlers.Num(); ++i)
		{
			UObject* HandlerObj = ConversationHandlers[i];

			if (!IsValid(HandlerObj))
			{
				continue;
			}
		
			if (IYapConversationHandler* CppInterface = Cast<IYapConversationHandler>(HandlerObj))
			{
				(CppInterface->*TFunction)(Args...);
			}
			else
			{
				check(HandlerObj->Implements<UYapConversationHandler>());
				(*TExecFunction)(HandlerObj, Args...);				
			}
		
			bHandled = true;
		}
	
		if (!bHandled)
		{
			UE_LOG(LogYap, Error, TEXT("No Yap Conversation Listeners are currently registered! You must inherit a class from IYapConversationListeners, implement its functions, and register it to the Yap subsystem."));
		}
	}
};
