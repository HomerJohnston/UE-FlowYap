// Copyright Ghost Pepper Games, Inc. All Rights Reserved.

#include "Widgets/SFlowGraphNode_YapDialogueWidget.h"

#include "FlowEditorStyle.h"
#include "FlowYapColors.h"
#include "FlowYapEditorSubsystem.h"
#include "FlowYapInputTracker.h"
#include "FlowYapTransactions.h"
#include "GraphEditorSettings.h"
#include "Blueprint/SlateBlueprintLibrary.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "FlowYap/FlowYapLog.h"
#include "FlowYap/Nodes/FlowNode_YapDialogue.h"
#include "Graph/FlowGraphEditor.h"
#include "Graph/FlowGraphUtils.h"
#include "GraphNodes/FlowGraphNode_YapDialogue.h"
#include "Widgets/SFlowGraphNode_YapFragmentWidget.h"
#include "Widgets/Layout/SSeparator.h"

#define LOCTEXT_NAMESPACE "FlowYap"

void SFlowGraphNode_YapDialogueWidget::Construct(const FArguments& InArgs, UFlowGraphNode* InNode)
{	
	GraphNode = InNode;
	FlowGraphNode = InNode;
	FlowGraphNode_YapDialogue = Cast<UFlowGraphNode_YapDialogue>(InNode);

	DialogueButtonsColor = FlowYapColor::DimGray;

	ConnectedPinColor = FlowYapColor::White;
	DisconnectedPinColor = FlowYapColor::DarkGray;
	ConnectedBypassPinColor = FlowYapColor::LightBlue;
	DisconnectedBypassPinColor = FlowYapColor::DarkBlue;
	
	// TODO move this to my editor subsystem, no need to build it over and over
	NormalText = FTextBlockStyle()
	.SetFont(FCoreStyle::GetDefaultFontStyle("Regular", 10))
	.SetFontSize(10)
	.SetColorAndOpacity(FSlateColor::UseForeground())
	.SetShadowOffset(FVector2D::ZeroVector)
	.SetShadowColorAndOpacity(FLinearColor::Black)
	.SetHighlightColor(FLinearColor(0.02f, 0.3f, 0.0f));

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
	if (IsHovered() && GetFlowYapDialogueNode()->GetNumFragments() > 1)
	{
		return EVisibility::Visible;
	}

	return EVisibility::Hidden;
}

