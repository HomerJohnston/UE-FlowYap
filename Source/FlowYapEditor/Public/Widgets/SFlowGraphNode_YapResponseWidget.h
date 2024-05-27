#pragma once

#pragma once

#include "CoreMinimal.h"
#include "SFlowGraphNode_YapSpeechBaseWidget.h"
#include "GraphNodes/FlowGraphNode_YapResponse.h"
#include "Graph/Widgets/SFlowGraphNode.h"
#include "Widgets/SCompoundWidget.h"

class UFlowNode_YapResponse;

class SFlowGraphNode_YapResponseWidget : public SFlowGraphNode_YapSpeechBaseWidget
{
	TSharedRef<SBox> GetAdditionalOptionsWidget() override;

protected:
	UFlowNode_YapResponse* FlowNode_YapResponse = nullptr;
	
public:
	void Construct(const FArguments& InArgs, UFlowGraphNode* InNode);
};
