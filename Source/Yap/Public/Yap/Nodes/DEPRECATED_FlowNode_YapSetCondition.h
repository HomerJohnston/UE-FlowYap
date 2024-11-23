#pragma once
#include "Nodes/FlowNode.h"

#include "DEPRECATED_FlowNode_YapSetCondition.generated.h"

// This should get replaced by a separate "facts" plugin
UCLASS(Deprecated)
class YAP_API UDEPRECATED_FlowNode_YapSetCondition : public UFlowNode
{
	GENERATED_BODY()

public:
	UDEPRECATED_FlowNode_YapSetCondition();

public:
	UPROPERTY(EditAnywhere)
	FGameplayTag Condition;

#if WITH_EDITOR
public:
	FString GetNodeCategory() const override;

	FText GetNodeTitle() const override;

	FString GetNodeDescription() const override;
#endif
};