// Copyright Ghost Pepper Games, Inc. All Rights Reserved.

#include "GraphNodes/FlowGraphNode_YapDialogue.h"

#include "FlowYapColors.h"
#include "FlowYapEditorCommands.h"
#include "FlowYap/FlowYapLog.h"
#include "FlowYap/Nodes/FlowNode_YapDialogue.h"
#include "Graph/FlowGraphUtils.h"
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

	//const FFlowYapGraphCommands& Commands = FFlowYapGraphCommands::Get();
	
	//FToolMenuSection& Test = Menu->AddSection("FlowYapActions", LOCTEXT("PinsSettings", "Pins Settings"));

	//Test.AddMenuEntry(Commands.ToggleMultipleInputs);
	//Test.AddMenuEntry(Commands.ToggleMultipleOutputs);
}

UFlowNode_YapDialogue* UFlowGraphNode_YapDialogue::GetFlowYapNode() const
{
	return Cast<UFlowNode_YapDialogue>(GetFlowNode());
}

FLinearColor UFlowGraphNode_YapDialogue::GetNodeBodyTintColor() const
{
	//return UFlowGraphNode::GetNodeBodyTintColor();
	
	if (GetFlowYapNode()->GetIsPlayerPrompt())
	{
		return FlowYapColor::White;
	}

	return FlowYapColor::LightGray;
}

FSlateIcon UFlowGraphNode_YapDialogue::GetIconAndTint(FLinearColor& OutColor) const
{
	return FSlateIcon(FAppStyle::GetAppStyleSetName(), "ShowFlagsMenu.SubMenu.Developer");
}

void UFlowGraphNode_YapDialogue::PreparePinsForFragmentDeletion(int16 DeleteIndex)
{
	// -2 is to account for the bypass node! don't touch the bypass node!
	uint8 LastIndex = OutputPins.Num() - 2;

	for (int i = DeleteIndex; i <= LastIndex - 1; ++i)
	{
		UEdGraphPin* PinToFixup = OutputPins[i];

		PinToFixup->BreakAllPinLinks(true);

		if (i < LastIndex)
		{
			TArray<UEdGraphPin*>& NextFragmentConnections = OutputPins[i + 1]->LinkedTo;

			for (UEdGraphPin* Pin : NextFragmentConnections)
			{
				PinToFixup->MakeLinkTo(Pin);
			}
		}
	}
}

