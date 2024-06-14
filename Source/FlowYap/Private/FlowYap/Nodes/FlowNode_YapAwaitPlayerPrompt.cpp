#include "FlowYap/Nodes/FlowNode_YapAwaitPlayerPrompt.h"

#include "FlowYap/Nodes/Category.h"

UFlowNode_YapAwaitPlayerPrompt::UFlowNode_YapAwaitPlayerPrompt()
{
}

FString UFlowNode_YapAwaitPlayerPrompt::GetNodeCategory() const
{
	return FlowYapRuntime::NodeCategory;
}

FText UFlowNode_YapAwaitPlayerPrompt::GetNodeTitle() const
{
	return Super::GetNodeTitle();
}

FText UFlowNode_YapAwaitPlayerPrompt::GetNodeToolTip() const
{
	return Super::GetNodeToolTip();
}

void UFlowNode_YapAwaitPlayerPrompt::InitializeInstance()
{
	Super::InitializeInstance();
}

void UFlowNode_YapAwaitPlayerPrompt::OnActivate()
{
	Super::OnActivate();
}
