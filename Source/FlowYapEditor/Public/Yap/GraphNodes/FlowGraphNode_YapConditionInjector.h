#pragma once
#include "Graph/Nodes/FlowGraphNode.h"

#include "FlowGraphNode_YapConditionInjector.generated.h"

UCLASS()
class UFlowGraphNode_YapConditionInjector : public UFlowGraphNode
{
	GENERATED_BODY()

public:
	UFlowGraphNode_YapConditionInjector();
	
public:
	TSharedPtr<SGraphNode> CreateVisualWidget() override;
};