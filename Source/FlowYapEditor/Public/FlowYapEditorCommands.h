#pragma once

#include "Framework/Commands/Commands.h"
#include "Graph/FlowGraphCommandInjector.h"
#include "FlowYapEditorCommands.generated.h"

class FFlowYapGraphCommands : public TCommands<FFlowYapGraphCommands>
{
public:
	/** Default constructor. */
	FFlowYapGraphCommands();

	TSharedPtr<FUICommandInfo> ToggleMultipleInputs;
	TSharedPtr<FUICommandInfo> ToggleMultipleOutputs;

	void RegisterCommands() override;
};

UCLASS()
class UFlowYapGraphCommandInjector : public UFlowGraphCommandInjector
{
	GENERATED_BODY()

public:
	void RegisterCommands(SFlowGraphEditor* FlowGraphEditor, TSharedPtr<FUICommandList> CommandList) override;

	// ----------
	void ToggleMultipleInputs(SFlowGraphEditor* FlowGraphEditor);
	
	bool CanToggleMultipleInputs(SFlowGraphEditor* FlowGraphEditor) const;
	
	bool MultipleInputsChecked(SFlowGraphEditor* FlowGraphEditor) const;
	
	// ----------
	void ToggleMultipleOutputs(SFlowGraphEditor* FlowGraphEditor);
	
	bool CanToggleMultipleOutputs(SFlowGraphEditor* FlowGraphEditor) const;
	
	bool MultipleOutputsChecked(SFlowGraphEditor* FlowGraphEditor) const;
};