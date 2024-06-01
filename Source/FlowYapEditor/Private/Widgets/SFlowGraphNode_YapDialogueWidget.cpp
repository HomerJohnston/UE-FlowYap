#include "Widgets/SFlowGraphNode_YapDialogueWidget.h"

#include "EdGraphUtilities.h"
#include "FlowYapTransactions.h"
#include "GraphEditorSettings.h"
#include "FlowYap/Nodes/FlowNode_YapDialogue.h"
#include "GraphNodes/FlowGraphNode_YapDialogue.h"
#include "Widgets/SFlowGraphNode_YapFragmentWidget.h"
#include "Widgets/Layout/SSeparator.h"

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

const UFlowNode_YapDialogue* SFlowGraphNode_YapDialogueWidget::GetFlowYapDialogueNode() const
{
	return Cast<UFlowNode_YapDialogue>(FlowGraphNode->GetFlowNode());
}

EVisibility SFlowGraphNode_YapDialogueWidget::GetFragmentMovementVisibility() const
{
	if (GetFlowYapDialogueNode()->GetNumFragments() > 1)
	{
		return EVisibility::Visible;
	}

	return EVisibility::Hidden;
}

FReply SFlowGraphNode_YapDialogueWidget::MoveFragment(bool bUp, int64 EditorID)
{
	// TODO remove all INVTEXT
	FFlowYapTransactions::BeginModify(INVTEXT("Move Fragment"), GetFlowYapDialogueNode());
	
	TArray<FFlowYapFragment>& Fragments = GetFlowYapDialogueNode()->GetFragments();

	int16 NodeIndex = Fragments.IndexOfByPredicate( [&] (const FFlowYapFragment& Fragment)
		{
			return Fragment.GetEditorID() == EditorID;
		});

	check(NodeIndex != INDEX_NONE);

	if ((bUp && NodeIndex <= 0) || (!bUp && NodeIndex >= Fragments.Num() - 1))
	{
		return FReply::Handled();
	}

	if (bUp)
	{
		Fragments.Swap(NodeIndex, NodeIndex - 1);
	}
	else
	{
		Fragments.Swap(NodeIndex, NodeIndex + 1);
	}

	GetFlowYapDialogueNode()->OnReconstructionRequested.ExecuteIfBound();

	FFlowYapTransactions::EndModify();

	return FReply::Handled();
}

