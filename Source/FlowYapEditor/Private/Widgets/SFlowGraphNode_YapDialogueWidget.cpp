// Copyright Ghost Pepper Games, Inc. All Rights Reserved.

#include "Widgets/SFlowGraphNode_YapDialogueWidget.h"

#include "FlowEditorStyle.h"
#include "FlowYapColors.h"
#include "FlowYapEditorSubsystem.h"
#include "FlowYapInputTracker.h"
#include "FlowYapTransactions.h"
#include "GraphEditorSettings.h"
#include "LandscapeRender.h"
#include "Blueprint/SlateBlueprintLibrary.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "FlowYap/FlowYapLog.h"
#include "FlowYap/FlowYapProjectSettings.h"
#include "FlowYap/Enums/FlowYapErrorLevel.h"
#include "FlowYap/Nodes/FlowNode_YapDialogue.h"
#include "Graph/FlowGraphEditor.h"
#include "Graph/FlowGraphSettings.h"
#include "Graph/FlowGraphUtils.h"
#include "GraphNodes/FlowGraphNode_YapDialogue.h"
#include "Widgets/SFlowGraphNode_YapFragmentWidget.h"

#define LOCTEXT_NAMESPACE "FlowYap"

void SFlowGraphNode_YapDialogueWidget::Construct(const FArguments& InArgs, UFlowGraphNode* InNode)
{	
	GraphNode = InNode;
	FlowGraphNode = InNode;
	FlowGraphNode_YapDialogue = Cast<UFlowGraphNode_YapDialogue>(InNode);

	DialogueButtonsColor = FlowYapColor::DimGray;

	ConnectedEndPinColor = FlowYapColor::White;
	DisconnectedEndPinColor = FlowYapColor::DarkGray;
	
	ConnectedStartPinColor = FlowYapColor::LightGreen;
	DisconnectedStartPinColor = FlowYapColor::DeepGreen;
	
	ConnectedInterruptPinColor = FlowYapColor::LightRed;
	DisconnectedInterruptPinColor = FlowYapColor::DeepRed;
	ConnectedInterruptPinColor_Disabled = ConnectedInterruptPinColor.Desaturate(0.75);
	DisconnectedInterruptPinColor_Disabled = DisconnectedInterruptPinColor.Desaturate(0.75);
	
	ConnectedBypassPinColor = FlowYapColor::LightBlue;
	DisconnectedBypassPinColor = FlowYapColor::DarkBlue;

	SelectedFragmentWidget.Reset();
	KeyboardFocusedFragmentWidget.Reset();
	
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

/*
EVisibility SFlowGraphNode_YapDialogueWidget::GetFragmentMovementVisibility(FFlowYapFragment* Fragment) const
{	
	if (bShiftHooked || (SelectedFragmentWidget && SelectedFragmentWidget.IsSet() && SelectedFragmentWidget.GetValue() == Fragment->IndexInDialogue))// IsHovered() && GetFlowYapDialogueNode()->GetNumFragments() > 1)
	{
		return EVisibility::Visible;
	}

	return EVisibility::Collapsed;
}

EVisibility SFlowGraphNode_YapDialogueWidget::GetFragmentDeleteVisibility(FFlowYapFragment* FlowYapFragment) const
{
	return (GetFragmentMovementVisibility(FlowYapFragment) == EVisibility::Visible) ? EVisibility::Collapsed : EVisibility::Visible;
}
*/

void SFlowGraphNode_YapDialogueWidget::MoveFragmentUp(uint8 FragmentIndex)
{
	check(FragmentIndex > 0);
	MoveFragment(FragmentIndex, -1);
}

void SFlowGraphNode_YapDialogueWidget::MoveFragmentDown(uint8 FragmentIndex)
{
	check(FragmentIndex < GetFlowYapDialogueNode()->GetNumFragments() - 1);
	MoveFragment(FragmentIndex, +1);
}

void SFlowGraphNode_YapDialogueWidget::MoveFragment(uint8 FragmentIndex, int16 By)
{
	FlowGraphNode_YapDialogue->SwapPinConnections(FragmentIndex, FragmentIndex + By);

	GetFlowYapDialogueNode()->SwapFragments(FragmentIndex, FragmentIndex + By);

	if (SelectedFragmentWidget.IsSet())
	{
		SelectedFragmentWidget = SelectedFragmentWidget.GetValue() + By;
	}
}

FSlateColor SFlowGraphNode_YapDialogueWidget::GetFragmentSeparatorColor() const
{
	return FlowYapColor::Black;
}

TOptional<int32> SFlowGraphNode_YapDialogueWidget::GetActivationLimit(FFlowYapFragment* Fragment) const
{
	int32 Limit = Fragment->ActivationLimit;

	return Limit == 0 ? TOptional<int32>() : Limit;
}

void SFlowGraphNode_YapDialogueWidget::OnActivationLimitChanged(int32 NewValue, FFlowYapFragment* Fragment)
{
	FFlowYapTransactions::BeginModify(LOCTEXT("DialogueNode", "Change Activation Limit"), GetFlowYapDialogueNode());

	Fragment->ActivationLimit = NewValue;

	FFlowYapTransactions::EndModify();
}

FSlateColor SFlowGraphNode_YapDialogueWidget::ActivationDot_ColorAndOpacity(FFlowYapFragment* Fragment, int32 ActivationIndex) const
{
	int32 Count = Fragment->ActivationCount;
	int32 Limit = Fragment->ActivationLimit;
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

FReply SFlowGraphNode_YapDialogueWidget::ActivationDot_OnClicked(FFlowYapFragment* Fragment, int ActivationIndex)
{
	// TODO ignore input during PIE?
	if (Fragment->ActivationLimit == ActivationIndex + 1)
	{
		Fragment->ActivationLimit = 0;
	}
	else
	{
		Fragment->ActivationLimit = ActivationIndex + 1;
	}

	return FReply::Handled();
}

EVisibility SFlowGraphNode_YapDialogueWidget::GetActivationIndicatorVisibility(SFlowGraphNode_YapDialogueWidget* FlowGraphNode_YapDialogueWidget, FFlowYapFragment* FlowYapFragment) const
{
	if (GEditor->PlayWorld)
	{
		return (FlowYapFragment->ActivationLimit > 0 || FlowYapFragment->ActivationCount > 0) ? EVisibility::Visible : EVisibility::Collapsed;		
	}
	else
	{
		if (IsHovered())
		{
			return EVisibility::Visible;
		}

		return (FlowYapFragment->ActivationLimit > 0) ? EVisibility::Visible : EVisibility::Collapsed;	
	}
}

EVisibility SFlowGraphNode_YapDialogueWidget::FragmentSequencingButton_Visibility() const
{
	if (GetFlowYapDialogueNode()->GetIsPlayerPrompt())
	{
		return EVisibility::Hidden; // Should be Collapsed but that destroys the parent widget layout for some reason
	}
	
	return (GetFlowYapDialogueNode()->GetNumFragments() > 1) ? EVisibility::Visible : EVisibility::Hidden;
}

FReply SFlowGraphNode_YapDialogueWidget::FragmentSequencingButton_OnClicked()
{
	FFlowYapTransactions::BeginModify(LOCTEXT("DialogueNodeChangeSequencing", "Change dialogue node sequencing setting"), GetFlowYapDialogueNode());

	GetFlowYapDialogueNode()->CycleFragmentSequencingMode();
	
	return FReply::Handled();
}

const FSlateBrush* SFlowGraphNode_YapDialogueWidget::FragmentSequencingButton_Image() const
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

FText SFlowGraphNode_YapDialogueWidget::FragmentSequencingButton_ToolTipText() const
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

FSlateColor SFlowGraphNode_YapDialogueWidget::FragmentSequencingButton_ColorAndOpacity() const
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

FReply SFlowGraphNode_YapDialogueWidget::InsertFragment(int Index)
{
	FFlowYapTransactions::BeginModify(LOCTEXT("DialogueAddFragment", "Add Fragment"), GetFlowYapDialogueNode());

	GetFlowYapDialogueNode()->InsertFragment(Index);

	FlowGraphNode_YapDialogue->UpdatePinsAfterFragmentInsertion(Index);

	UpdateGraphNode();

	FFlowYapTransactions::EndModify();
	
	return FReply::Handled();
}

TSharedRef<SWidget> SFlowGraphNode_YapDialogueWidget::CreateNodeContentArea()
{
	SAssignNew(FragmentBox, SVerticalBox);

	bool bFirstFragment = true;

	bool bSingleFragment = GetFlowYapDialogueNode()->GetNumFragments() == 1;

	bool bLastFragment = false;

	//for (FFlowYapFragment& Fragment : GetFlowYapDialogueNode()->GetFragments())
	for (int f = 0; f < GetFlowYapDialogueNode()->GetNumFragments(); ++f)
	{
		FFlowYapFragment& Fragment = GetFlowYapDialogueNode()->GetFragmentsMutable()[f];

		if (f == GetFlowYapDialogueNode()->GetNumFragments() - 1)
		{
			bLastFragment = true;
		}
		
		TSharedPtr<SFlowGraphNode_YapFragmentWidget> NewFragmentWidget = MakeShared<SFlowGraphNode_YapFragmentWidget>();

		FragmentWidgets.Add(NewFragmentWidget);

		TSharedPtr<SVerticalBox> LeftSide;
		TSharedPtr<SVerticalBox> RightSide;

		FragmentBox->AddSlot()
		.AutoHeight()
		.Padding(0, 0, 0, 0)
		[
			SNew(SButton)
			.ButtonStyle(FAppStyle::Get(), "SimpleButton")
			.ToolTipText(LOCTEXT("DialogueNode", "Insert new fragment"))
			.ContentPadding(0)
			.OnClicked(this, &SFlowGraphNode_YapDialogueWidget::InsertFragment, f)
			[
				SNew(SImage)
				.Image(FAppStyle::GetBrush("Menu.Separator"))
				.DesiredSizeOverride(FVector2D(1, 2))
				.ColorAndOpacity(this, &SFlowGraphNode_YapDialogueWidget::GetFragmentSeparatorColor)
			]
		];	

		TSharedPtr<SVerticalBox> LeftSideActivationIndicator;
		
		FragmentBox->AddSlot()
		.AutoHeight()
		[
			SNew(SHorizontalBox)
			// LEFT PANE
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0, 0, 0, 0)
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
						.MinDesiredHeight(16)
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
			.Padding(0, bFirstFragment ? 2 : 12, 0, bSingleFragment || bLastFragment ? 2 : 12)
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
					SAssignNew(RightSide, SVerticalBox)
				]
			]
		];
		
		int32 ActivationCount = Fragment.GetActivationCount();
		int32 ActivationLimit = Fragment.GetActivationLimit();

		if (ActivationLimit <= 3) // TODO 3 twice should be a gvar
		{
			for (int i = 0; i < 3; ++i)
			{
				int32 Size = 16;// i < ActivationCount ? 16 : 12;
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
						.OnClicked(this, &SFlowGraphNode_YapDialogueWidget::ActivationDot_OnClicked, &Fragment, i)
						.ToolTipText(FText::Format(LOCTEXT("DialogueNode_Tooltip", "Set Activation Limit to {0}"), i + 1))
						[
							SNew(SImage)
							.DesiredSizeOverride(FVector2D(Size, Size))
							.Image(FAppStyle::GetBrush("Icons.BulletPoint"))
							.ColorAndOpacity(this, &SFlowGraphNode_YapDialogueWidget::ActivationDot_ColorAndOpacity, &Fragment, i)
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
		bFirstFragment = false;
	};
	
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
				.Visibility(this, &SFlowGraphNode_YapDialogueWidget::FragmentSequencingButton_Visibility)
				.OnClicked(this, &SFlowGraphNode_YapDialogueWidget::FragmentSequencingButton_OnClicked)
				.ToolTipText(this, &SFlowGraphNode_YapDialogueWidget::FragmentSequencingButton_ToolTipText)
				[
					SNew(SImage)
					.ColorAndOpacity(this, &SFlowGraphNode_YapDialogueWidget::FragmentSequencingButton_ColorAndOpacity)
					.DesiredSizeOverride(FVector2D(16, 16))
					.Image(this, &SFlowGraphNode_YapDialogueWidget::FragmentSequencingButton_Image)
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
			.Visibility(this, &SFlowGraphNode_YapDialogueWidget::BottomAddFragmentButton_Visibility)
			.OnClicked(this, &SFlowGraphNode_YapDialogueWidget::BottomAddFragmentButton_OnClicked)
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

ECheckBoxState SFlowGraphNode_YapDialogueWidget::PlayerPromptCheckBox_IsChecked() const
{
	return GetFlowYapDialogueNode()->GetIsPlayerPrompt() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SFlowGraphNode_YapDialogueWidget::PlayerPromptCheckBox_OnCheckStateChanged(ECheckBoxState CheckBoxState)
{
	GetFlowYapDialogueNode()->SetIsPlayerPrompt(CheckBoxState == ECheckBoxState::Checked ? true : false);
}

ECheckBoxState SFlowGraphNode_YapDialogueWidget::InterruptibleToggle_IsChecked() const
{
	switch (GetFlowYapDialogueNode()->Interruptible)
	{
	case EFlowYapInterruptible::UseProjectDefaults:
		{
			return ECheckBoxState::Undetermined;
		}
	case EFlowYapInterruptible::NotInterruptible:
		{
			return ECheckBoxState::Unchecked;
		}
	case EFlowYapInterruptible::Interruptible:
		{
			return ECheckBoxState::Checked;
		}
	}

	check(false);
	return ECheckBoxState::Undetermined;
}

void SFlowGraphNode_YapDialogueWidget::InterruptibleToggle_OnCheckStateChanged(ECheckBoxState CheckBoxState)
{
	if (GEditor->GetEditorSubsystem<UFlowYapEditorSubsystem>()->GetInputTracker()->GetControlPressed())
	{
		GetFlowYapDialogueNode()->Interruptible = EFlowYapInterruptible::UseProjectDefaults;
	}
	else if (CheckBoxState == ECheckBoxState::Checked)
	{
		GetFlowYapDialogueNode()->Interruptible = EFlowYapInterruptible::Interruptible;
	}
	else
	{
		GetFlowYapDialogueNode()->Interruptible = EFlowYapInterruptible::NotInterruptible;
	}
}

TSharedRef<SWidget> SFlowGraphNode_YapDialogueWidget::CreateTitleWidget(TSharedPtr<SNodeTitle> NodeTitle)
{
	TSharedRef<SWidget> Title = SFlowGraphNode::CreateTitleWidget(NodeTitle);

	TSharedPtr<SCheckBox> InterruptibleCheckBox;

	TestStyle = UFlowYapEditorSubsystem::GetCheckBoxStyles().ToggleButtonCheckBox_White;

	TestStyle.CheckedImage = *FAppStyle::Get().GetBrush("Icons.Rotate180");
	TestStyle.CheckedHoveredImage = *FAppStyle::Get().GetBrush("Icons.Rotate180");
	TestStyle.CheckedPressedImage = *FAppStyle::Get().GetBrush("Icons.Rotate180");

	TestStyle.UndeterminedImage = *FAppStyle::Get().GetBrush("Icons.Rotate180");
	TestStyle.UndeterminedHoveredImage = *FAppStyle::Get().GetBrush("Icons.Rotate180");
	TestStyle.UndeterminedPressedImage = *FAppStyle::Get().GetBrush("Icons.Rotate180");

	TestStyle.UncheckedImage = *FAppStyle::Get().GetBrush("Icons.Rotate180");
	TestStyle.UncheckedHoveredImage = *FAppStyle::Get().GetBrush("Icons.Rotate180");
	TestStyle.UncheckedPressedImage = *FAppStyle::Get().GetBrush("Icons.Rotate180");

	TestStyle.CheckedImage.TintColor = FlowYapColor::Green;
	TestStyle.CheckedHoveredImage.TintColor = FlowYapColor::GreenHovered;
	TestStyle.CheckedPressedImage.TintColor = FlowYapColor::GreenPressed;

	TestStyle.UndeterminedImage.TintColor = FlowYapColor::DarkGray;
	TestStyle.UndeterminedHoveredImage.TintColor = FlowYapColor::DarkGrayHovered;
	TestStyle.UndeterminedPressedImage.TintColor = FlowYapColor::DarkGrayPressed;
	
	TestStyle.UncheckedImage.TintColor = FlowYapColor::Red;
	TestStyle.UncheckedHoveredImage.TintColor = FlowYapColor::RedHovered;
	TestStyle.UncheckedPressedImage.TintColor = FlowYapColor::RedPressed;
	
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
	.Padding(2,0,2,0)
	[
		SNew(SCheckBox)
		.Style(&UFlowYapEditorSubsystem::GetCheckBoxStyles().ToggleButtonCheckBox_PlayerPrompt)
		.Padding(FMargin(4, 0))
		.CheckBoxContentUsesAutoWidth(true)
		.ToolTipText(LOCTEXT("DialogueNode_Tooltip", "Toggle Player Prompt Node"))
		.IsChecked(this, &SFlowGraphNode_YapDialogueWidget::PlayerPromptCheckBox_IsChecked)
		.OnCheckStateChanged(this, &SFlowGraphNode_YapDialogueWidget::PlayerPromptCheckBox_OnCheckStateChanged)
		.Content()
		[
			SNew(SBox)
			.WidthOverride(66)
			.HeightOverride(16)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(SImage)
				.ColorAndOpacity(FlowYapColor::DarkGray)
				.DesiredSizeOverride(FVector2D(16, 16))
				.Image(FAppStyle::Get().GetBrush("MainFrame.VisitForums"))
			]
		]
	]
	+ SHorizontalBox::Slot()
	.HAlign(HAlign_Right)
	.AutoWidth()
	.Padding(2,0,-24,0)
	[
		SNew(SBox)
		.WidthOverride(32)
		.HAlign(HAlign_Center)
		[
			SAssignNew(InterruptibleCheckBox, SCheckBox)
			.Style(&TestStyle)
			.Type(ESlateCheckBoxType::ToggleButton)
			.Padding(FMargin(0, 0))
			.CheckBoxContentUsesAutoWidth(true)
			.ToolTipText(LOCTEXT("DialogueNode_Tooltip", "Toggle whether this dialogue can be skipped by the player. Hold CTRL while clicking to reset to project defaults."))
			.IsChecked(this, &SFlowGraphNode_YapDialogueWidget::InterruptibleToggle_IsChecked)
			.OnCheckStateChanged(this, &SFlowGraphNode_YapDialogueWidget::InterruptibleToggle_OnCheckStateChanged)
			.Content()
			[
				SNew(SBox)
				.WidthOverride(16)
				.HeightOverride(16)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				.Visibility_Lambda([this](){return ( GetFlowYapDialogueNode()->Interruptible == EFlowYapInterruptible::NotInterruptible) ? EVisibility::HitTestInvisible : EVisibility::Hidden; })
				[
					SNew(SImage)
					.ColorAndOpacity(FlowYapColor::LightRed)
					.DesiredSizeOverride(FVector2D(16, 16))
					.Image(FAppStyle::Get().GetBrush("SourceControl.StatusIcon.Off"))
				]
			]
		]
	];
}

EVisibility SFlowGraphNode_YapDialogueWidget::BottomAddFragmentButton_Visibility() const
{
	return IsHovered() ? EVisibility::Visible : EVisibility::Hidden;
}

void SFlowGraphNode_YapDialogueWidget::AddPin(const TSharedRef<SGraphPin>& PinToAdd)
{
	if (GetFlowYapDialogueNode()->GetFragments().Num() == 0)
	{
		return;
	}

	TSharedPtr<SFlowGraphEditor> GraphEditor = FFlowGraphUtils::GetFlowGraphEditor(this->FlowGraphNode->GetGraph());

	if (!GraphEditor)
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

	LeftMargins.Top = -1;
	LeftMargins.Right = 0;
	LeftMargins.Bottom = 0;

	RightMargins.Left = 0;
	RightMargins.Bottom = 0;
	RightMargins.Top = 0;
	
	if (OutputPins.Num() == 0)
	{
		RightMargins.Top = -1;
	}
	else if (OutputPins.Num() % 3 == 0)
	{
		RightMargins.Top = 8;
	}

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
								 *GetFlowYapDialogueNode()->GetFragments()[0].Bit.GetDialogueText().ToString());
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

		//PinToAdd->SetToolTipText(FText::Format(LOCTEXT("DialogueNode", "Input {0}"), Index));
		PinToAdd->SetToolTipText(FText::FromName(PinToAdd->GetPinObj()->GetFName()));
		PinToAdd->SetColorAndOpacity((PinToAdd->IsConnected() || InputPins.Num() == 0) ? ConnectedEndPinColor : DisconnectedEndPinColor);

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
			int32 FragmentIndex = FMath::DivideAndRoundDown(OutputPins.Num(), 3);

			if (!FragmentOutputBoxes.IsValidIndex(FragmentIndex))
			{
				UE_LOG(FlowYap, Warning, TEXT("COULD NOT ADD OUTPUT PIN: %s, perhaps node is corrupt?"
								"This might fix itself by refreshing the graph and saving. "
								"Consider disconnecting all pins and cycling Multiple Outputs off and on. "
								"First Node dialogue entry: %s"),
								*PinObj->GetName(),
								*GetFlowYapDialogueNode()->GetFragments()[0].Bit.GetDialogueText().ToString());
				return;
			}
			
			FragmentOutputBoxes[FragmentIndex]->AddSlot()// InsertSlot(0)
			.HAlign(HAlign_Right)
			.VAlign(VAlign_Top)
			.AutoHeight()
			.Padding(RightMargins)
			[
				PinToAdd
			];

			FName PinName = PinToAdd->GetPinObj()->GetFName();

			FLinearColor Color;
			FText ToolTipText;
			if (PinName.IsEqual(FName("DialogueEnd"), ENameCase::IgnoreCase, false))
			{
				Color = PinToAdd->IsConnected() ? ConnectedEndPinColor : DisconnectedEndPinColor;
				ToolTipText = LOCTEXT("Fragment", "On End");
			}
			else if (PinName.IsEqual(FName("DialogueStart"), ENameCase::IgnoreCase, false))
			{
				Color = PinToAdd->IsConnected() ? ConnectedStartPinColor : DisconnectedStartPinColor;
				ToolTipText = LOCTEXT("Fragment", "On Start");
			}
			else // if (PinName.IsEqual(FName("DialogueInterrupt"), ENameCase::IgnoreCase, false))
			{
				Color = PinToAdd->IsConnected() ? ConnectedInterruptPinColor : DisconnectedInterruptPinColor;
				ToolTipText = LOCTEXT("Fragment", "On Interrupt");
			}

			PinToAdd->SetColorAndOpacity(Color);
			PinToAdd->SetToolTipText(ToolTipText);

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

FReply SFlowGraphNode_YapDialogueWidget::BottomAddFragmentButton_OnClicked()
{
	FFlowYapTransactions::BeginModify(LOCTEXT("DialogueAddFragment", "Add Fragment"), GetFlowYapDialogueNode());
	
	GetFlowYapDialogueNode()->AddFragment();

	UpdateGraphNode();

	FFlowYapTransactions::EndModify();
	
	return FReply::Handled();
}

void SFlowGraphNode_YapDialogueWidget::DeleteFragment(uint8 FragmentIndex)
{
	FFlowYapTransactions::BeginModify(LOCTEXT("DialogueDeleteFragment", "Delete Fragment"), GetFlowYapDialogueNode());

	FlowGraphNode_YapDialogue->UpdatePinsForFragmentDeletion(FragmentIndex);

	GetFlowYapDialogueNode()->DeleteFragmentByIndex(FragmentIndex);

	UpdateGraphNode();

	FFlowYapTransactions::EndModify();
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

	if (!GraphEditor)
	{
		return;
	}
	
	bIsSelected = GraphEditor->GetSelectedFlowNodes().Contains(FlowGraphNode);

	bool bShiftPressed = GEditor->GetEditorSubsystem<UFlowYapEditorSubsystem>()->GetInputTracker()->GetShiftPressed();

	if (bIsSelected)
	{
		bWasSelected = true;
	}
	
	if (bIsSelected && bShiftPressed && !KeyboardFocusedFragmentWidget.IsSet())
	{
		bShiftHooked = true;
	}

	if (!bIsSelected)
	{
		bWasSelected = false;
		bShiftHooked = false;
		SelectedFragmentWidget.Reset();
		KeyboardFocusedFragmentWidget.Reset();
	}

	for (int i = 2; i < OutputPins.Num() - 1; i += 3)
	{
		TSharedRef<SGraphPin> Pin = OutputPins[i];

		FLinearColor InterruptPinColor;

		if (GetFlowYapDialogueNode()->GetInterruptible())
		{
			InterruptPinColor = Pin->IsConnected() ? ConnectedInterruptPinColor	: DisconnectedInterruptPinColor;
		}
		else
		{
			InterruptPinColor = Pin->IsConnected() ? ConnectedInterruptPinColor_Disabled : DisconnectedInterruptPinColor_Disabled;
		}
		
		Pin->SetColorAndOpacity(InterruptPinColor);
	}
}

bool SFlowGraphNode_YapDialogueWidget::GetIsSelected() const
{
	return bIsSelected;
}

bool SFlowGraphNode_YapDialogueWidget::GetControlHooked() const
{
	return bShiftHooked;
}

void SFlowGraphNode_YapDialogueWidget::SetFocusedFragment(uint8 InFragment)
{
	if (SelectedFragmentWidget != InFragment)
	{
		TSharedPtr<SFlowGraphEditor> GraphEditor = FFlowGraphUtils::GetFlowGraphEditor(this->FlowGraphNode->GetGraph());
		GraphEditor->SetNodeSelection(FlowGraphNode, true);
		
		SelectedFragmentWidget = InFragment;
	}
}

void SFlowGraphNode_YapDialogueWidget::ClearFocusedFragment(uint8 FragmentIndex)
{
	if (SelectedFragmentWidget == FragmentIndex)
	{
		SelectedFragmentWidget.Reset();
	}
}

const TSharedPtr<SFlowGraphNode_YapFragmentWidget> SFlowGraphNode_YapDialogueWidget::GetFocusedFragment() const
{
	if (SelectedFragmentWidget.IsSet())
	{
		return FragmentWidgets[SelectedFragmentWidget.GetValue()];
	}
	else
	{
		return nullptr;
	}
}

void SFlowGraphNode_YapDialogueWidget::SetTypingFragment(uint8 FragmentIndex)
{
	KeyboardFocusedFragmentWidget = FragmentIndex;
}

void SFlowGraphNode_YapDialogueWidget::ClearTypingFragment(uint8 FragmentIndex)
{
	if (KeyboardFocusedFragmentWidget == FragmentIndex)
	{
		KeyboardFocusedFragmentWidget.Reset();
	}
}

TSharedPtr<SFlowGraphNode_YapFragmentWidget> SFlowGraphNode_YapDialogueWidget::GetKeyboardFocusedFragmentWidget() const
{
	if (KeyboardFocusedFragmentWidget.IsSet())
	{
		return FragmentWidgets[KeyboardFocusedFragmentWidget.GetValue()];
	}
	else
	{
		return nullptr;
	}
}

#undef LOCTEXT_NAMESPACE
