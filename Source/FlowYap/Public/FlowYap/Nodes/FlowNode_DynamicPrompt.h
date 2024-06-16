#pragma once
#include "Nodes/FlowNode.h"

#include "FlowNode_DynamicPrompt.generated.h"

UCLASS()
class FLOWYAP_API UFlowNode_DynamicPrompt : public UFlowNode
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere)
	TArray<FName> Prompts;
	
public:
	UFlowNode_DynamicPrompt();

	bool SupportsContextPins() const override;
	
	virtual TArray<FFlowPin> GetContextOutputs() override;
};