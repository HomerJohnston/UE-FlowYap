#pragma once

#include "FlowGraphNode_YapSpeechBase.h"

#include "FlowGraphNode_YapResponse.generated.h"

UCLASS()
class UFlowGraphNode_YapResponse : public UFlowGraphNode_YapSpeechBase
{
	GENERATED_BODY()

public:
	UFlowGraphNode_YapResponse();
	
	TSharedPtr<SGraphNode> CreateVisualWidget() override;

};