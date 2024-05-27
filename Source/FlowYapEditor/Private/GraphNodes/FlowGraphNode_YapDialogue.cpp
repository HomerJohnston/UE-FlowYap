#include "GraphNodes/FlowGraphNode_YapDialogue.h"

#include "Widgets/SFlowGraphNode_YapDialogueWidget.h"

#include "FlowYap/Nodes/FlowNode_YapDialogue.h"

UFlowGraphNode_YapDialogue::UFlowGraphNode_YapDialogue()
{
	AssignedNodeClasses = {UFlowNode_YapDialogue::StaticClass()};
}

TSharedPtr<SGraphNode> UFlowGraphNode_YapDialogue::CreateVisualWidget()
{
	return SNew(SFlowGraphNode_YapDialogueWidget, this);
}
