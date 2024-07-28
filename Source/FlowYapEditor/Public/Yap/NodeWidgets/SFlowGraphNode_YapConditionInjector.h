#pragma once

#include "Graph/Nodes/FlowGraphNode.h"
#include "Graph/Widgets/SFlowGraphNode.h"

class SFlowGraphNode_YapConditionInjector : public SFlowGraphNode
{
public:
	SFlowGraphNode_YapConditionInjector();
	
	void Construct(const FArguments& InArgs);
	void Construct(const FArguments& InArgs, UFlowGraphNode* InNode);

	void UpdateGraphNode() override;
};