TSharedRef<SWidget> SFlowGraphNode_YapDialogueWidget::CreateNodeContentArea()
{
	SAssignNew(FragmentBox, SVerticalBox);

	bool bFirstFragment = true;
	
	for (FFlowYapFragment& Fragment : GetFlowYapDialogueNode()->GetFragments())
	{
		TSharedPtr<SFlowGraphNode_YapFragmentWidget> NewFragmentWidget = MakeShared<SFlowGraphNode_YapFragmentWidget>();

		FragmentWidgets.Add(NewFragmentWidget);

		TSharedPtr<SVerticalBox> LeftSide;
		TSharedPtr<SVerticalBox> RightSide;

		if (!bFirstFragment)
		{
			FragmentBox->AddSlot()
			.AutoHeight()
			[
				SNew(SSeparator)
				.Orientation(Orient_Horizontal)
				.Thickness(10.0f)
				.ColorAndOpacity(FLinearColor::White)
			];
		}

		bFirstFragment = false;
		
		FragmentBox->AddSlot()
		.AutoHeight()
		[
			SNew(SHorizontalBox)
			// LEFT PANE
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SBox)
				.MinDesiredWidth(40)
				[
					SNew(SOverlay)
					+ SOverlay::Slot()
					.VAlign(VAlign_Top)
					[
						SAssignNew(LeftSide, SVerticalBox)
					]
					+ SOverlay::Slot()
					.VAlign(VAlign_Center)
					[
						// CONTROLS FOR UP/DELETE/DOWN
						SNew(SVerticalBox)
						// UP
						+ SVerticalBox::Slot()
						.AutoHeight()
						.VAlign(VAlign_Center)
						.HAlign(HAlign_Center)
						[
							SNew(SButton)
							.ButtonStyle(FAppStyle::Get(), "SimpleButton")
							.ContentPadding(FMargin(8, 8))
							.Visibility(this, &SFlowGraphNode_YapDialogueWidget::GetFragmentMovementVisibility)
							.OnClicked(this, &SFlowGraphNode_YapDialogueWidget::MoveFragment, true, Fragment.GetEditorID())
							[
								SNew(SImage)
								.Image(FAppStyle::Get().GetBrush("Symbols.UpArrow"))
								.ColorAndOpacity(FLinearColor(1,1,1,0.2))
							]
						]
						// DELETE
						+ SVerticalBox::Slot()
						.AutoHeight()
						.VAlign(VAlign_Center)
						.HAlign(HAlign_Center)
						[
							SNew(SButton)
							.ButtonStyle(FAppStyle::Get(), "SimpleButton")
							.Visibility(this, &SFlowGraphNode_YapDialogueWidget::GetFragmentMovementVisibility)
							.OnClicked(this, &SFlowGraphNode_YapDialogueWidget::DeleteFragment, Fragment.GetEditorID())
							[
								SNew(SImage)
								.Image(FAppStyle::GetBrush("Icons.Delete"))
								.ColorAndOpacity(FLinearColor(1,1,1,0.2))
							]
						]
						// DOWN
						+ SVerticalBox::Slot()
						.AutoHeight()
						.VAlign(VAlign_Center)
						.HAlign(HAlign_Center)
						[
							SNew(SButton)
							.ButtonStyle(FAppStyle::Get(), "SimpleButton")
							.ContentPadding(FMargin(8, 8))
							.Visibility(this, &SFlowGraphNode_YapDialogueWidget::GetFragmentMovementVisibility)
							.OnClicked(this, &SFlowGraphNode_YapDialogueWidget::MoveFragment, false, Fragment.GetEditorID())
							[
								SNew(SImage)
								.Image(FAppStyle::Get().GetBrush("Symbols.DownArrow"))
								.ColorAndOpacity(FLinearColor(1,1,1,0.2))
							]
						]
					]
				]
			]
			// MIDDLE PANE
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Top)
			[
				SAssignNew(FragmentWidgets[FragmentWidgets.Num() -1], SFlowGraphNode_YapFragmentWidget, this, &Fragment)
			]
			// RIGHT PANE
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Top)
			[
				SAssignNew(RightSide, SVerticalBox)
			]
		];

		FragmentInputBoxes.Add(LeftSide);
		FragmentOutputBoxes.Add(RightSide);
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
			.HAlign(HAlign_Center)
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
	PinToAdd->SetShowLabel(false);
	
	const UEdGraphPin* PinObj = PinToAdd->GetPinObj();
	const bool bAdvancedParameter = (PinObj != nullptr) && PinObj->bAdvancedView;
	if (bAdvancedParameter)
	{
		PinToAdd->SetVisibility( TAttribute<EVisibility>(PinToAdd, &SGraphPin::IsPinVisibleAsAdvanced) );
	}

	FMargin LeftMargins = Settings->GetInputPinPadding();
	FMargin RightMargins = Settings->GetInputPinPadding();

	LeftMargins.Right = 0;
	RightMargins.Left = 0;
	
	if (PinToAdd->GetDirection() == EEdGraphPinDirection::EGPD_Input)
	{
		FragmentInputBoxes[InputPins.Num()]->InsertSlot(0)
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Top)
		.AutoHeight()
		.Padding(LeftMargins)
		[
			PinToAdd
		];
		InputPins.Add(PinToAdd);
	}
	else
	{
		FragmentOutputBoxes[OutputPins.Num()]->InsertSlot(0)
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Top)
		.AutoHeight()
		.Padding(RightMargins)
		[
			PinToAdd
		];
		OutputPins.Add(PinToAdd);
	}
}

FReply SFlowGraphNode_YapDialogueWidget::AddFragment()
{
	FFlowYapTransactions::BeginModify(INVTEXT("Add Fragment"), GetFlowYapDialogueNode());
	
	GetFlowYapDialogueNode()->AddFragment();

	UpdateGraphNode();

	FFlowYapTransactions::EndModify();
	
	return FReply::Handled();
}

FReply SFlowGraphNode_YapDialogueWidget::DeleteFragment(int64 FragmentID)
{
	FFlowYapTransactions::BeginModify(INVTEXT("Delete Fragment"), GetFlowYapDialogueNode());

	GetFlowYapDialogueNode()->RemoveFragment(FragmentID);

	UpdateGraphNode();

	FFlowYapTransactions::EndModify();

	return FReply::Handled();
}

/*
TSharedPtr<SGraphPin> SFlowGraphNode_YapDialogueWidget::CreatePinWidget(UEdGraphPin* Pin) const
{
	check(InPin != NULL);

	// First give a shot to the registered pin factories
	for (auto FactoryIt = FEdGraphUtilities::VisualPinFactories.CreateIterator(); FactoryIt; ++FactoryIt)
	{
		TSharedPtr<FGraphPanelPinFactory> FactoryPtr = *FactoryIt;
		if (FactoryPtr.IsValid())
		{
			TSharedPtr<SGraphPin> ResultVisualPin = FactoryPtr->CreatePin(InPin);
			if (ResultVisualPin.IsValid())
			{
				return ResultVisualPin;
			}
		}
	}

	if (const UEdGraphSchema_K2* K2Schema = Cast<const UEdGraphSchema_K2>(InPin->GetSchema()))
	{
		TSharedPtr<SGraphPin> K2PinWidget = CreateK2PinWidget(InPin);
		if(K2PinWidget.IsValid())
		{
			return K2PinWidget;
		}
	}
	
	// If we didn't pick a custom pin widget, use an uncustomized basic pin
	return SNew(SGraphPin, InPin);
}
*/