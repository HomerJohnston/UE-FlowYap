#include "Yap/GraphNodes/FlowGraphNode_YapConditionInjector.h"

#include "Yap/Nodes/FlowNode_ConditionInjector.h"
#include "Yap/NodeWidgets/SFlowGraphNode_YapConditionInjector.h"

UFlowGraphNode_YapConditionInjector::UFlowGraphNode_YapConditionInjector()
{
	AssignedNodeClasses = {UFlowNode_ConditionInjector::StaticClass()};
}

TSharedPtr<SGraphNode> UFlowGraphNode_YapConditionInjector::CreateVisualWidget()
{
	return SNew(SFlowGraphNode_YapConditionInjector);
}
