#pragma once

#include "Graph/Nodes/FlowGraphNode.h"

#include "FlowGraphNode_YapSpeechBase.generated.h"

class UAkAudioEvent;

UCLASS()
class UFlowGraphNode_YapSpeechBase : public UFlowGraphNode
{
	GENERATED_BODY()

public:
	bool ShowPaletteIconOnNode() const override;

	FText GetTitleText() const;
	
	FText GetDialogueText() const;

	UAkAudioEvent* GetDialogueAudio() const;
};