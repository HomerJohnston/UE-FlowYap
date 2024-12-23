#pragma once

#include "Graph/Nodes/FlowGraphNode.h"

#include "FlowGraphNode_YapDialogue.generated.h"

class UFlowNode_YapDialogue;
class UAkAudioEvent;
class SGraphNode;
struct FYapFragment;

UCLASS()
class UFlowGraphNode_YapDialogue : public UFlowGraphNode
{
	GENERATED_BODY()

public:
	UFlowGraphNode_YapDialogue();

public:
	FFlowGraphNodeEvent OnYapNodeChanged;

protected:
	// Fragment GUID, array of pin names for this fragment - basically only used for fragment deletion
	TMap<FGuid, TArray<FName>> FragmentPins;
	
public:	
	TSharedPtr<SGraphNode> CreateVisualWidget() override;

	bool ShowPaletteIconOnNode() const override;

	UFlowNode_YapDialogue* GetFlowYapNode() const;

	FLinearColor GetNodeBodyTintColor() const override;

	FSlateIcon GetIconAndTint(FLinearColor& OutColor) const override;
};
