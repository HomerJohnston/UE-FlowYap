// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once

#include "Nodes/FlowNode.h"

#include "FlowNode_YapConversation_Open.generated.h"

UCLASS(NotBlueprintable, meta = (DisplayName = "Yap Conversation Start", Keywords = "yap"))
class YAP_API UFlowNode_YapConversation_Open : public UFlowNode
{
	GENERATED_BODY()
public:
	UFlowNode_YapConversation_Open();
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag Conversation;

	void InitializeInstance() override;

	void IterateDownstreamNodes(UFlowNode* DownstreamNode, TArray<UFlowNode*>& ConnectedNodes);
	
	void OnActivate() override;

	void ExecuteInput(const FName& PinName) override;
	
#if WITH_EDITOR
public:
	FText GetNodeTitle() const override;

	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
