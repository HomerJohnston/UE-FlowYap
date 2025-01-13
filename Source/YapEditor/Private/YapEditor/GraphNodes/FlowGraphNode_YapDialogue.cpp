// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "YapEditor/GraphNodes/FlowGraphNode_YapDialogue.h"

#include "Graph/FlowGraph.h"
#include "UObject/ObjectSaveContext.h"
#include "Yap/Nodes/FlowNode_YapDialogue.h"
#include "YapEditor/YapColors.h"
#include "YapEditor/YapEditorEventBus.h"
#include "YapEditor/YapEditorEvents.h"
#include "YapEditor/YapLogEditor.h"
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

void UFlowGraphNode_YapDialogue::PreSave(FObjectPreSaveContext SaveContext)
{
	Super::PreSave(SaveContext);

	UFlowNode_YapDialogue* Node = GetFlowYapNode();

	TMap<FYapFragment*, TArray<FName>> FragmentOptionalPins;
	FragmentOptionalPins.Reserve(Node->GetNumFragments());

	// TODO transaction here
	
	bool bDirty = false;
	
	for (FYapFragment& Fragment : Node->GetFragmentsMutable())
	{
		if (Fragment.UsesStartPin())
		{
			FName PinName = Fragment.GetStartPinName();

			if (Pins.ContainsByPredicate([PinName] (UEdGraphPin* PinEntry) { return !PinEntry->HasAnyConnections() && PinEntry->PinName == PinName; } ))
			{
				Fragment.ResetStartPin();
				bDirty = true;
			}
		}
		
		if (Fragment.UsesEndPin())
		{
			FName PinName = Fragment.GetEndPinName();

			if (Pins.ContainsByPredicate([PinName] (UEdGraphPin* PinEntry) { return !PinEntry->HasAnyConnections() && PinEntry->PinName == PinName; } ))
			{
				Fragment.ResetEndPin();
				bDirty = true;
			}
		}
	}

	if (bDirty)
	{
		ReconstructNode();
	}
}

#undef LOCTEXT_NAMESPACE
