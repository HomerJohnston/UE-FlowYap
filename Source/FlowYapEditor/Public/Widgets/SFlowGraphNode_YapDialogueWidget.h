#pragma once
#include "Graph/Widgets/SFlowGraphNode.h"
#include "Widgets/SUserWidget.h"

class SFlowGraphNode_YapFragmentWidget;
class UFlowNode_YapDialogue;
class UFlowGraphNode_YapDialogue;

class SFlowGraphNode_YapDialogueWidget : public SFlowGraphNode
{
protected:
	// TODO: Is this safe?
	UFlowGraphNode_YapDialogue* FlowGraphNode_YapDialogue = nullptr;

	TSharedPtr<SVerticalBox> FragmentBox;
	
	TArray<TSharedPtr<SFlowGraphNode_YapFragmentWidget>> FragmentWidgets;

	TArray<TSharedPtr<SVerticalBox>> FragmentInputBoxes;
	
public:
	void Construct(const FArguments& InArgs, UFlowGraphNode* InNode);

	UFlowNode_YapDialogue* GetFlowYapDialogueNode();
	

protected:
	virtual TSharedRef<SWidget> CreateNodeContentArea() override;

	virtual void AddPin(const TSharedRef<SGraphPin>& PinToAdd);

	void AddInputPin(const TSharedRef<SGraphPin>& PinToAdd);

	void AddOutputPin(const TSharedRef<SGraphPin>& PinToAdd);

	/*
	virtual FReply OnAddFlowPin(const EEdGraphPinDirection Direction);
	*/
	
	FReply AddFragment();

public:
	FReply DeleteFragment(int64 FragmentID);
};
