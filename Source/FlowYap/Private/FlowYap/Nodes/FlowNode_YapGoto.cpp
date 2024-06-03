#include "FlowYap/Nodes/FlowNode_YapGoto.h"

#include "FlowAsset.h"

UFlowNode_YapGoto::UFlowNode_YapGoto()
{
#if WITH_EDITOR
	NodeStyle = EFlowNodeStyle::InOut;
#endif
	OutputPins = {};
}

void UFlowNode_YapGoto::OnActivate()
{
	GetFlowAsset()->TriggerCustomInput(Target);
}
