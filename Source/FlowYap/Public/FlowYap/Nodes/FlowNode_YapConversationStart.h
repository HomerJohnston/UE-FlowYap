#pragma once

#include "Nodes/FlowNode.h"

#include "FlowNode_YapConversationStart.generated.h"

UCLASS(NotBlueprintable, meta = (DisplayName = "Conversation Start", Keywords = "yap"))
class FLOWYAP_API UFlowNode_YapConversationStart : public UFlowNode
{
	GENERATED_BODY()
public:
	UFlowNode_YapConversationStart();
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName Name;

	void InitializeInstance() override;

	void IterateDownstreamNodes(UFlowNode* DownstreamNode, TArray<UFlowNode*>& ConnectedNodes);
	
	void OnActivate() override;

	void ExecuteInput(const FName& PinName) override;
	
#if WITH_EDITOR
	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
