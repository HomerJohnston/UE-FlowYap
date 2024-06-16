#pragma once
#include "Nodes/FlowNode.h"

#include "FlowNode_TopicTagInjector.generated.h"

UCLASS()
class FLOWYAP_API UFlowNode_TopicTagInjector : public UFlowNode
{
	GENERATED_BODY()

public:
	UFlowNode_TopicTagInjector();
	
	virtual FString GetNodeCategory() const;
	virtual FText GetNodeTitle() const;
	virtual FText GetNodeToolTip() const;

public:
	void InitializeInstance() override;

	void OnActivate() override;
};