FReply SFlowGraphNode_YapDialogueWidget::MoveFragment(bool bUp, FFlowYapFragment* Fragment)
{
	FFlowYapTransactions::BeginModify(LOCTEXT("DialogueNode", "Move Fragment"), GetFlowYapDialogueNode());
	
	TArray<FFlowYapFragment>& Fragments = GetFlowYapDialogueNode()->GetFragments();

	int16 NodeIndex = Fragments.IndexOfByPredicate( [&] (const FFlowYapFragment& NodeFragment)
		{
			return &NodeFragment == Fragment;
		});

	// TODO hide the buttons at top and bottom
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

FSlateColor SFlowGraphNode_YapDialogueWidget::GetFragmentSeparatorColor() const
{
	return FLinearColor::Black;
	//return GetFlowYapDialogueNode()->GetIsPlayerPrompt() ? FLinearColor::Black : FlowYapColors::Gray;
}

TOptional<int32> SFlowGraphNode_YapDialogueWidget::GetActivationLimit(FFlowYapFragment* Fragment) const
{
	int32 Limit = Fragment->GetActivationLimit();

	return Limit == 0 ? TOptional<int32>() : Limit;
}

void SFlowGraphNode_YapDialogueWidget::OnActivationLimitChanged(int32 NewValue, FFlowYapFragment* Fragment)
{
	FFlowYapTransactions::BeginModify(LOCTEXT("DialogueNode", "Change Activation Limit"), GetFlowYapDialogueNode());

	Fragment->SetActivationLimit(NewValue);

	FFlowYapTransactions::EndModify();
}

FSlateColor SFlowGraphNode_YapDialogueWidget::GetActivationDotColor(FFlowYapFragment* Fragment, int32 ActivationIndex) const
{
	int32 Count = Fragment->GetActivationCount();
	int32 Limit = Fragment->GetActivationLimit();
	int32 Current = ActivationIndex + 1;
	
	// Make them all red if the limit has been hit
	if (Limit > 0 && Count >= Limit && Current <= Count)
	{
		return FlowYapColor::Red;
	}

	// Draw "available" ones gray
	if (Current > Count && Current <= Limit)
	{
		return GEditor->PlayWorld ? FlowYapColor::Noir : FlowYapColor::Gray;
	}

	// Draw points that are done with a limit set white
	if (Current <= Count && Limit > 0)
	{
		return FlowYapColor::White;
	}

	// Draw points that are done without a limit set white
	if (Current <= Count)
	{
		return FlowYapColor::White;
	}

	// Draw all other pins black, or transparent if in play
	if (GEditor->PlayWorld)
	{
		return FlowYapColor::Transparent;
	}

	return IsHovered() ? FlowYapColor::Black : FlowYapColor::Transparent;
}

FReply SFlowGraphNode_YapDialogueWidget::OnClickedActivationDot(FFlowYapFragment* Fragment, int ActivationIndex)
{
	// TODO ignore input during PIE?
	if (Fragment->GetActivationLimit() == ActivationIndex + 1)
	{
		Fragment->SetActivationLimit(0);
	}
	else
	{
		Fragment->SetActivationLimit(ActivationIndex + 1);
	}

	return FReply::Handled();
}

EVisibility SFlowGraphNode_YapDialogueWidget::GetActivationIndicatorVisibility(SFlowGraphNode_YapDialogueWidget* FlowGraphNode_YapDialogueWidget, FFlowYapFragment* FlowYapFragment) const
{
	if (GEditor->PlayWorld)
	{
		return (FlowYapFragment->GetActivationLimit() > 0 || FlowYapFragment->GetActivationCount() > 0) ? EVisibility::Visible : EVisibility::Collapsed;		
	}
	else
	{
		if (IsHovered())
		{
			return EVisibility::Visible;
		}

		return (FlowYapFragment->GetActivationLimit() > 0) ? EVisibility::Visible : EVisibility::Collapsed;	
	}
}

EVisibility SFlowGraphNode_YapDialogueWidget::GetDialogueCycleFragmentSequencingVisibility() const
{
	if (GetFlowYapDialogueNode()->GetIsPlayerPrompt())
	{
		return EVisibility::Hidden; // Should be Collapsed but that destroys the parent widget layout for some reason
	}
	
	return (GetFlowYapDialogueNode()->GetNumFragments() > 1) ? EVisibility::Visible : EVisibility::Hidden;
}

FReply SFlowGraphNode_YapDialogueWidget::HandleDialogueCycleFragmentSequencingClicked()
{
	FFlowYapTransactions::BeginModify(LOCTEXT("DialogueNodeChangeSequencing", "Change dialogue node sequencing setting"), GetFlowYapDialogueNode());

	GetFlowYapDialogueNode()->CycleFragmentSequencingMode();
	
	return FReply::Handled();
}

const FSlateBrush* SFlowGraphNode_YapDialogueWidget::GetDialogueFragmentSequencingIcon() const
{
	switch (GetFlowYapDialogueNode()->GetMultipleFragmentSequencing())
	{
	case EFlowYapMultipleFragmentSequencing::Sequential:
		{
			return FAppStyle::Get().GetBrush("LevelEditor.Profile"); 
		}
	case EFlowYapMultipleFragmentSequencing::Random:
		{
			return FAppStyle::Get().GetBrush("Icons.Convert"); 
		}
	}

	return FAppStyle::Get().GetBrush("Icons.Error"); 
}

FText SFlowGraphNode_YapDialogueWidget::GetDialogueCycleFragmentSequencingTooltip() const
{
	switch (GetFlowYapDialogueNode()->GetMultipleFragmentSequencing())
	{
	case EFlowYapMultipleFragmentSequencing::Sequential:
		{
			return LOCTEXT("DialogueNodeSequence", "Run nodes from top to bottom");
		}
	case EFlowYapMultipleFragmentSequencing::Random:
		{
			// TODO should I describe if this is deterministic or not? Should I make it deterministic? How?
			return LOCTEXT("DialogueNodeSequence", "Select nodes randomly");
		}
	default:
		{
			return LOCTEXT("DialogueNodeSequence", "ERROR");
		}
	}
}

FSlateColor SFlowGraphNode_YapDialogueWidget::GetDialogueCycleFragmentSequencingColor() const
{
	switch (GetFlowYapDialogueNode()->GetMultipleFragmentSequencing())
	{
	case EFlowYapMultipleFragmentSequencing::Sequential:
		{
			return FlowYapColor::DarkGray;
		}
	case EFlowYapMultipleFragmentSequencing::Random:
		{
			return FlowYapColor::Orange;
		}
	default:
		{
			return FlowYapColor::White;
		}
	}
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
			.Padding(0,2,0,5)
			[
				SNew(SSeparator)
				.SeparatorImage(FAppStyle::Get().GetBrush("Menu.Separator"))
				.Orientation(Orient_Horizontal)
				.Thickness(3.0f)
				.ColorAndOpacity(this, &SFlowGraphNode_YapDialogueWidget::GetFragmentSeparatorColor)
			];
		}

		bFirstFragment = false;

		TSharedPtr<SVerticalBox> LeftSideActivationIndicator;
		
		FragmentBox->AddSlot()
		.AutoHeight()
		[
			SNew(SHorizontalBox)
			// LEFT PANE
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0, 0, 0, 2)
			.VAlign(VAlign_Fill)
			[
				SNew(SBox)
				.WidthOverride(40)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.VAlign(VAlign_Top)
					.AutoHeight()
					.HAlign(HAlign_Center)
					[
						SNew(SBox)
						.MinDesiredHeight(12)
						[
							SAssignNew(LeftSide, SVerticalBox)
						]
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					[
						SNew(SBox)
						.Visibility(this, &SFlowGraphNode_YapDialogueWidget::GetActivationIndicatorVisibility, this, &Fragment)
						[
							SAssignNew(LeftSideActivationIndicator, SVerticalBox)
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
			.VAlign(VAlign_Fill)
			[
				SNew(SBox)
				.MinDesiredWidth(40)
				[
					SNew(SOverlay)
					+ SOverlay::Slot()
					[
						SNew(SBox)
						.VAlign(VAlign_Bottom)
						.Visibility(this, &SFlowGraphNode_YapDialogueWidget::GetFragmentMovementVisibility)
						[
							// CONTROLS FOR UP/DELETE/DOWN
							SNew(SVerticalBox)
							// UP
							+ SVerticalBox::Slot()
							.AutoHeight()
							.VAlign(VAlign_Center)
							.HAlign(HAlign_Center)
							.Padding(0, 1)
							[
								SNew(SButton)
								.ButtonStyle(FAppStyle::Get(), "SimpleButton")
								.ContentPadding(FMargin(4, 4))
								.ToolTipText(LOCTEXT("DialogueMoveFragmentUp_Tooltip", "Move Fragment Up"))
								.OnClicked(this, &SFlowGraphNode_YapDialogueWidget::MoveFragment, true, &Fragment)
								.Visibility(this, &SFlowGraphNode_YapDialogueWidget::GetMoveFragmentUpVisibility, &Fragment)
								[
									SNew(SImage)
									.Image(FAppStyle::Get().GetBrush("Symbols.UpArrow"))
									.DesiredSizeOverride(FVector2D(8,8))
									.ColorAndOpacity(DialogueButtonsColor)
								]
							]
							// DELETE
							+ SVerticalBox::Slot()
							.AutoHeight()
							.VAlign(VAlign_Center)
							.HAlign(HAlign_Center)
							.Padding(0, 1)
							[
								SNew(SButton)
								.ButtonStyle(FAppStyle::Get(), "SimpleButton")
								.ContentPadding(FMargin(4, 4))
								.ToolTipText(LOCTEXT("DialogueDeleteFragment_Tooltip", "Delete Fragment"))
								.OnClicked(this, &SFlowGraphNode_YapDialogueWidget::DeleteFragment, &Fragment)
								[
									SNew(SImage)
									.Image(FAppStyle::GetBrush("Icons.Delete"))
									.DesiredSizeOverride(FVector2D(8,8))
									.ColorAndOpacity(DialogueButtonsColor)
								]
							]
							// DOWN
							+ SVerticalBox::Slot()
							.AutoHeight()
							.VAlign(VAlign_Center)
							.HAlign(HAlign_Center)
							.Padding(0, 1)
							[
								SNew(SButton)
								.ButtonStyle(FAppStyle::Get(), "SimpleButton")
								.ContentPadding(FMargin(4, 4))
								.ToolTipText(LOCTEXT("DialogueMoveFragmentDown_Tooltip", "Move Fragment Down"))
								.OnClicked(this, &SFlowGraphNode_YapDialogueWidget::MoveFragment, false, &Fragment)
								.Visibility(this, &SFlowGraphNode_YapDialogueWidget::GetMoveFragmentDownVisibility, &Fragment)
								[
									SNew(SImage)
									.Image(FAppStyle::Get().GetBrush("Symbols.DownArrow"))
									.DesiredSizeOverride(FVector2D(8,8))
									.ColorAndOpacity(DialogueButtonsColor)
								]
							]
						]
					]
					+ SOverlay::Slot()
					[
						SAssignNew(RightSide, SVerticalBox)
					]
				]
			]
		];
		
		int32 ActivationCount = Fragment.GetActivationCount();
		int32 ActivationLimit = Fragment.GetActivationLimit();

		if (ActivationLimit <= 4)
		{
			for (int i = 0; i < 4; ++i)
			{
				int32 Size = 12;// i < ActivationCount ? 16 : 12;
				LeftSideActivationIndicator->AddSlot()
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Center)
				.AutoHeight()
				.Padding(0)
				[
					SNew(SBox)
					.WidthOverride(16)
					.HeightOverride(12)
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					.Padding(0)
					.ToolTipText(FText::Format(LOCTEXT("DialogueNode_Tooltip", "Set Activation Limit to {0}"), i + 1))
					[
						SNew(SButton)
						.ButtonStyle(FCoreStyle::Get(), "SimpleButton")
						.ContentPadding(0)
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Center)
						.OnClicked(this, &SFlowGraphNode_YapDialogueWidget::OnClickedActivationDot, &Fragment, i)
						.ToolTipText(FText::Format(LOCTEXT("DialogueNode_Tooltip", "Set Activation Limit to {0}"), i + 1))
						[
							SNew(SImage)
							.DesiredSizeOverride(FVector2D(Size, Size))
							.Image(FAppStyle::GetBrush("Icons.BulletPoint"))
							.ColorAndOpacity(this, &SFlowGraphNode_YapDialogueWidget::GetActivationDotColor, &Fragment, i)
						]
					]
				];
			}
		}
		else
		{			
			LeftSideActivationIndicator->AddSlot()
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Center)
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(FText::Join(FText::FromString("/"), FText::AsNumber(ActivationCount), FText::AsNumber(Fragment.GetActivationLimit())))
				.ColorAndOpacity(FlowYapColor::White)
				.TextStyle(&NormalText)
				.Justification(ETextJustify::Center)
			];
		}
		
		FragmentInputBoxes.Add(LeftSide);
		FragmentOutputBoxes.Add(RightSide);
	};

	// BOTTOM BAR
	FragmentBox->AddSlot()
	.AutoHeight()
	.Padding(0,2,0,0)
	[
		SNew(SSeparator)
		.SeparatorImage(FAppStyle::Get().GetBrush("Menu.Separator"))
		.Orientation(Orient_Horizontal)
		.Thickness(3.0f)
		.ColorAndOpacity(this, &SFlowGraphNode_YapDialogueWidget::GetFragmentSeparatorColor)
	];
	
	FragmentBox->AddSlot()
	.AutoHeight()
	.Padding(0,0)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.HAlign(HAlign_Left)
		[
			SNew(SBox)
			.WidthOverride(40)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			.Padding(8, 0)
			[
				SNew(SButton)
				.ButtonStyle(FAppStyle::Get(), "SimpleButton")
				.ContentPadding(2)
				.Visibility(this, &SFlowGraphNode_YapDialogueWidget::GetDialogueCycleFragmentSequencingVisibility)
				.OnClicked(this, &SFlowGraphNode_YapDialogueWidget::HandleDialogueCycleFragmentSequencingClicked)
				.ToolTipText(this, &SFlowGraphNode_YapDialogueWidget::GetDialogueCycleFragmentSequencingTooltip)
				[
					SNew(SImage)
					.ColorAndOpacity(this, &SFlowGraphNode_YapDialogueWidget::GetDialogueCycleFragmentSequencingColor)
					.DesiredSizeOverride(FVector2D(16, 16))
					.Image(this, &SFlowGraphNode_YapDialogueWidget::GetDialogueFragmentSequencingIcon)
				]
			]
		]
		+ SHorizontalBox::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SNew(SButton)
			.HAlign(HAlign_Center)
			.ButtonStyle(FAppStyle::Get(), "SimpleButton")
			.ToolTipText(LOCTEXT("DialogueAddFragment_Tooltip", "Add Fragment"))
			.Visibility(this, &SFlowGraphNode_YapDialogueWidget::GetDialogueFragmentButtonsVisibility)
			.OnClicked(this, &SFlowGraphNode_YapDialogueWidget::AddFragment)
			[
				SNew(SBox)
				.VAlign(VAlign_Center)
				[
					SNew(SImage)
					.Image(FAppStyle::GetBrush(TEXT("Icons.PlusCircle")))
					.ColorAndOpacity(DialogueButtonsColor)
				]
			]
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.HAlign(HAlign_Center)
		.Padding(0, 0, 2, 0)
		[
			SAssignNew(BypassOutputBox, SBox)
			.HAlign(HAlign_Center)
			.WidthOverride(40)
		]
	];

	return SNew(SBorder)
	.BorderImage(FAppStyle::GetBrush("NoBorder"))
	.HAlign(HAlign_Fill)
	.VAlign(VAlign_Fill)
	[
		FragmentBox.ToSharedRef()
	];
}

