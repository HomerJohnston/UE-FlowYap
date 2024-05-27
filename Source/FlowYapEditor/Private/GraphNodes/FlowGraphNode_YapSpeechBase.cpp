#include "GraphNodes/FlowGraphNode_YapSpeechBase.h"

#include "FlowYap/Nodes/FlowNode_YapSpeechBase.h"
#include "Widgets/SFlowGraphNode_YapSpeechBaseWidget.h"

bool UFlowGraphNode_YapSpeechBase::ShowPaletteIconOnNode() const
{
	return true;
}

FText UFlowGraphNode_YapSpeechBase::GetTitleText() const
{
	UFlowNode_YapSpeechBase* YapSpeechBase = Cast<UFlowNode_YapSpeechBase>(GetFlowNode());

	check(YapSpeechBase);
	
	return YapSpeechBase->GetTitleText();
}

FText UFlowGraphNode_YapSpeechBase::GetDialogueText() const
{
	UFlowNode_YapSpeechBase* YapSpeechBase = Cast<UFlowNode_YapSpeechBase>(GetFlowNode());

	check(YapSpeechBase);
	
	return YapSpeechBase->GetDialogueText();
}

UAkAudioEvent* UFlowGraphNode_YapSpeechBase::GetDialogueAudio() const
{
	UFlowNode_YapSpeechBase* YapSpeechBase = Cast<UFlowNode_YapSpeechBase>(GetFlowNode());

	check(YapSpeechBase);
	
	return YapSpeechBase->GetDialogueAudio();
}
