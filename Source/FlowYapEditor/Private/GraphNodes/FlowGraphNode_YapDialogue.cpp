// Copyright Ghost Pepper Games, Inc. All Rights Reserved.

#include "GraphNodes/FlowGraphNode_YapDialogue.h"

#include "FlowYapColors.h"
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

void UFlowGraphNode_YapDialogue::UpdatePinsForFragmentDeletion(uint8 FragmentIndex)
{
	uint8 LastFragmentIndex = GetFlowYapNode()->GetNumFragments() - 1;

	uint8 NumPins = 3;

	// For each fragment
	for (int i = FragmentIndex; i <= LastFragmentIndex; ++i)
	{
		for (int j = 0; j < NumPins; ++j)
		{
			uint8 PinIndex = i * NumPins + j;

			UEdGraphPin* PinToFixup = OutputPins[PinIndex];

			PinToFixup->BreakAllPinLinks(true);

			if (i < LastFragmentIndex)
			{
				TArray<UEdGraphPin*>& NextFragmentConnections = OutputPins[PinIndex + NumPins]->LinkedTo;

				for (UEdGraphPin* Pin : NextFragmentConnections)
				{
					PinToFixup->MakeLinkTo(Pin);
				}
			}
		}
	}
}

void UFlowGraphNode_YapDialogue::SwapPinConnections(uint8 FragmentIndexA, uint8 FragmentIndexB)
{
	uint8 PinCount = 3;

	for (int i = 0; i < PinCount; ++i)
	{
		DoTrick(FragmentIndexA * PinCount + i, FragmentIndexB * PinCount + i);
	}
}

void UFlowGraphNode_YapDialogue::DoTrick(uint8 FragmentIndexA, uint8 FragmentIndexB)
{
	UEdGraphPin* PinA = OutputPins[FragmentIndexA];
	UEdGraphPin* PinB = OutputPins[FragmentIndexB];
	
	TArray<UEdGraphPin*> PinALinks = PinA->LinkedTo;
	PinA->BreakAllPinLinks(true);

	for (UEdGraphPin* PinBConnection : PinB->LinkedTo)
	{
		PinA->MakeLinkTo(PinBConnection);
	}

	PinB->BreakAllPinLinks(true);
	
	for (UEdGraphPin* PinAConnection : PinALinks)
	{
		PinB->MakeLinkTo(PinAConnection);
	}
}