ECheckBoxState SFlowGraphNode_YapDialogueWidget::GetIsUserPromptDialogue() const
{
	return GetFlowYapDialogueNode()->GetIsPlayerPrompt() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SFlowGraphNode_YapDialogueWidget::HandleUserPromptDialogueChanged(ECheckBoxState CheckBoxState)
{
	GetFlowYapDialogueNode()->SetIsPlayerPrompt(CheckBoxState == ECheckBoxState::Checked ? true : false);
}

TSharedRef<SWidget> SFlowGraphNode_YapDialogueWidget::CreateTitleWidget(TSharedPtr<SNodeTitle> NodeTitle)
{
	TSharedRef<SWidget> Title = SFlowGraphNode::CreateTitleWidget(NodeTitle);

	return SNew(SHorizontalBox)
	+ SHorizontalBox::Slot()
	.FillWidth(1.0)
	.HAlign(HAlign_Fill)
	.Padding(12,0,2,0)
	[
		Title
	]
	+ SHorizontalBox::Slot()
	.HAlign(HAlign_Right)
	.AutoWidth()
	.Padding(2,0,12,0)
	[
		SNew(SCheckBox)
		.Style(&UFlowYapEditorSubsystem::GetCheckBoxStyles().ToggleButtonCheckBox_White)
		.Padding(FMargin(4, 0))
		.CheckBoxContentUsesAutoWidth(true)
		.ToolTipText(LOCTEXT("DialogueNode_Tooltip", "Toggle Player Prompt Node"))
		.IsChecked(this, &SFlowGraphNode_YapDialogueWidget::GetIsUserPromptDialogue)
		.OnCheckStateChanged(this, &SFlowGraphNode_YapDialogueWidget::HandleUserPromptDialogueChanged)
		.Content()
		[
			SNew(SBox)
			.WidthOverride(66)
			.HeightOverride(16)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(SImage)
				.ColorAndOpacity(FlowYapColor::Noir)
				.DesiredSizeOverride(FVector2D(16, 16))
				.Image(FAppStyle::Get().GetBrush("MainFrame.VisitForums"))
			]
		]
	];
}

