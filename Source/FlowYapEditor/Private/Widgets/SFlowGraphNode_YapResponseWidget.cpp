#include "Widgets/SFlowGraphNode_YapResponseWidget.h"

#include "FlowYap/Nodes/FlowNode_YapResponse.h"

TSharedRef<SBox> SFlowGraphNode_YapResponseWidget::GetAdditionalOptionsWidget()
{
	TSharedPtr<SBox> Box;

	SAssignNew(Box, SBox);

	return Box.ToSharedRef();
}

void SFlowGraphNode_YapResponseWidget::Construct(const FArguments& InArgs, UFlowGraphNode* InNode)
{
	SFlowGraphNode_YapSpeechBaseWidget::Construct(InArgs, InNode);
	
	FlowNode_YapResponse = Cast<UFlowNode_YapResponse>(FlowGraphNode_YapSpeechBase->GetFlowNode());
	
	UpdateGraphNode();
}
