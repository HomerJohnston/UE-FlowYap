#pragma once
#include "Nodes/FlowNode.h"

#include "FlowNode_YapGoto.generated.h"

class UFlowNode;
class UFlowGraphNode;

UCLASS(NotBlueprintable, meta = (DisplayName = "Goto In", Keywords = "yap"))
class FLOWYAP_API UFlowNode_YapGoto : public UFlowNode
{
	GENERATED_BODY()

public:
	UFlowNode_YapGoto();

	UPROPERTY(EditAnywhere)
	FName Target;

	void OnActivate() override;
};