#pragma once
#include "Nodes/Route/FlowNode_CustomInput.h"

#include "FlowNode_YapGotoTarget.generated.h"

UCLASS(DisplayName = "Goto Out")
class UFlowNode_YapGotoTarget : public UFlowNode_CustomInput
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	FName Name;

public:
	virtual void PostInitProperties() override;
};