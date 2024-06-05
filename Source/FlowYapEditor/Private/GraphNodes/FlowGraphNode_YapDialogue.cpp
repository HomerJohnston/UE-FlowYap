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

void UFlowGraphNode_YapDialogue::UpdatePinsAfterFragmentInsertion(uint8 InsertionIndex)
{
	// Start above Bypass pin PLUS above new pin that got added after fragment insertion
	uint8 LastIndex = OutputPins.Num() - 3;

	for (int i = LastIndex; i >= InsertionIndex; --i)
	{
		UEdGraphPin* CurrentPin = OutputPins[i];

		TArray<UEdGraphPin*> LinksToMoveDown = CurrentPin->LinkedTo;
		
		CurrentPin->BreakAllPinLinks(true);

		for (UEdGraphPin* Pin : LinksToMoveDown)
		{
			OutputPins[i + 1 ]->MakeLinkTo(Pin);
		}
	}
}

void UFlowGraphNode_YapDialogue::UpdatePinsForFragmentDeletion(uint8 DeleteIndex)
{
	// -2 is to account for the bypass node, the last node! don't mess with the bypass node!
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

void UFlowGraphNode_YapDialogue::SwapPinConnections(uint8 A, uint8 B)
{
	UEdGraphPin* PinA = OutputPins[A];
	UEdGraphPin* PinB = OutputPins[B];

	TArray<UEdGraphPin*> PinALinksCopy = PinA->LinkedTo;
	TArray<UEdGraphPin*> PinBLinksCopy = PinB->LinkedTo;

	PinA->BreakAllPinLinks(true);
	PinB->BreakAllPinLinks(true);

	for (UEdGraphPin* ConnectToA : PinBLinksCopy)
	{
		PinA->MakeLinkTo(ConnectToA);
	}
	
	for (UEdGraphPin* ConnectToB : PinALinksCopy)
	{
		PinB->MakeLinkTo(ConnectToB);
	}
}

