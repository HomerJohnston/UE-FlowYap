#pragma once

struct FGameplayTag;
struct FYapPromptHandle;
struct FFlowYapBit;

#include "Yap/YapPromptHandle.h"

#include "FlowYapConversationHandler.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UFlowYapConversationHandler : public UInterface
{
	GENERATED_BODY()
};

class IFlowYapConversationHandler
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void OnConversationStarts(const FGameplayTag& Conversation);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void OnConversationEnds(const FGameplayTag& Conversation);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void OnDialogueStart(const FGameplayTag& Conversation, const FFlowYapBit& DialogueInfo);
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void OnDialogueEnd(const FGameplayTag& Conversation, const FFlowYapBit& DialogueInfo);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void AddPrompt(const FGameplayTag& Conversation, const FFlowYapBit& DialogueInfo, FYapPromptHandle Handle); // TODO THIS SHOULD PASS THE HANDLE BY VALUE!!!
};
