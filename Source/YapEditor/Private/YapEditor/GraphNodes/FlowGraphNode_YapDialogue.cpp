// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "YapEditor/GraphNodes/FlowGraphNode_YapDialogue.h"

#include "Graph/FlowGraph.h"
#include "Yap/Nodes/FlowNode_YapDialogue.h"
#include "YapEditor/YapColors.h"
#include "YapEditor/YapEditorEventBus.h"
#include "YapEditor/YapEditorEvents.h"
#include "YapEditor/NodeWidgets/SFlowGraphNode_YapDialogueWidget.h"

#define LOCTEXT_NAMESPACE "YapEditor"

UFlowGraphNode_YapDialogue::UFlowGraphNode_YapDialogue()
{
	AssignedNodeClasses = {UFlowNode_YapDialogue::StaticClass()};

	EventBus << YapEditor::Events::DialogueNode::Test;
}

TSharedPtr<SGraphNode> UFlowGraphNode_YapDialogue::CreateVisualWidget()
{
	return SNew(SFlowGraphNode_YapDialogueWidget, this);
}

bool UFlowGraphNode_YapDialogue::ShowPaletteIconOnNode() const
{
	return true;
}

UFlowNode_YapDialogue* UFlowGraphNode_YapDialogue::GetFlowYapNode() const
{
	return Cast<UFlowNode_YapDialogue>(GetFlowNodeBase());
}

FLinearColor UFlowGraphNode_YapDialogue::GetNodeBodyTintColor() const
{
	return YapColor::LightGray;
}

FSlateIcon UFlowGraphNode_YapDialogue::GetIconAndTint(FLinearColor& OutColor) const
{
	return FSlateIcon();
}

#undef LOCTEXT_NAMESPACE