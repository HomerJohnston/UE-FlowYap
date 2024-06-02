// Copyright Ghost Pepper Games, Inc. All Rights Reserved.

#include "Widgets/SFlowGraphNode_YapDialogueWidget.h"

#include "FlowEditorStyle.h"
#include "FlowYapColors.h"
#include "FlowYapEditorSubsystem.h"
#include "FlowYapTransactions.h"
#include "GraphEditorSettings.h"
#include "Blueprint/SlateBlueprintLibrary.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "FlowYap/FlowYapLog.h"
#include "FlowYap/Nodes/FlowNode_YapDialogue.h"
#include "GraphNodes/FlowGraphNode_YapDialogue.h"
#include "Widgets/SFlowGraphNode_YapFragmentWidget.h"
#include "Widgets/Layout/SSeparator.h"

#define LOCTEXT_NAMESPACE "FlowYap"

void SFlowGraphNode_YapDialogueWidget::Construct(const FArguments& InArgs, UFlowGraphNode* InNode)
{
	GraphNode = InNode;
	FlowGraphNode = InNode;
	FlowGraphNode_YapDialogue = Cast<UFlowGraphNode_YapDialogue>(InNode);

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
	if (GetFlowYapDialogueNode()->GetNumFragments() > 1)
	{
		return EVisibility::Visible;
	}

	return EVisibility::Hidden;
}

FSlateColor SFlowGraphNode_YapDialogueWidget::GetFragmentMovementControlsColor() const
{
	return IsHovered() ? FlowYapColors::Gray : FlowYapColors::DarkGray;
}

FReply SFlowGraphNode_YapDialogueWidget::MoveFragment(bool bUp, int64 EditorID)
{
	FFlowYapTransactions::BeginModify(LOCTEXT("DialogueNode", "Move Fragment"), GetFlowYapDialogueNode());
	
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
	Fragment->SetActivationLimit(NewValue);
}

