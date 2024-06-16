
#include "Yap/Nodes/FlowNode_DynamicPrompt.h"

UFlowNode_DynamicPrompt::UFlowNode_DynamicPrompt()
{
	OutputPins.Empty();
}

bool UFlowNode_DynamicPrompt::SupportsContextPins() const
{
	return true;
}

TArray<FFlowPin> UFlowNode_DynamicPrompt::GetContextOutputs()
{
	TArray<FFlowPin> Outputs;

	for (FName PromptName : Prompts)
	{
		Outputs.Add(PromptName);
	}

	return Outputs;
}
