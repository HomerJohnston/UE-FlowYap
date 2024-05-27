#pragma once

#include "FlowYap/Nodes/FlowNode_YapDialogue.h"

#include "FlowNode_YapResponse.generated.h"

UCLASS(NotBlueprintable, meta = (DisplayName = "Response", Keywords = "event"))
class FLOWYAP_API UFlowNode_YapResponse : public UFlowNode_YapSpeechBase
{
	GENERATED_BODY()

public:
	UFlowNode_YapResponse();
};