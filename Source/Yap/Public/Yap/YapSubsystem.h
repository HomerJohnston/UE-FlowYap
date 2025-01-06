// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once
#include "GameplayTagContainer.h"
#include "YapCharacterComponent.h"
#include "YapConversationBrokerBase.h"
#include "Enums/YapMaturitySetting.h"
#include "Yap/YapBitReplacement.h"

#include "YapSubsystem.generated.h"

class UYapConversationBrokerBase;
class UFlowNode_YapDialogue;
struct FYapPromptHandle;
class UFlowAsset;
class IYapConversationListener;
struct FYapBit;
class UYapTextCalculator;
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
	TArray<UObject*> Listeners;

	UPROPERTY(Transient)
	TObjectPtr<UYapConversationBrokerBase> ConversationBroker;
	
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
	TArray<UClass*> DialogueAudioAssetClasses;

	/**  */
	UPROPERTY(Transient)
	TMap<FGameplayTag, TWeakObjectPtr<UYapCharacterComponent>> YapCharacterComponents;

	/**  */
	UPROPERTY(Transient)
	TSet<TObjectPtr<AActor>> RegisteredYapCharacterActors;

	UPROPERTY(Transient)
	TSubclassOf<UObject> ConversationBrokerClass;

	// TODO running dialog interruption
	//UPROPERTY(Transient)
	//TMap<FYapDialogueHandle, 
	
	// ------------------------------------------
	// PUBLIC API
public:
	/** When a conversation starts */
	UFUNCTION(BlueprintCallable)
	void RegisterConversationListener(UObject* NewListener);

	/**  */
	UFUNCTION(BlueprintCallable)
	void UnregisterConversationListener(UObject* RemovedListener);

	UFUNCTION(BlueprintCallable)
	UYapCharacterComponent* GetYapCharacter(const FGameplayTag& CharacterTag);

	UFUNCTION(BlueprintCallable, DisplayName = "Get Maturity Setting")
	EYapMaturitySetting K2_GetMaturitySetting() { return GetMaturitySetting(); };

	static UYapConversationBrokerBase* GetConversationBroker();
	
	static EYapMaturitySetting GetMaturitySetting();
	
	// ------------------------------------------
	// YAP API - These are called by Yap classes
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
	void OnFinishedBroadcastingPrompts();
	
	/**  */
	void BroadcastDialogueStart(UFlowNode_YapDialogue* Dialogue, uint8 FragmentIndex); // Called by Dialogue node, 2nd output pin 

	/**  */
	void BroadcastDialogueEnd(const UFlowNode_YapDialogue* OwnerDialogue, uint8 FragmentIndex); // Called by Dialogue node, 1st output pin

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
	void OnConversationStarts_Internal(const FGameplayTag& ConversationName);

	/**  */
	void OnConversationEnds_Internal(const FGameplayTag& Name);

	/**  */
	bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

	//#define FUNCTION_REF(Class, FunctionRef) Class, &Class::FunctionRef, &Class::Execute_##FunctionRef
	
	template<auto TFunction, auto TExecFunction, typename... TArgs>
	void BroadcastBrokerListenerFuncs(TArgs&&... Args)
	{
		bool bHandled = false;
		
		if (IsValid(ConversationBroker))
		{
			(ConversationBroker->*TFunction)(Args...);
			bHandled = true;
		}

		for (int i = 0; i < Listeners.Num(); ++i)
		{
			UObject* Listener = Listeners[i];

			if (!IsValid(Listener))
			{
				continue;
			}

			(*TExecFunction)(Listener, Args...);
			bHandled = true;
		}

		if (!bHandled)
		{
			UE_LOG(LogYap, Error, TEXT("Yap has no conversation broker or event listeners registered! You must either write a C++ broker and set it in project settings, or create a class implementing IYapConversationListenerInterface and register it to the Yap subsystem."));
		}
	}
	
	template<auto TFunction, auto TExecFunction, typename R, typename... TArgs>
	R ExecuteBrokerListenerFuncs(TArgs&&... Args)
	{
		if (IsValid(ConversationBroker))
		{
			return (ConversationBroker->*TFunction)(Args...);
		}

		for (int i = 0; i < Listeners.Num(); ++i)
		{
			UObject* Listener = Listeners[i];

			if (!IsValid(Listener))
			{
				continue;
			}

			return (*TExecFunction)(Listener, Args...);
		}

		UE_LOG(LogYap, Error, TEXT("Yap has no conversation broker or event listeners registered! You must either write a C++ broker and set it in project settings, or create a class implementing IYapConversationListenerInterface and register it to the Yap subsystem."));

		return R{};
	}
};
