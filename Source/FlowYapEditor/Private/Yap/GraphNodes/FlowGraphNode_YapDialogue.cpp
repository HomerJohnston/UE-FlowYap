// Copyright Ghost Pepper Games, Inc. All Rights Reserved.

#include "Yap/GraphNodes/FlowGraphNode_YapDialogue.h"
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
	return YapColor::LightGray;
}

FSlateIcon UFlowGraphNode_YapDialogue::GetIconAndTint(FLinearColor& OutColor) const
{
	//return FSlateIcon(FAppStyle::GetAppStyleSetName(), "ShowFlagsMenu.SubMenu.Developer");
	return FSlateIcon();
}

void UFlowGraphNode_YapDialogue::UpdatePinsAfterFragmentInsertion(uint8 InsertionIndex)
{
	/*
	uint8 LastFragmentIndex = GetFlowYapNode()->GetNumFragments() - 1;

	// For each fragment from the very bottom up to the inserted index, copy pins downward from above
	for (int i = LastFragmentIndex; i > InsertionIndex; --i)
	{
		for (uint8 j = 0; j < GetNumPinsPerFragment(); ++j)
		{
			uint8 CurrentPinIndex = GetBasePinsNum() + i * GetNumPinsPerFragment() + j;
			uint8 SourcePinIndex = CurrentPinIndex - GetNumPinsPerFragment();
			
			UEdGraphPin* CurrentPin = OutputPins[CurrentPinIndex];
			UEdGraphPin* SourcePin = OutputPins[SourcePinIndex];

			TArray<UEdGraphPin*> LinksToMoveDown = SourcePin->LinkedTo;
			CurrentPin->BreakAllPinLinks(true);
			SourcePin->BreakAllPinLinks(true);
			
			for (UEdGraphPin* TargetPin : LinksToMoveDown)
			{
				CurrentPin->MakeLinkTo(TargetPin);
			}
		}
	}
	*/
}

void UFlowGraphNode_YapDialogue::UpdatePinsForFragmentDeletion(uint8 FragmentIndex)
{
	/*
	uint8 LastFragmentIndex = GetFlowYapNode()->GetNumFragments() - 1;
	
	// For each fragment from the deleted index downwards, copy pins upward from below
	for (int i = FragmentIndex; i <= LastFragmentIndex; ++i)
	{
		for (int j = 0; j < GetNumPinsPerFragment(); ++j)
		{
			uint8 PinIndex = GetBasePinsNum() + i * GetNumPinsPerFragment() + j;

			UEdGraphPin* PinToFixup = OutputPins[PinIndex];

			PinToFixup->BreakAllPinLinks(true);

			if (i < LastFragmentIndex)
			{
				TArray<UEdGraphPin*>& NextFragmentConnections = OutputPins[PinIndex + GetNumPinsPerFragment()]->LinkedTo;

				for (UEdGraphPin* Pin : NextFragmentConnections)
				{
					PinToFixup->MakeLinkTo(Pin);
				}
			}
		}
	}
	*/
}

void UFlowGraphNode_YapDialogue::SwapFragmentPinConnections(uint8 FragmentIndexA, uint8 FragmentIndexB)
{
	uint8 BasePinsNum = GetBasePinsNum();

	int32 AStartPinIndex = -1;
	int32 BStartPinIndex = -1;
	int32 AEndPinIndex = -1;
	int32 BEndPinIndex = -1;
	
	for (uint8 i = 0; i < OutputPins.Num(); ++i)
	{
		UEdGraphPin* Pin = OutputPins[i];

		FName AA = FName("FragmentStart", FragmentIndexA);
		FName AB = FName("FragmentStart", FragmentIndexA + 1);
		
		if (Pin->PinName == FName("FragmentStart", FragmentIndexA + 1))
		{
			AStartPinIndex = i;
			continue;
		}
		else if (Pin->PinName == FName("FragmentEnd", FragmentIndexA + 1))
		{
			AEndPinIndex = i;
			continue;
		}
		else if (Pin->PinName == FName("FragmentStart", FragmentIndexB + 1))
		{
			BStartPinIndex = i;
			continue;
		}
		else if (Pin->PinName == FName("FragmentEnd", FragmentIndexB + 1))
		{
			BEndPinIndex = i;
			continue;
		}
	}

	if (AStartPinIndex >= 0 && BStartPinIndex >= 0)
	{
		SwapPinConnections(AStartPinIndex, BStartPinIndex);
	}
	
	if (AEndPinIndex >= 0 && BEndPinIndex >= 0)
	{
		SwapPinConnections(AEndPinIndex, BEndPinIndex);
	}
}

void UFlowGraphNode_YapDialogue::SwapPinConnections(uint8 OutputIndexA, uint8 OutputIndexB)
{
	UEdGraphPin* PinA = OutputPins[OutputIndexA];
	UEdGraphPin* PinB = OutputPins[OutputIndexB];
	
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

uint8 UFlowGraphNode_YapDialogue::GetBasePinsNum() const
{
	return (GetFlowYapNode()->GetIsPlayerPrompt() ? 1 : 2);
}

