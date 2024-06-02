#pragma once

#include "Graph/Nodes/FlowGraphNode.h"

#include "FlowGraphNode_YapDialogue.generated.h"

class UFlowNode_YapDialogue;
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

	void GetNodeContextMenuActions(class UToolMenu* Menu, class UGraphNodeContextMenuContext* Context) const override;

	UFlowNode_YapDialogue* GetFlowYapNode() const;

	FLinearColor GetNodeBodyTintColor() const override;

	FSlateIcon GetIconAndTint(FLinearColor& OutColor) const override;
};