EVisibility SFlowGraphNode_YapDialogueWidget::GetDialogueFragmentButtonsVisibility() const
{
	return IsHovered() ? EVisibility::Visible : EVisibility::Hidden;
}

void SFlowGraphNode_YapDialogueWidget::AddPin(const TSharedRef<SGraphPin>& PinToAdd)
{
	// TODO OH MY GOD THIS IS JANKY. CAN I DEJANK IT AT ALL!?!?
	
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
		int32 Index = InputPins.Num();

		if (!FragmentInputBoxes.IsValidIndex(Index))
		{
			UE_LOG(FlowYap, Warning, TEXT("COULD NOT ADD INPUT PIN: %s, perhaps node is corrupt?"
								 "This might fix itself by refreshing the graph and saving. "
								 "Otherwise, consider disconnecting all pins and cycling Multiple Inputs off and on. "
								 "First Node dialogue entry: %s"),
								 *PinObj->GetName(),
								 *GetFlowYapDialogueNode()->GetFragments()[0].GetDialogueText().ToString());
			return;
		}
		
		FragmentInputBoxes[Index]->InsertSlot(0)
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Top)
		.AutoHeight()
		.Padding(LeftMargins)
		[
			PinToAdd
		];

		PinToAdd->SetToolTipText(FText::Format(LOCTEXT("DialogueNode", "Input {0}"), Index));
		PinToAdd->SetColorAndOpacity((PinToAdd->IsConnected() || InputPins.Num() == 0) ? ConnectedPinColor : DisconnectedPinColor);

		InputPins.Add(PinToAdd);
	}
	else
	{
		if (PinToAdd->GetPinObj()->GetFName() == FName("Bypass"))
		{
			AddBypassPin(PinToAdd);
		}
		else
		{
			// We don't have to account for the bypass pin because it is added last
			int32 Index = OutputPins.Num();

			if (!FragmentOutputBoxes.IsValidIndex(Index))
			{
				UE_LOG(FlowYap, Warning, TEXT("COULD NOT ADD OUTPUT PIN: %s, perhaps node is corrupt?"
								"This might fix itself by refreshing the graph and saving. "
								"Consider disconnecting all pins and cycling Multiple Outputs off and on. "
								"First Node dialogue entry: %s"),
								*PinObj->GetName(),
								*GetFlowYapDialogueNode()->GetFragments()[0].GetDialogueText().ToString());
				return;
			}
			
			FragmentOutputBoxes[Index]->InsertSlot(0)
			.HAlign(HAlign_Right)
			.VAlign(VAlign_Top)
			.AutoHeight()
			.Padding(RightMargins)
			[
				PinToAdd
			];

			PinToAdd->SetToolTipText(FText::Format(LOCTEXT("DialogueNode", "Output {0}"), Index));
			PinToAdd->SetColorAndOpacity((PinToAdd->IsConnected() || OutputPins.Num() == 0) ? ConnectedPinColor : DisconnectedPinColor);

			OutputPins.Add(PinToAdd);	
		}
	}
}

