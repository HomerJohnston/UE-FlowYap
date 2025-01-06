// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once

#include "Nodes/FlowNode.h"

#include "FlowNode_YapConversationEnd.generated.h"

UCLASS(NotBlueprintable, meta = (DisplayName = "Yap Conversation End", Keywords = "yap"))
class YAP_API UFlowNode_YapConversationEnd : public UFlowNode
{
	GENERATED_BODY()

	UPROPERTY()
	FName ConversationName;
	
public:
	UFlowNode_YapConversationEnd();

	void ExecuteInput(const FName& PinName) override;
#if WITH_EDITOR
	FText GetNodeTitle() const override;
#endif
};