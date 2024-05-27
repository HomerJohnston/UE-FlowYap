#include "GraphNodes/FlowGraphNode_YapResponse.h"

#include "FlowYap/Nodes/FlowNode_YapResponse.h"

#include "Widgets/SFlowGraphNode_YapResponseWidget.h"

UFlowGraphNode_YapResponse::UFlowGraphNode_YapResponse()
{
	AssignedNodeClasses = {UFlowNode_YapResponse::StaticClass()};
}

TSharedPtr<SGraphNode> UFlowGraphNode_YapResponse::CreateVisualWidget()
{
	return SNew(SFlowGraphNode_YapResponseWidget, this);
}
