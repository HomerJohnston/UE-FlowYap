#pragma once

#include "IFlowYapConversationListener.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UFlowYapConversationListener : public UInterface
{
	GENERATED_BODY()
};

class IFlowYapConversationListener
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void OnConversationStarts(FName ConversationName);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void OnConversationEnds(FName ConversationName);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void OnDialogueStart(FName ConversationName, const FFlowYapBit& DialogueInfo);
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void OnDialogueEnd(FName ConversationName, const FFlowYapBit& DialogueInfo);
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void OnDialogueInterrupt(FName ConversationName, const FFlowYapBit& DialogueInfo);
};