// Copyright Ghost Pepper Games, Inc. All Rights Reserved.

#include "FlowYapEditorCommands.h"

#include "FlowYapTransactions.h"
#include "FlowYap/Nodes/FlowNode_YapDialogue.h"
#include "Graph/FlowGraphEditor.h"
#include "GraphNodes/FlowGraphNode_YapDialogue.h"

#define LOCTEXT_NAMESPACE "FlowYap"

FFlowYapGraphCommands::FFlowYapGraphCommands()
	: TCommands<FFlowYapGraphCommands>("FlowYap", NSLOCTEXT("Nodes", "FlowYap", "FlowYap"), NAME_None, FAppStyle::GetAppStyleSetName()) { }

// ================================================================================================
void FFlowYapGraphCommands::RegisterCommands()
{
	UI_COMMAND(ToggleMultipleInputs, "Enable Multiple Inputs", "TODO.", EUserInterfaceActionType::ToggleButton, FInputChord());
	UI_COMMAND(ToggleMultipleOutputs, "Enable Multiple Outputs", "TODO.", EUserInterfaceActionType::ToggleButton, FInputChord());
}

// ================================================================================================
void UFlowYapGraphCommandInjector::RegisterCommands(SFlowGraphEditor* FlowGraphEditor, TSharedPtr<FUICommandList> CommandList)
{
	FFlowYapGraphCommands::Register();
	
	const FFlowYapGraphCommands& GraphCommands = FFlowYapGraphCommands::Get();

	/*
	FUIAction Action( FExecuteAction::CreateSP(this, &ThisClass::, EVisualizerViewMode::Coalesced ),
				FCanExecuteAction(),
				FIsActionChecked::CreateSP( this, &SEventsTree::CheckViewMode, EVisualizerViewMode::Coalesced ) );
	*/
	
	CommandList->MapAction(
		GraphCommands.ToggleMultipleInputs,
		FExecuteAction::CreateUObject(this, &UFlowYapGraphCommandInjector::ToggleMultipleInputs, FlowGraphEditor),
		FCanExecuteAction::CreateUObject(this, &UFlowYapGraphCommandInjector::CanToggleMultipleInputs, FlowGraphEditor),
		FIsActionChecked::CreateUObject(this, &UFlowYapGraphCommandInjector::MultipleInputsChecked, FlowGraphEditor));
	
	CommandList->MapAction(
		GraphCommands.ToggleMultipleOutputs,
		FExecuteAction::CreateUObject(this, &UFlowYapGraphCommandInjector::ToggleMultipleOutputs, FlowGraphEditor),
		FCanExecuteAction::CreateUObject(this, &UFlowYapGraphCommandInjector::CanToggleMultipleOutputs, FlowGraphEditor),
		FIsActionChecked::CreateUObject(this, &UFlowYapGraphCommandInjector::MultipleOutputsChecked, FlowGraphEditor));
}

// ================================================================================================
void UFlowYapGraphCommandInjector::ToggleMultipleInputs(SFlowGraphEditor* FlowGraphEditor)
{
	FFlowYapTransactions::BeginModify(LOCTEXT("DialogueNode", "Toggle Multiple Inputs"), this);
	
	UEdGraphNode* Node = FlowGraphEditor->GetSingleSelectedNode();

	UFlowGraphNode_YapDialogue* YapNode = Cast<UFlowGraphNode_YapDialogue>(Node);

	YapNode->GetFlowYapNode()->ToggleMultipleInputs();

	FFlowYapTransactions::EndModify();
}

bool UFlowYapGraphCommandInjector::CanToggleMultipleInputs(SFlowGraphEditor* FlowGraphEditor) const
{
	return true;
}

bool UFlowYapGraphCommandInjector::MultipleInputsChecked(SFlowGraphEditor* FlowGraphEditor) const
{
	UEdGraphNode* Node = FlowGraphEditor->GetSingleSelectedNode();

	UFlowGraphNode_YapDialogue* YapNode = Cast<UFlowGraphNode_YapDialogue>(Node);

	return YapNode->GetFlowYapNode()->UsesMultipleInputs();
}


// ------------------------------------------------------------------------------------------------
void UFlowYapGraphCommandInjector::ToggleMultipleOutputs(SFlowGraphEditor* FlowGraphEditor)
{
	FFlowYapTransactions::BeginModify(LOCTEXT("DialogueNode", "Toggle Multiple Outputs"), this);
	
	UEdGraphNode* Node = FlowGraphEditor->GetSingleSelectedNode();

	UFlowGraphNode_YapDialogue* YapNode = Cast<UFlowGraphNode_YapDialogue>(Node);

	YapNode->GetFlowYapNode()->ToggleMultipleOutputs();

	FFlowYapTransactions::EndModify();
}

bool UFlowYapGraphCommandInjector::CanToggleMultipleOutputs(SFlowGraphEditor* FlowGraphEditor) const
{
	return true;
}

bool UFlowYapGraphCommandInjector::MultipleOutputsChecked(SFlowGraphEditor* FlowGraphEditor) const
{
	UEdGraphNode* Node = FlowGraphEditor->GetSingleSelectedNode();

	UFlowGraphNode_YapDialogue* YapNode = Cast<UFlowGraphNode_YapDialogue>(Node);

	return YapNode->GetFlowYapNode()->UsesMultipleOutputs();
}
// ------------------------------------------------------------------------------------------------
#undef LOCTEXT_NAMESPACE
