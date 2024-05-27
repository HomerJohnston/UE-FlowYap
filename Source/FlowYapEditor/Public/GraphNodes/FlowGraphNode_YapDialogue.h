#pragma once

#include "FlowGraphNode_YapSpeechBase.h"

#include "FlowGraphNode_YapDialogue.generated.h"

UCLASS()
class UFlowGraphNode_YapDialogue : public UFlowGraphNode_YapSpeechBase
{
	GENERATED_BODY()

public:
	UFlowGraphNode_YapDialogue();
	
	TSharedPtr<SGraphNode> CreateVisualWidget() override;
};