FSlateColor SFlowGraphNode_YapDialogueWidget::GetActivationDotColor(FFlowYapFragment* Fragment, int32 ActivationIndex) const
{
	int32 Count = Fragment->GetActivationCount();
	int32 Limit = Fragment->GetActivationLimit();
	int32 Current = ActivationIndex + 1;
	
	// Make them all red if the limit has been hit
	if (Limit > 0 && Count >= Limit && Current <= Count)
	{
		return FlowYapColors::Red;
	}

	// Draw "available" ones gray
	if (Current > Count && Current <= Limit)
	{
		return FlowYapColors::DimGray;
	}

	// Draw points that are done with a limit set white
	if (Current <= Count && Limit > 0)
	{
		return FlowYapColors::White;
	}

	// Draw points that are done without a limit set green
	if (Current <= Count)
	{
		return FlowYapColors::LightGreen;
	}

	// Draw all other pins black
	return FlowYapColors::Noir;
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
			.Padding(0,7,0,8)
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
					SNew(SOverlay)
					+ SOverlay::Slot()
					.VAlign(VAlign_Top)
					[
						SAssignNew(LeftSide, SVerticalBox)
					]
					+ SOverlay::Slot()
					.VAlign(VAlign_Center)
					.HAlign(HAlign_Center)
					[
						SAssignNew(LeftSideActivationIndicator, SVerticalBox)
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
						.VAlign(VAlign_Center)
						.Visibility(this, &SFlowGraphNode_YapDialogueWidget::GetFragmentMovementVisibility)
						[
							// CONTROLS FOR UP/DELETE/DOWN
							SNew(SVerticalBox)
							// UP
							+ SVerticalBox::Slot()
							.AutoHeight()
							.VAlign(VAlign_Center)
							.HAlign(HAlign_Center)
							.Padding(0, 2)
							[
								SNew(SButton)
								.ButtonStyle(FAppStyle::Get(), "SimpleButton")
								.ContentPadding(FMargin(8, 8))
								.ToolTipText(LOCTEXT("DialogueMoveFragmentUp_Tooltip", "Move Fragment Up"))
								.OnClicked(this, &SFlowGraphNode_YapDialogueWidget::MoveFragment, true, Fragment.GetEditorID())
								[
									SNew(SImage)
									.Image(FAppStyle::Get().GetBrush("Symbols.UpArrow"))
									.ColorAndOpacity(this, &SFlowGraphNode_YapDialogueWidget::GetFragmentMovementControlsColor)
								]
							]
							// DELETE
							+ SVerticalBox::Slot()
							.AutoHeight()
							.VAlign(VAlign_Center)
							.HAlign(HAlign_Center)
							.Padding(0, 2)
							[
								SNew(SButton)
								.ButtonStyle(FAppStyle::Get(), "SimpleButton")
								.ToolTipText(LOCTEXT("DialogueDeleteFragment_Tooltip", "Delete Fragment"))
								.OnClicked(this, &SFlowGraphNode_YapDialogueWidget::DeleteFragment, Fragment.GetEditorID())
								[
									SNew(SImage)
									.Image(FAppStyle::GetBrush("Icons.Delete"))
									.ColorAndOpacity(this, &SFlowGraphNode_YapDialogueWidget::GetFragmentMovementControlsColor)
								]
							]
							// DOWN
							+ SVerticalBox::Slot()
							.AutoHeight()
							.VAlign(VAlign_Center)
							.HAlign(HAlign_Center)
							.Padding(0, 2)
							[
								SNew(SButton)
								.ButtonStyle(FAppStyle::Get(), "SimpleButton")
								.ContentPadding(FMargin(8, 8))
								.ToolTipText(LOCTEXT("DialogueMoveFragmentDown_Tooltip", "Move Fragment Down"))
								.OnClicked(this, &SFlowGraphNode_YapDialogueWidget::MoveFragment, false, Fragment.GetEditorID())
								[
									SNew(SImage)
									.Image(FAppStyle::Get().GetBrush("Symbols.DownArrow"))
									.ColorAndOpacity(this, &SFlowGraphNode_YapDialogueWidget::GetFragmentMovementControlsColor)
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

		if (ActivationLimit <= 5)
		{
			for (int i = 0; i < 5; ++i)
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
					.HeightOverride(14)
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
				.ColorAndOpacity(FlowYapColors::White)
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
	.Padding(0,8,0,0)
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
		]
		+ SHorizontalBox::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SNew(SButton)
			.HAlign(HAlign_Center)
			.ButtonStyle(FAppStyle::Get(), "SimpleButton")
			.ToolTipText(LOCTEXT("DialogueAddFragment_Tooltip", "Add Fragment"))
			.Visibility(this, &SFlowGraphNode_YapDialogueWidget::GetAddFragmentButtonVisibility)
			.OnClicked(this, &SFlowGraphNode_YapDialogueWidget::AddFragment)
			[
				SNew(SBox)
				.VAlign(VAlign_Center)
				[
					SNew(SImage)
					.Image(FAppStyle::GetBrush(TEXT("Icons.PlusCircle")))
					.ColorAndOpacity(this, &SFlowGraphNode_YapDialogueWidget::GetAddFragmentButtonColor)
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

	return SNew(SOverlay)
	+ SOverlay::Slot()
	[
		Title
	]
	+ SOverlay::Slot()
	.HAlign(HAlign_Right)
	.VAlign(VAlign_Fill)
	.Padding(0,0,12,0)
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
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Fill)
			.Padding(6,0,6,1)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("DialogueNode", "Prompt"))
				.ColorAndOpacity(FlowYapColors::DimGray)
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(4,0,0,0)
			[
				SNew(SImage)
				.ColorAndOpacity(FlowYapColors::DimGray)
				.Image(FAppStyle::Get().GetBrush("MainFrame.VisitForums"))
			]
		]
	];
}

EVisibility SFlowGraphNode_YapDialogueWidget::GetAddFragmentButtonVisibility() const
{
	return IsHovered() ? EVisibility::Visible : EVisibility::Hidden;
}

FSlateColor SFlowGraphNode_YapDialogueWidget::GetAddFragmentButtonColor() const
{
	return IsHovered() ? FlowYapColors::Gray : FlowYapColors::DarkGray;
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
		
		InputPins.Add(PinToAdd);
	}
	else
	{
		if (PinToAdd->GetPinObj()->GetFName() == FName("Bypass"))
		{
			AddBypassPin(PinToAdd);

			PinToAdd->SetToolTipText(LOCTEXT("Dialogue", "Bypass, executes immediately when this node (or all fragments) have reached activation limits"));
		}
		else
		{
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

			OutputPins.Add(PinToAdd);	
		}
	}
}

void SFlowGraphNode_YapDialogueWidget::AddBypassPin(const TSharedRef<SGraphPin>& PinToAdd)
{
	PinToAdd->SetColorAndOpacity(FlowYapColors::LightBlue);
	BypassOutputBox->SetContent(PinToAdd);
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

FReply SFlowGraphNode_YapDialogueWidget::DeleteFragment(int64 FragmentID)
{
	FFlowYapTransactions::BeginModify(LOCTEXT("DialogueDeleteFragment", "Delete Fragment"), GetFlowYapDialogueNode());

	GetFlowYapDialogueNode()->RemoveFragmentByID(FragmentID);

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

#undef LOCTEXT_NAMESPACE