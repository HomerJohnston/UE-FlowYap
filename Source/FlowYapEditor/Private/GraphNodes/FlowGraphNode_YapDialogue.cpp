// Copyright Ghost Pepper Games, Inc. All Rights Reserved.

#include "GraphNodes/FlowGraphNode_YapDialogue.h"

#include "FlowYapEditorCommands.h"
#include "FlowYap/Nodes/FlowNode_YapDialogue.h"
#include "Widgets/SFlowGraphNode_YapDialogueWidget.h"

#define LOCTEXT_NAMESPACE "FlowYap"

UFlowGraphNode_YapDialogue::UFlowGraphNode_YapDialogue()
{
	AssignedNodeClasses = {UFlowNode_YapDialogue::StaticClass()};
}

TSharedPtr<SGraphNode> UFlowGraphNode_YapDialogue::CreateVisualWidget()
{
	GetFlowNode()->OnReconstructionRequested.ExecuteIfBound();

	return SNew(SFlowGraphNode_YapDialogueWidget, this);
}

bool UFlowGraphNode_YapDialogue::ShowPaletteIconOnNode() const
{
	return true;
}

void UFlowGraphNode_YapDialogue::GetNodeContextMenuActions(UToolMenu* Menu, UGraphNodeContextMenuContext* Context) const
{
	Super::GetNodeContextMenuActions(Menu, Context);

	const FFlowYapGraphCommands& Commands = FFlowYapGraphCommands::Get();
	
	FToolMenuSection& Test = Menu->AddSection("FlowYapActions", LOCTEXT("PinsSettings", "Pins Settings"));

	//Test.AddMenuEntry(Commands.ToggleMultipleInputs);
	//Test.AddMenuEntry(Commands.ToggleMultipleOutputs);
}

UFlowNode_YapDialogue* UFlowGraphNode_YapDialogue::GetFlowYapNode() const
{
	return Cast<UFlowNode_YapDialogue>(GetFlowNode());
}

FLinearColor UFlowGraphNode_YapDialogue::GetNodeBodyTintColor() const
{
	return UFlowGraphNode::GetNodeBodyTintColor();
	/*
	if (GetFlowYapNode()->GetIsPlayerPrompt())
	{
		return FLinearColor::White;
	}

	return FlowYapColors::Gray;
	*/
}

FSlateIcon UFlowGraphNode_YapDialogue::GetIconAndTint(FLinearColor& OutColor) const
{
	return FSlateIcon(FAppStyle::GetAppStyleSetName(), "ShowFlagsMenu.SubMenu.Developer");
}

