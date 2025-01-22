// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "YapEditor/GraphNodes/FlowGraphNode_YapDialogue.h"

#include "Graph/FlowGraph.h"
#include "Graph/FlowGraphEditor.h"
#include "Graph/FlowGraphUtils.h"
#include "UObject/ObjectSaveContext.h"
#include "Yap/Nodes/FlowNode_YapDialogue.h"
#include "YapEditor/YapColors.h"
#include "YapEditor/YapDialogueNodeCommands.h"
#include "YapEditor/YapEditorEventBus.h"
#include "YapEditor/YapEditorEvents.h"
#include "YapEditor/YapLogEditor.h"
#include "YapEditor/YapEditorSubsystem.h"
#include "YapEditor/YapTransactions.h"
#include "YapEditor/NodeWidgets/SFlowGraphNode_YapDialogueWidget.h"

#define LOCTEXT_NAMESPACE "YapEditor"

UFlowGraphNode_YapDialogue::UFlowGraphNode_YapDialogue()
{
	AssignedNodeClasses = {UFlowNode_YapDialogue::StaticClass()};

	EventBus << YapEditor::Events::DialogueNode::Test;
}

TSharedPtr<SGraphNode> UFlowGraphNode_YapDialogue::CreateVisualWidget()
{
	FYapDialogueNodeCommands::Register();

	Commands = MakeShared<FUICommandList>();
	Commands->MapAction(FYapDialogueNodeCommands::Get().RecalculateText, FExecuteAction::CreateUObject(this, &UFlowGraphNode_YapDialogue::RecalculateTextOnAllFragments));
	
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

void UFlowGraphNode_YapDialogue::GetNodeContextMenuActions(class UToolMenu* Menu, class UGraphNodeContextMenuContext* Context) const
{
	Super::GetNodeContextMenuActions(Menu, Context);

	const FYapDialogueNodeCommands& DialogeNodeCommands = FYapDialogueNodeCommands::Get();

	{
		FToolMenuSection& Section = Menu->AddSection("Yap", LOCTEXT("Yap", "Yap"));
		Section.AddMenuEntryWithCommandList(DialogeNodeCommands.RecalculateText, Commands);
	}
}

void UFlowGraphNode_YapDialogue::RecalculateTextOnAllFragments()
{
	FGraphPanelSelectionSet Nodes = FFlowGraphUtils::GetFlowGraphEditor(GetGraph())->GetSelectedNodes();
	
	FYapScopedTransaction T(FName("Default"), FText::Format(LOCTEXT("RecalculateTextLength_Command","Recalculate text length on {0} {0}|plural(one=node,other=nodes)"), Nodes.Num()), nullptr);

	for (UObject* Node : Nodes)
	{
		if (UFlowGraphNode_YapDialogue* DialogeGraphNode = Cast<UFlowGraphNode_YapDialogue>(Node))
		{
			UFlowNode_YapDialogue* DialogueNode2 = DialogeGraphNode->GetFlowYapNode();
			DialogueNode2->Modify();

			for (FYapFragment& Fragment : DialogueNode2->GetFragmentsMutable())
			{
				Fragment.GetBitMutable().RecalculateText();
			}
		}
	}
}

void UFlowGraphNode_YapDialogue::InitializeInstance()
{
	Super::InitializeInstance();
}

#undef LOCTEXT_NAMESPACE