void SFlowGraphNode_YapDialogueWidget::AddBypassPin(const TSharedRef<SGraphPin>& PinToAdd)
{
	BypassOutputBox->SetContent(PinToAdd);
	PinToAdd->SetToolTipText(LOCTEXT("Dialogue", "Bypass, executes immediately when this node (or all fragments) have reached activation limits"));
	PinToAdd->SetColorAndOpacity(PinToAdd->IsConnected() ? ConnectedBypassPinColor : DisconnectedBypassPinColor);
	OutputPins.Add(PinToAdd);
}

FReply SFlowGraphNode_YapDialogueWidget::AddFragment()
{
	FFlowYapTransactions::BeginModify(LOCTEXT("DialogueAddFragment", "Add Fragment"), GetFlowYapDialogueNode());
	
	GetFlowYapDialogueNode()->AddFragment();

	UpdateGraphNode();

	FFlowYapTransactions::EndModify();
	
	return FReply::Handled();
}

FReply SFlowGraphNode_YapDialogueWidget::DeleteFragment(FFlowYapFragment* Fragment)
{
	FFlowYapTransactions::BeginModify(LOCTEXT("DialogueDeleteFragment", "Delete Fragment"), GetFlowYapDialogueNode());

	int16 Index = GetFlowYapDialogueNode()->FindFragmentIndex(Fragment);

	if (Index == INDEX_NONE)
	{
		return FReply::Handled();
	}
		
	FlowGraphNode_YapDialogue->PreparePinsForFragmentDeletion(Index);

	GetFlowYapDialogueNode()->DeleteFragmentByIndex(Index);

	UpdateGraphNode();

	FFlowYapTransactions::EndModify();

	return FReply::Handled();
}

