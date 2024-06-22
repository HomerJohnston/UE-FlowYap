

#include "Yap/Nodes/FlowNode_ConditionInjector.h"

#include "Yap/FlowYapUtil.h"

UFlowNode_ConditionInjector::UFlowNode_ConditionInjector()
{	
	//InputPins.Empty();
}

FString UFlowNode_ConditionInjector::GetNodeCategory() const
{
	return FlowYapUtil::NodeCategory;
}

FText UFlowNode_ConditionInjector::GetNodeTitle() const
{
	return INVTEXT("Condition Injector");
}

FText UFlowNode_ConditionInjector::GetNodeToolTip() const
{
	return Super::GetNodeToolTip();
}

void UFlowNode_ConditionInjector::InitializeInstance()
{
	Super::InitializeInstance();

	if (IsInputConnected(DefaultInputPin.PinName))
	{
		return;
	}
	
	for (UFlowNode* ConnectedNode : GetConnectedNodes())
	{
		
	}
}

void UFlowNode_ConditionInjector::OnActivate()
{
	TriggerFirstOutput(true);
}
