#include "FlowGraphNode_YapDialogue.h"

#include "SFlowGraphNode_YapDialogue.h"
#include "FlowYap/Nodes/FlowNode_YapDialogue.h"

UFlowGraphNode_YapDialogue::UFlowGraphNode_YapDialogue()
{
	AssignedNodeClasses = {UFlowNode_YapDialogue::StaticClass()};
}

TSharedPtr<SGraphNode> UFlowGraphNode_YapDialogue::CreateVisualWidget()
{
	return SNew(SFlowGraphNode_YapDialogue, this);
}

bool UFlowGraphNode_YapDialogue::ShowPaletteIconOnNode() const
{
	return true;
}

FText UFlowGraphNode_YapDialogue::GetTitleText() const
{
	UFlowNode_YapDialogue* YapDialogue = Cast<UFlowNode_YapDialogue>(GetFlowNode());

	check(YapDialogue);
	
	return YapDialogue->GetTitleText();
}

FText UFlowGraphNode_YapDialogue::GetDialogueText() const
{
	UFlowNode_YapDialogue* YapDialogue = Cast<UFlowNode_YapDialogue>(GetFlowNode());

	check(YapDialogue);
	
	return YapDialogue->GetDialogueText();
}

UAkAudioEvent* UFlowGraphNode_YapDialogue::GetDialogueAudio() const
{
	UFlowNode_YapDialogue* YapDialogue = Cast<UFlowNode_YapDialogue>(GetFlowNode());

	check(YapDialogue);
	
	return YapDialogue->GetDialogueAudio();
}
