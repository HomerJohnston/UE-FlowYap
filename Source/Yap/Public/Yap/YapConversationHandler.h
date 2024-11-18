#pragma once

struct FGameplayTag;
struct FYapPromptHandle;
struct FYapBit;

#include "Yap/YapPromptHandle.h"

#include "YapConversationHandler.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UYapConversationHandler : public UInterface
{
	GENERATED_BODY()
};

class IYapConversationHandler
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void OnConversationStarts(const FGameplayTag& Conversation);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void OnConversationEnds(const FGameplayTag& Conversation);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void OnDialogueStart(const FGameplayTag& Conversation, const FYapBit& DialogueInfo);
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void OnDialogueEnd(const FGameplayTag& Conversation, const FYapBit& DialogueInfo);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void AddPrompt(const FGameplayTag& Conversation, const FYapBit& DialogueInfo, FYapPromptHandle Handle); // TODO THIS SHOULD PASS THE HANDLE BY VALUE!!!
};
