#pragma once
#include "Nodes/FlowNode.h"

#include "FlowNode_YapConversationEnd.generated.h"

UCLASS(NotBlueprintable, meta = (DisplayName = "Conversation End", Keywords = "event"))
class FLOWYAP_API UFlowNode_YapConversationEnd : public UFlowNode
{
	GENERATED_BODY()

public:
	UFlowNode_YapConversationEnd() { Category = TEXT("YapTEST"); }
	
#if WITH_EDITOR
	bool SupportsContextPins() const override { return true; };

	TArray<FFlowPin> GetContextInputs() override { return {1,2,3,4};};
	
	bool CanRefreshContextPinsOnLoad() const override { return true; };
#endif
};