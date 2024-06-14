#pragma once
#include "Nodes/FlowNode.h"

#include "FlowNode_YapAwaitPlayerPrompt.generated.h"

UCLASS()
class FLOWYAP_API UFlowNode_YapAwaitPlayerPrompt : public UFlowNode
{
	GENERATED_BODY()

public:
	UFlowNode_YapAwaitPlayerPrompt();

public:
	virtual FString GetNodeCategory() const;
	virtual FText GetNodeTitle() const;
	virtual FText GetNodeToolTip() const;

public:
	void InitializeInstance() override;

	void OnActivate() override;
};