// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once
#include "GameplayTagContainer.h"
#include "YapCharacterComponent.h"
#include "YapBroker.h"
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
	
protected:
	// TODO I should have a queue system???... otherwise I'll have odd race conditions on multiple conversation requests
	//UPROPERTY(Transient)
	//TArray<FName> ConversationQueue;
	
	/**  */
	UPROPERTY(Transient)
	FYapActiveConversation ActiveConversation;

	/**  */
	UPROPERTY(Transient)
	TArray<UObject*> ConversationHandlers;

	UPROPERTY(Transient)
	TObjectPtr<UYapBroker> Broker;
	
	/** Stores the tag of a fragment and the owning dialogue node where that fragment can be found */
	UPROPERTY(Transient)
	TMap<FGameplayTag, UFlowNode_YapDialogue*> TaggedFragments;

	/** Stores overrides of bit replacements. Can only store one at a time, new assignments simply replace the old one. */
	UPROPERTY(Transient)
	TMap<FGameplayTag, FYapBitReplacement> BitReplacements;

	/**  */
	UPROPERTY(Transient)
	TMap<FGameplayTag, TWeakObjectPtr<UYapCharacterComponent>> YapCharacterComponents;

	/**  */
	UPROPERTY(Transient)
	TSet<TObjectPtr<AActor>> RegisteredYapCharacterActors;

	UPROPERTY(Transient)
	TSubclassOf<UObject> BrokerClass;

	// Running dialogue instances. Since only one fragment of a dialogue node can be running at any time, we don't need handles to map to individual fragments.
	UPROPERTY(Transient)
	TMap<TObjectPtr<UFlowNode_YapDialogue>, FYapDialogueHandle> DialogueHandles;

	static bool bGetGameMaturitySettingWarningIssued;
	
	// ------------------------------------------
	// PUBLIC API
public:
	/** When a conversation starts */
	UFUNCTION(BlueprintCallable)
	void RegisterConversationHandler(UObject* NewHandler);

	/**  */
	UFUNCTION(BlueprintCallable)
	void UnregisterConversationHandler(UObject* RemovedHandler);

	UFUNCTION(BlueprintCallable)
	UYapCharacterComponent* GetYapCharacter(const FGameplayTag& CharacterTag);

	UFUNCTION(BlueprintCallable, DisplayName = "Get Maturity Setting")
	EYapMaturitySetting K2_GetMaturitySetting() { return GetGameMaturitySetting(); };

	static UYapBroker* GetBroker();
	
	static EYapMaturitySetting GetGameMaturitySetting();

	// ------------------------------------------
	// YAP API - These are called by Yap classes
public:
	/**  */
	FYapFragment* FindTaggedFragment(const FGameplayTag& FragmentTag);

protected:
	/**  */
	void RegisterTaggedFragment(const FGameplayTag& FragmentTag, UFlowNode_YapDialogue* DialogueNode);

	/**  */
	bool OpenConversation(UFlowAsset* OwningAsset, const FGameplayTag& ConversationName); // Called by Open Conversation node

	/**  */
	void CloseConversation(); // Called by Close Conversation node

	/**  */
	void BroadcastPrompt(UFlowNode_YapDialogue* Dialogue, uint8 FragmentIndex);

	/**  */
	void OnFinishedBroadcastingPrompts();
	
	/**  */
	void BroadcastDialogueStart(UFlowNode_YapDialogue* DialogueNode, uint8 FragmentIndex); // Called by Dialogue node, 2nd output pin 

	/**  */
	void BroadcastDialogueEnd(const UFlowNode_YapDialogue* OwnerDialogue, uint8 FragmentIndex); // Called by Dialogue node, 1st output pin

	/**  */
	void BroadcastPaddingTimeOver(const UFlowNode_YapDialogue* OwnerDialogue, uint8 FragmentIndex);
	
	/**  */
	UFUNCTION(BlueprintCallable)
	void RunPrompt(const FYapPromptHandle& Handle);

public:
	/**  */
	void RegisterCharacterComponent(UYapCharacterComponent* YapCharacterComponent);

	/**  */
	void UnregisterCharacterComponent(UYapCharacterComponent* YapCharacterComponent);
	
public:
	/**  */
	void Initialize(FSubsystemCollectionBase& Collection) override;

	void Deinitialize() override;
	
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
			UE_LOG(LogYap, Error, TEXT("IYapConversationListener and register it to the Yap subsystem."));
		}
	}
};
