#include "FlowYap/Nodes/FlowNode_YapGotoTarget.h"

void UFlowNode_YapGotoTarget::PostInitProperties()
{
	EventName = Name;
	Super::PostInitProperties();
}