inline bool SFlowGraphNode_YapDialogueWidget::GetNormalisedMousePositionInGeometry(UObject* WorldContextObject, FGeometry Geometry, FVector2D& Position) const
{
	FVector2D pixelPosition;
	FVector2D viewportMinPosition;
	FVector2D viewportMaxPosition;

	// Get the top left and bottom right viewport positions
	USlateBlueprintLibrary::LocalToViewport( WorldContextObject, Geometry, FVector2D( 0, 0 ), pixelPosition, viewportMinPosition );
	USlateBlueprintLibrary::LocalToAbsolute(Geometry, pixelPosition);

	// Get the mouse's current position in the viewport
	FVector2D mousePosition = UWidgetLayoutLibrary::GetMousePositionOnViewport( WorldContextObject );

	// Determine where the mouse is relative to the widget geometry
	Position = ( mousePosition - viewportMinPosition ) / ( viewportMaxPosition - viewportMinPosition );

	// Calculate if the mouse is inside the geometry or not
	return FMath::Min( Position.X, Position.Y ) >= 0.f && FMath::Max( Position.X, Position.Y ) <= 1.f;
}

const FSlateBrush* SFlowGraphNode_YapDialogueWidget::GetShadowBrush(bool bSelected) const
{
	if (GEditor->PlayWorld)
	{
		switch (FlowGraphNode->GetActivationState())
		{
		case EFlowNodeState::NeverActivated:
			return SGraphNode::GetShadowBrush(bSelected);
		case EFlowNodeState::Active:
			return FFlowEditorStyle::Get()->GetBrush(TEXT("Flow.Node.ActiveShadow"));
		case EFlowNodeState::Completed:
		case EFlowNodeState::Aborted:
			return FFlowEditorStyle::Get()->GetBrush(TEXT("Flow.Node.WasActiveShadow"));
		default:
			return FAppStyle::GetBrush(TEXT("Graph.Node.Shadow"));
		}
	}
	else
	{
		if (bSelected)
		{
			return FAppStyle::GetBrush(TEXT("Graph.Node.ShadowSelected"));
		}
		else
		{
			if (GetFlowYapDialogueNode()->GetIsPlayerPrompt())
			{
				// TODO a custom brush style could be less obtuse
				return FAppStyle::GetBrush(TEXT("Graph.ReadOnlyBorder"));
			}
			else
			{
				return FAppStyle::GetBrush(TEXT("Graph.Node.Shadow"));
			}
		}
	}
}

