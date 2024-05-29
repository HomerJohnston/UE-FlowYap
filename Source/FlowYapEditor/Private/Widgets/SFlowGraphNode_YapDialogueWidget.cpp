#include "Widgets/SFlowGraphNode_YapDialogueWidget.h"

#include "GraphEditorSettings.h"
#include "FlowYap/Nodes/FlowNode_YapDialogue.h"
#include "GraphNodes/FlowGraphNode_YapDialogue.h"
#include "Widgets/SFlowGraphNode_YapFragmentWidget.h"

void SFlowGraphNode_YapDialogueWidget::Construct(const FArguments& InArgs, UFlowGraphNode* InNode)
{
	GraphNode = InNode;
	FlowGraphNode = InNode;
	FlowGraphNode_YapDialogue = Cast<UFlowGraphNode_YapDialogue>(InNode);

	UpdateGraphNode();
}

UFlowNode_YapDialogue* SFlowGraphNode_YapDialogueWidget::GetFlowYapDialogueNode()
{
	return Cast<UFlowNode_YapDialogue>(FlowGraphNode->GetFlowNode());
}

TSharedRef<SWidget> SFlowGraphNode_YapDialogueWidget::CreateNodeContentArea()
{
	UE_LOG(LogTemp, Warning, TEXT("AHAHAHA"));
	SAssignNew(FragmentBox, SVerticalBox);
	
	for (FFlowYapFragment& Fragment :  GetFlowYapDialogueNode()->GetFragments())
	{
		TSharedPtr<SFlowGraphNode_YapFragmentWidget> NewFragmentWidget = MakeShared<SFlowGraphNode_YapFragmentWidget>();

		FragmentWidgets.Add(NewFragmentWidget);

		TSharedPtr<SVerticalBox> FragmentInputBox;
		
		FragmentBox->AddSlot()
		.AutoHeight()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SAssignNew(FragmentInputBox, SVerticalBox)
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SAssignNew(FragmentWidgets[FragmentWidgets.Num() -1], SFlowGraphNode_YapFragmentWidget, this, &Fragment)
			]
		];

		FragmentInputBoxes.Add(FragmentInputBox);
	};

	return SNew(SBorder)
	.BorderImage(FAppStyle::GetBrush("NoBorder"))
	.HAlign(HAlign_Fill)
	.VAlign(VAlign_Fill)
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SAssignNew(LeftNodeBox, SVerticalBox)
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				FragmentBox.ToSharedRef()
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SAssignNew(RightNodeBox, SVerticalBox)
			]
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SButton)
			.Text(INVTEXT("Add"))
			.OnClicked(this, &SFlowGraphNode_YapDialogueWidget::AddFragment)
		]
	];
}

void SFlowGraphNode_YapDialogueWidget::AddPin(const TSharedRef<SGraphPin>& PinToAdd)
{
	if (GetFlowYapDialogueNode()->GetFragments().Num() == 0)
	{
		return;
	}

	PinToAdd->SetOwner(SharedThis(this));
	
	const UEdGraphPin* PinObj = PinToAdd->GetPinObj();
	const bool bAdvancedParameter = (PinObj != nullptr) && PinObj->bAdvancedView;
	if (bAdvancedParameter)
	{
		PinToAdd->SetVisibility( TAttribute<EVisibility>(PinToAdd, &SGraphPin::IsPinVisibleAsAdvanced) );
	}

	if (PinToAdd->GetDirection() == EEdGraphPinDirection::EGPD_Input)
	{
		FragmentInputBoxes[InputPins.Num()]->AddSlot()
		
		//LeftNodeBox->AddSlot()
		//.AutoHeight()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Center)
		.AutoHeight()
		.Padding(Settings->GetInputPinPadding())
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				PinToAdd
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SSpacer)
				.Size(20)
			]
		];
		InputPins.Add(PinToAdd);
	}
	else // Direction == EEdGraphPinDirection::EGPD_Output
	{
		RightNodeBox->AddSlot()
		.AutoHeight()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Center)
		.Padding(Settings->GetInputPinPadding())
		[
			PinToAdd
		];
		OutputPins.Add(PinToAdd);
	}
}

FReply SFlowGraphNode_YapDialogueWidget::AddFragment()
{
	GetFlowYapDialogueNode()->AddFragment();

	UpdateGraphNode();

	return FReply::Handled();
}

void SFlowGraphNode_YapDialogueWidget::DeleteFragment(int64 FragmentID)
{
	GetFlowYapDialogueNode()->RemoveFragment(FragmentID);

	UpdateGraphNode();
}
