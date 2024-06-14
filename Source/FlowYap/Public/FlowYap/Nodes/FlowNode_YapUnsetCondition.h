#pragma once
#include "Nodes/FlowNode.h"

#include "FlowNode_YapUnsetCondition.generated.h"

UCLASS()
class FLOWYAP_API UFlowNode_YapUnsetCondition : public UFlowNode
{
	GENERATED_BODY()

public:
	UFlowNode_YapUnsetCondition();

public:
	UPROPERTY(EditAnywhere)
	FGameplayTag Condition;
	
public:
	FString GetNodeCategory() const override;

	FText GetNodeTitle() const override;

	FString GetNodeDescription() const override;
};