void SFlowGraphNode_YapDialogueWidget::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	TSharedPtr<SFlowGraphEditor> GraphEditor = FFlowGraphUtils::GetFlowGraphEditor(this->FlowGraphNode->GetGraph());

	bIsSelected = GraphEditor->GetSelectedFlowNodes().Contains(FlowGraphNode);

	bool bControlPressed = GEditor->GetEditorSubsystem<UFlowYapEditorSubsystem>()->GetInputTracker()->GetControlPressed();

	if (bIsSelected)
	{
		bWasSelected = true;
	}
	
	if (bIsSelected && bControlPressed)
	{
		bControlHooked = true;
	}

	if (!bIsSelected)
	{
		bWasSelected = false;
		bControlHooked = false;
		FocusedFragment = nullptr;
	}
}

bool SFlowGraphNode_YapDialogueWidget::GetIsSelected() const
{
	return bIsSelected;
}

bool SFlowGraphNode_YapDialogueWidget::GetControlHooked() const
{
	return bControlHooked;
}

void SFlowGraphNode_YapDialogueWidget::SetFocusedFragment(const SFlowGraphNode_YapFragmentWidget* InFragment)
{
	if (FocusedFragment != InFragment)
	{
		TSharedPtr<SFlowGraphEditor> GraphEditor = FFlowGraphUtils::GetFlowGraphEditor(this->FlowGraphNode->GetGraph());
		GraphEditor->SetNodeSelection(FlowGraphNode, true);
		
		FocusedFragment = InFragment;
	}
}

void SFlowGraphNode_YapDialogueWidget::ClearFocusedFragment(const SFlowGraphNode_YapFragmentWidget* InFragment)
{
	if (FocusedFragment == InFragment)
	{
		FocusedFragment = nullptr;
	}
}

const SFlowGraphNode_YapFragmentWidget* SFlowGraphNode_YapDialogueWidget::GetFocusedFragment() const
{
	return FocusedFragment;
}

#undef LOCTEXT_NAMESPACE
