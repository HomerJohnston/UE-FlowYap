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

	for (FFlowYapFragment& Fragment : GetFlowYapDialogueNode()->GetFragments())
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
				+ SVerticalBox::Slot()
				.AutoHeight()
				.FillHeight(1.0)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.FillWidth(0.5)
					[
						SNew(SBox)
						.WidthOverride(6)
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.HAlign(HAlign_Fill)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot()
							.FillWidth(1.0)
							[
								SNew(SBox)
							]
							+ SHorizontalBox::Slot()
							.AutoWidth()
							[
								SNew(SButton)
								.ButtonStyle(FAppStyle::Get(), "SimpleButton")
								.ContentPadding(FMargin(8, 8))
								//.OnClicked(this, &SFlowGraphNode_YapDialogueWidget::DeleteFragment, Fragment.GetEditorID())
								[
									SNew(SImage)
									.Image(FAppStyle::Get().GetBrush("Symbols.UpArrow"))
									.ColorAndOpacity(FLinearColor(1,1,1,0.25))
								]
							]
							+ SHorizontalBox::Slot()
							.FillWidth(1.0)
							[
								SNew(SBox)
							]
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(SButton)
							.ButtonStyle(FAppStyle::Get(), "SimpleButton")
							.OnClicked(this, &SFlowGraphNode_YapDialogueWidget::DeleteFragment, Fragment.GetEditorID())
							[
								SNew(SImage)
								.Image(FAppStyle::GetBrush("Icons.Delete"))
								.ColorAndOpacity(FLinearColor(1,1,1,0.25))
							]
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot()
							.FillWidth(1.0)
							[
								SNew(SBox)
							]
							+ SHorizontalBox::Slot()
							.AutoWidth()
							[
								SNew(SButton)
								.ButtonStyle(FAppStyle::Get(), "SimpleButton")
								.ContentPadding(FMargin(8, 8))
								//.OnClicked(this, &SFlowGraphNode_YapDialogueWidget::DeleteFragment, Fragment.GetEditorID())
								[
									SNew(SImage)
									.Image(FAppStyle::Get().GetBrush("Symbols.DownArrow"))
									.ColorAndOpacity(FLinearColor(1,1,1,0.25))
								]
							]
							+ SHorizontalBox::Slot()
							.FillWidth(1.0)
							[
								SNew(SBox)
							]
						]
					]
					+ SHorizontalBox::Slot()
					.FillWidth(0.5)
					[
						SNew(SBox)
						.WidthOverride(4)
					]
				]
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SAssignNew(FragmentWidgets[FragmentWidgets.Num() -1], SFlowGraphNode_YapFragmentWidget, this, &Fragment)
			]
		];

		FragmentInputBoxes.Add(FragmentInputBox);
	};

	/*
	+ SHorizontalBox::Slot()
	.AutoWidth()
	[
		SNew(SButton)
		.Text(INVTEXT("TEST"))
		.OnClicked(this, &SFlowGraphNode_YapFragmentWidget::Delete)
	]
	*/
	
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
			/*
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SAssignNew(LeftNodeBox, SVerticalBox)
				+ SVerticalBox::Slot()
				[
					SNew(SBox)
					.Padding(0, 0)
				]
			]
			*/
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
		FragmentInputBoxes[InputPins.Num()]->InsertSlot(0)
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

FReply SFlowGraphNode_YapDialogueWidget::DeleteFragment(int64 FragmentID)
{
	GetFlowYapDialogueNode()->RemoveFragment(FragmentID);

	UpdateGraphNode();

	return FReply::Handled();
}
