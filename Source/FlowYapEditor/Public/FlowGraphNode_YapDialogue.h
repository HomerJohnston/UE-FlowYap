#pragma once

#include "Graph/Nodes/FlowGraphNode.h"

#include "FlowGraphNode_YapDialogue.generated.h"

class UAkAudioEvent;

UCLASS()
class UFlowGraphNode_YapDialogue : public UFlowGraphNode
{
	GENERATED_BODY()

public:
	UFlowGraphNode_YapDialogue();
	
	TSharedPtr<SGraphNode> CreateVisualWidget() override;

	bool ShowPaletteIconOnNode() const override;

	FText GetTitleText() const;
	
	FText GetDialogueText() const;

	UAkAudioEvent* GetDialogueAudio() const;
};