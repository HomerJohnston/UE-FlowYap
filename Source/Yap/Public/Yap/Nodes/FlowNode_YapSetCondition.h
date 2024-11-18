#pragma once
#include "Nodes/FlowNode.h"

#include "FlowNode_YapSetCondition.generated.h"

UCLASS()
class YAP_API UFlowNode_YapSetCondition : public UFlowNode
{
	GENERATED_BODY()

public:
	UFlowNode_YapSetCondition();

public:
	UPROPERTY(EditAnywhere)
	FGameplayTag Condition;
	
public:
	FString GetNodeCategory() const override;

	FText GetNodeTitle() const override;

	FString GetNodeDescription() const override;
};