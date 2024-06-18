#pragma once
#include "Nodes/FlowNode.h"

#include "FlowNode_ConditionInjector.generated.h"

UCLASS(meta = (DisplayName="Yap Condition Injector"))
class FLOWYAP_API UFlowNode_ConditionInjector : public UFlowNode
{
	GENERATED_BODY()

public:
	UFlowNode_ConditionInjector();
	
	virtual FString GetNodeCategory() const;
	virtual FText GetNodeTitle() const;
	virtual FText GetNodeToolTip() const;

public:
	void InitializeInstance() override;

	void OnActivate() override;
};