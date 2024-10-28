// Copyright Ghost Pepper Games, Inc. All Rights Reserved.

#include "Yap/GraphNodes/FlowGraphNode_YapDialogue.h"

#include "Graph/FlowGraph.h"
#include "Yap/Nodes/FlowNode_YapDialogue.h"
#include "Yap/FlowYapColors.h"
#include "Yap/NodeWidgets/SFlowGraphNode_YapDialogueWidget.h"

#define LOCTEXT_NAMESPACE "FlowYap"

UFlowGraphNode_YapDialogue::UFlowGraphNode_YapDialogue()
{
	AssignedNodeClasses = {UFlowNode_YapDialogue::StaticClass()};
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

void UFlowGraphNode_YapDialogue::DeleteFragment(FGuid FragmentGuid)
{
	uint32 FragmentIndex = GetFlowYapNode()->FindFragmentIndex(FragmentGuid);

	if (FragmentIndex == INDEX_NONE)
	{
		UE_LOG(FlowYap, Warning, TEXT("Fragment with Guid not found: %s"), *FragmentGuid.ToString());
		return;
	}

	for (UEdGraphPin* Pin : GetAllPins())
	{
		
	}
	
	GetFlowYapNode()->RemoveFragment(FragmentIndex);
}
