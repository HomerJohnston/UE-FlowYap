#pragma once

#include "Graph/Nodes/FlowGraphNode.h"

#include "FlowGraphNode_YapDialogue.generated.h"

class UAkAudioEvent;
class SGraphNode;
struct FFlowYapFragment;

UCLASS()
class UFlowGraphNode_YapDialogue : public UFlowGraphNode
{
	GENERATED_BODY()

public:
	UFlowGraphNode_YapDialogue();

public:
	TSharedPtr<SGraphNode> CreateVisualWidget() override;

	bool ShowPaletteIconOnNode() const override;
};