// Copyright Ghost Pepper Games, Inc. All Rights Reserved.

#include "Yap/NodeWidgets/SFlowGraphNode_YapDialogueWidget.h"

#include "FlowEditorStyle.h"
#include "Graph/FlowGraphEditor.h"
#include "Graph/FlowGraphSettings.h"
#include "Graph/FlowGraphUtils.h"
#include "Logging/StructuredLog.h"
#include "Widgets/Input/SSlider.h"
#include "Widgets/Input/SSpinBox.h"
#include "Widgets/Input/SVolumeControl.h"
#include "Widgets/Notifications/SProgressBar.h"
#include "Yap/FlowYapBit.h"
#include "Yap/FlowYapColors.h"
#include "Yap/FlowYapEditorSettings.h"
#include "Yap/FlowYapEditorSubsystem.h"
#include "Yap/FlowYapFragment.h"
#include "Yap/FlowYapInputTracker.h"
#include "Yap/FlowYapLog.h"
#include "Yap/FlowYapProjectSettings.h"
#include "Yap/FlowYapTransactions.h"
#include "Yap/YapEditorStyle.h"
#include "Yap/GraphNodes/FlowGraphNode_YapDialogue.h"
#include "Yap/Nodes/FlowNode_YapDialogue.h"
#include "Yap/NodeWidgets/SFlowYapGraphPinExec.h"

#define LOCTEXT_NAMESPACE "FlowYap"

constexpr int32 NUM_PINS_PER_FRAGMENT {2};

// TODO move to a proper style
FButtonStyle SFlowGraphNode_YapDialogueWidget::MoveFragmentButtonStyle;
bool SFlowGraphNode_YapDialogueWidget::bStylesInitialized = false;

// ------------------------------------------
// CONSTRUCTION
void SFlowGraphNode_YapDialogueWidget::Construct(const FArguments& InArgs, UFlowGraphNode* InNode)
{	
	GraphNode = InNode;
	FlowGraphNode = InNode;
	FlowGraphNode_YapDialogue = Cast<UFlowGraphNode_YapDialogue>(InNode);

	DialogueButtonsColor = YapColor::DimGray;

	ConnectedEndPinColor = YapColor::White;
	DisconnectedEndPinColor = YapColor::DarkGray;
	DisconnectedEndPinColor_Prompt = YapColor::Red;
	
	ConnectedStartPinColor = YapColor::LightGreen;
	DisconnectedStartPinColor = YapColor::DarkGray;
	
	ConnectedInterruptPinColor = YapColor::LightRed;
	DisconnectedInterruptPinColor = YapColor::DarkGray;
	ConnectedInterruptPinColor_Disabled = YapColor::DarkGray;
	DisconnectedInterruptPinColor_Disabled = YapColor::DarkGray;
	
	ConnectedBypassPinColor = YapColor::LightBlue;
	DisconnectedBypassPinColor = YapColor::DarkGray;

	ConnectedConditionPinColor = YapColor::Orange;
	DisconnectedConditionPinColor = YapColor::DarkGray;
	
	FocusedFragmentIndex.Reset();
	
	if (!bStylesInitialized)
	{
		MoveFragmentButtonStyle = FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("PropertyEditor.AssetComboStyle");

		// Button colors
		MoveFragmentButtonStyle.Normal.TintColor = YapColor::Noir_Trans;
		MoveFragmentButtonStyle.Hovered.TintColor = YapColor::DarkGray_Trans;
		MoveFragmentButtonStyle.Pressed.TintColor = YapColor::DarkGrayPressed_Trans;

		// Text colors
		MoveFragmentButtonStyle.NormalForeground = YapColor::LightGray;
		MoveFragmentButtonStyle.HoveredForeground = YapColor::White;
		MoveFragmentButtonStyle.PressedForeground = YapColor::LightGrayPressed;

		bStylesInitialized = true;
	}
	
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

// ------------------------------------------
// WIDGETS

// ================================================================================================
// TITLE WIDGET
// ------------------------------------------------------------------------------------------------

TSharedRef<SWidget> SFlowGraphNode_YapDialogueWidget::CreateTitleWidget(TSharedPtr<SNodeTitle> NodeTitle)
{
	TSharedRef<SWidget> Title = SFlowGraphNode::CreateTitleWidget(NodeTitle);

	TSharedPtr<SCheckBox> InterruptibleCheckBox;

	// TODO move to a proper style
	InterruptibleCheckBoxStyle = FAppStyle::Get().GetWidgetStyle<FCheckBoxStyle>("ToggleButtonCheckBox");

	InterruptibleCheckBoxStyle.SetCheckBoxType(ESlateCheckBoxType::ToggleButton);
	InterruptibleCheckBoxStyle.CheckedImage = InterruptibleCheckBoxStyle.UncheckedImage;
	InterruptibleCheckBoxStyle.CheckedHoveredImage = InterruptibleCheckBoxStyle.UncheckedHoveredImage;
	InterruptibleCheckBoxStyle.CheckedPressedImage = InterruptibleCheckBoxStyle.UncheckedPressedImage;

	InterruptibleCheckBoxStyle.UndeterminedImage = InterruptibleCheckBoxStyle.UncheckedImage;
	InterruptibleCheckBoxStyle.UndeterminedHoveredImage = InterruptibleCheckBoxStyle.UncheckedHoveredImage;
	InterruptibleCheckBoxStyle.UndeterminedPressedImage = InterruptibleCheckBoxStyle.UncheckedPressedImage;
	
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
	.Padding(2, -2, 2, -2)
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
			.WidthOverride(50)
			.HeightOverride(20)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(SImage)
				.ColorAndOpacity(YapColor::DarkGray)
				.DesiredSizeOverride(FVector2D(16, 16))
				.Image(FAppStyle::Get().GetBrush("MainFrame.VisitForums"))
			]
		]
	]
	+ SHorizontalBox::Slot()
	.HAlign(HAlign_Right)
	.AutoWidth()
	.Padding(9,-2,-27,-2)
	[
		SNew(SBox)
		.WidthOverride(20)
		.HAlign(HAlign_Center)
		[
			SAssignNew(InterruptibleCheckBox, SCheckBox)
			.Style(&InterruptibleCheckBoxStyle)
			.Type(ESlateCheckBoxType::ToggleButton)
			.Padding(FMargin(0, 0))
			.CheckBoxContentUsesAutoWidth(true)
			.ToolTipText(LOCTEXT("DialogueNode_Tooltip", "Toggle whether this dialogue can be skipped by the player. Hold CTRL while clicking to reset to project defaults."))
			.IsChecked(this, &SFlowGraphNode_YapDialogueWidget::InterruptibleToggle_IsChecked)
			.OnCheckStateChanged(this, &SFlowGraphNode_YapDialogueWidget::InterruptibleToggle_OnCheckStateChanged)
			.Content()
			[
				SNew(SBox)
				.WidthOverride(20)
				.HeightOverride(20)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					SNew(SOverlay)
					+ SOverlay::Slot()
					[
						SNew(SImage)
						.ColorAndOpacity(this, &SFlowGraphNode_YapDialogueWidget::InterruptibleToggleIcon_ColorAndOpacity)
						.DesiredSizeOverride(FVector2D(16, 16))
						.Image(FAppStyle::Get().GetBrush("Icons.Rotate180"))
					]
					+ SOverlay::Slot()
					[
						SNew(SImage)
						.ColorAndOpacity(YapColor::LightRed)
						.DesiredSizeOverride(FVector2D(16, 16))
						.Image(FAppStyle::Get().GetBrush("SourceControl.StatusIcon.Off"))
						.Visibility_Lambda([this](){return ( GetFlowYapDialogueNodeMutable()->Interruptible == EFlowYapInterruptible::NotInterruptible) ? EVisibility::HitTestInvisible : EVisibility::Collapsed; })
					]
				]
			]
		]
	];
}

ECheckBoxState SFlowGraphNode_YapDialogueWidget::PlayerPromptCheckBox_IsChecked() const
{
	return GetFlowYapDialogueNode()->GetIsPlayerPrompt() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SFlowGraphNode_YapDialogueWidget::PlayerPromptCheckBox_OnCheckStateChanged(ECheckBoxState CheckBoxState)
{
	FFlowYapTransactions::BeginModify(LOCTEXT("Dialogue", "Toggle player prompt node"), GetFlowYapDialogueNodeMutable());

	GetFlowYapDialogueNodeMutable()->SetIsPlayerPrompt(CheckBoxState == ECheckBoxState::Checked ? true : false);

	FFlowYapTransactions::EndModify();
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
		GetFlowYapDialogueNodeMutable()->Interruptible = EFlowYapInterruptible::UseProjectDefaults;
	}
	else if (CheckBoxState == ECheckBoxState::Checked)
	{
		GetFlowYapDialogueNodeMutable()->Interruptible = EFlowYapInterruptible::Interruptible;
	}
	else
	{
		GetFlowYapDialogueNodeMutable()->Interruptible = EFlowYapInterruptible::NotInterruptible;
	}
}

FSlateColor SFlowGraphNode_YapDialogueWidget::InterruptibleToggleIcon_ColorAndOpacity() const
{
	if (GetFlowYapDialogueNode()->Interruptible == EFlowYapInterruptible::NotInterruptible)
	{
		return YapColor::Red;
	}
	else if (GetFlowYapDialogueNode()->Interruptible == EFlowYapInterruptible::Interruptible)
	{
		return YapColor::Green;
	}
	else
	{
		return YapColor::DarkGray;
	}
}

// ================================================================================================
// NODE CONTENT WIDGET
// ------------------------------------------------------------------------------------------------

TSharedRef<SWidget> SFlowGraphNode_YapDialogueWidget::CreateNodeContentArea()
{
	SAssignNew(FragmentBox, SVerticalBox);

	bool bSingleFragment = GetFlowYapDialogueNodeMutable()->GetNumFragments() == 1;
	bool bFirstFragment = true;
	bool bLastFragment = false;

	uint8 Spacing = GetDefault<UFlowYapEditorSettings>()->GetDialogueRowSpacing(); // TODO put this into project settings
	uint8 Padding = 1 + Spacing * 8;
	
	for (uint8 FragmentIndex = 0; FragmentIndex < GetFlowYapDialogueNodeMutable()->GetNumFragments(); ++FragmentIndex)
	{
		FFlowYapFragment& Fragment = GetFlowYapDialogueNodeMutable()->GetFragmentsMutable()[FragmentIndex];

		if (FragmentIndex == GetFlowYapDialogueNodeMutable()->GetNumFragments() - 1)
		{
			bLastFragment = true;
		}
		
		TSharedPtr<SFlowGraphNode_YapFragmentWidget> NewFragmentWidget = MakeShared<SFlowGraphNode_YapFragmentWidget>();

		FragmentWidgets.Add(NewFragmentWidget);

		FragmentBox->AddSlot()
		.AutoHeight()
		.Padding(0, 0, 0, 0)
		[
			CreateFragmentSeparatorWidget(FragmentIndex)
		];	
		
		FragmentBox->AddSlot()
		.AutoHeight()
		.Padding(0, bFirstFragment ? 1 : Padding, 0, bSingleFragment || bLastFragment ? 2 : Padding)
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			[
				CreateFragmentRowWidget(FragmentIndex)
			]
			+ SOverlay::Slot()
			.Padding(26, -4)
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				SNew(SBorder)
				//.BorderImage(FAppStyle::GetBrush("Menu.Background")) // Filled, Square, bit dark
				.BorderImage(FAppStyle::GetBrush("Graph.StateNode.Body")) // Filled, rounded nicely
				//.BorderImage(FAppStyle::GetBrush("Brushes.Panel")) // Filled, Square, Dark
				.Visibility(this, &SFlowGraphNode_YapDialogueWidget::FragmentRowHighlight_Visibility, FragmentIndex)
				.BorderBackgroundColor(this, &SFlowGraphNode_YapDialogueWidget::FragmentRowHighlight_BorderBackgroundColor, FragmentIndex)
			]
		];
		
		bFirstFragment = false;
	};
	
	FragmentBox->AddSlot()
	.AutoHeight()
	.Padding(0,0)
	[
		CreateBottomBarWidget()
	];

	return SNew(SBorder)
	.BorderImage(FAppStyle::GetBrush("NoBorder"))
	.HAlign(HAlign_Fill)
	.VAlign(VAlign_Fill)
	[
		FragmentBox.ToSharedRef()
	];
}

EVisibility SFlowGraphNode_YapDialogueWidget::FragmentRowHighlight_Visibility(uint8 f) const
{
	if (FlashFragmentIndex == f || GetFlowYapDialogueNode()->GetRunningFragmentIndex() == f)
	{
		return EVisibility::HitTestInvisible;
	}

	return EVisibility::Collapsed;
}

FSlateColor SFlowGraphNode_YapDialogueWidget::FragmentRowHighlight_BorderBackgroundColor(uint8 f) const
{
	if (GetFlowYapDialogueNode()->GetRunningFragmentIndex() == f)
	{
		return YapColor::White_Glass;
	}
	
	if (FlashFragmentIndex == f)
	{
		return FlashHighlight * YapColor::White_Trans;
	}

	return YapColor::Transparent;
}


TSharedRef<SWidget> SFlowGraphNode_YapDialogueWidget::CreateFragmentSeparatorWidget(uint8 FragmentIndex)
{
	TSharedRef<SHorizontalBox> Box = SNew(SHorizontalBox);
	
	Box->AddSlot()
	.HAlign(HAlign_Fill)
	.Padding(0, -2, 0, -2)
	[
		SNew(SButton)
		.ButtonStyle(FAppStyle::Get(), "SimpleButton")
		.ToolTipText(LOCTEXT("DialogueNode", "Insert new fragment"))
		.ContentPadding(FMargin(0, 2, 0, 2))
		.Visibility(this, &SFlowGraphNode_YapDialogueWidget::FragmentSeparator_Visibility)
		.OnClicked(this, &SFlowGraphNode_YapDialogueWidget::FragmentSeparator_OnClicked, FragmentIndex)			
		[
			SNew(SImage)
			.Image(FAppStyle::GetBrush("Menu.Separator"))
			.DesiredSizeOverride(FVector2D(1, 1))
			.ColorAndOpacity(this, &SFlowGraphNode_YapDialogueWidget::FragmentSeparator_ColorAndOpacity)
		]
	];

	return Box;
}


EVisibility SFlowGraphNode_YapDialogueWidget::FragmentSeparator_Visibility() const
{
	return GetIsSelected() ? EVisibility::Visible : EVisibility::Hidden;
}

FSlateColor SFlowGraphNode_YapDialogueWidget::FragmentSeparator_ColorAndOpacity() const
{
	return YapColor::Gray; 
}

FReply SFlowGraphNode_YapDialogueWidget::FragmentSeparator_OnClicked(uint8 Index)
{
	FFlowYapTransactions::BeginModify(LOCTEXT("DialogueAddFragment", "Add Fragment"), GetFlowYapDialogueNodeMutable());

	GetFlowYapDialogueNodeMutable()->InsertFragment(Index);

	FlowGraphNode_YapDialogue->UpdatePinsAfterFragmentInsertion(Index);

	UpdateGraphNode();

	FFlowYapTransactions::EndModify();
		
	return FReply::Handled();
}

// ================================================================================================
// FRAGMENT ROW
// ------------------------------------------------------------------------------------------------

TSharedRef<SWidget> SFlowGraphNode_YapDialogueWidget::CreateFragmentRowWidget(uint8 FragmentIndex)
{
	return SNew(SHorizontalBox)
	// LEFT PANE
	+ SHorizontalBox::Slot()
	.AutoWidth()
	.Padding(0, 0, 0, 0)
	.VAlign(VAlign_Fill)
	[
		CreateLeftFragmentPane(FragmentIndex)
	]
	// MIDDLE PANE
	+ SHorizontalBox::Slot()
	.AutoWidth()
	.VAlign(VAlign_Top)
	[
		SAssignNew(FragmentWidgets[FragmentWidgets.Num() -1], SFlowGraphNode_YapFragmentWidget, this, FragmentIndex)
	]
	// RIGHT PANE
	+ SHorizontalBox::Slot()
	.AutoWidth()
	.VAlign(VAlign_Fill)
	[
		CreateRightFragmentPane()
	];
}

// ================================================================================================
// LEFT SIDE PANE
// ------------------------------------------------------------------------------------------------

TSharedRef<SBox> SFlowGraphNode_YapDialogueWidget::CreateLeftFragmentPane(uint8 FragmentIndex)
{
	return SNew(SBox)
	//.WidthOverride(40)
	[
		SNew(SOverlay)
		+ SOverlay::Slot()
		.Padding(-72, 0, 0, 0)
		[
			CreateFragmentControlsWidget(FragmentIndex)
		]
		+ SOverlay::Slot()
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.VAlign(VAlign_Fill)
			//.AutoHeight()
			.HAlign(HAlign_Center)
			[
				CreateLeftSideNodeBox()
			]
			/*
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			[
				CreateActivationLimiterWidget(FragmentIndex)
			]
			*/
		]
	];
}

// ================================================================================================
// FRAGMENT CONTROLS WIDGET
// ------------------------------------------------------------------------------------------------

TSharedRef<SBox> SFlowGraphNode_YapDialogueWidget::CreateFragmentControlsWidget(uint8 FragmentIndex)
{
	return SNew(SBox)
	[
		SNew(SVerticalBox)
		.Visibility(this, &SFlowGraphNode_YapDialogueWidget::FragmentControls_Visibility, FragmentIndex)
		// UP
		+ SVerticalBox::Slot()
		.AutoHeight()
		.VAlign(VAlign_Top)
		.HAlign(HAlign_Center)
		.Padding(0, 2, 0, 1)
		[
			SNew(SButton)
			.ButtonStyle(&MoveFragmentButtonStyle)
			.ContentPadding(FMargin(4, 4))
			.ToolTipText(LOCTEXT("DialogueMoveFragmentUp_Tooltip", "Move Fragment Up"))
			.Visibility(this, &SFlowGraphNode_YapDialogueWidget::MoveFragmentUpButton_Visibility, FragmentIndex)
			.OnClicked(this, &SFlowGraphNode_YapDialogueWidget::MoveFragmentUpButton_OnClicked, FragmentIndex)
			[
				SNew(SImage)
				.Image(FAppStyle::Get().GetBrush("Symbols.UpArrow"))
				.DesiredSizeOverride(FVector2D(8, 8))
				.ColorAndOpacity(FSlateColor::UseForeground())
			]
		]
		// DELETE
		+ SVerticalBox::Slot()
		.AutoHeight()
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Center)
		.Padding(0, 0)
		[
			SNew(SButton)
			.ButtonStyle(&MoveFragmentButtonStyle)
			.ContentPadding(FMargin(4, 4))
			.ToolTipText(LOCTEXT("DialogueDeleteFragment_Tooltip", "Delete Fragment"))
			.Visibility(this, &SFlowGraphNode_YapDialogueWidget::DeleteFragmentButton_Visibility, FragmentIndex)
			.OnClicked(this, &SFlowGraphNode_YapDialogueWidget::DeleteFragmentButton_OnClicked, FragmentIndex)
			[
				SNew(SImage)
				.Image(FAppStyle::GetBrush("Cross"))
				.DesiredSizeOverride(FVector2D(8, 8))
				.ColorAndOpacity(FSlateColor::UseForeground())
			]
		]
		// DOWN
		+ SVerticalBox::Slot()
		.AutoHeight()
		.VAlign(VAlign_Bottom)
		.HAlign(HAlign_Center)
		.Padding(0, 1, 0, 2)
		[
			SNew(SButton)
			.ButtonStyle(&MoveFragmentButtonStyle)
			.ContentPadding(FMargin(4, 4))
			.ToolTipText(LOCTEXT("DialogueMoveFragmentDown_Tooltip", "Move Fragment Down"))
			.Visibility(this, &SFlowGraphNode_YapDialogueWidget::MoveFragmentDownButton_Visibility, FragmentIndex)
			.OnClicked(this, &SFlowGraphNode_YapDialogueWidget::MoveFragmentDownButton_OnClicked, FragmentIndex)
			[
				SNew(SImage)
				.Image(FAppStyle::Get().GetBrush("Symbols.DownArrow"))
				.DesiredSizeOverride(FVector2D(8, 8))
				.ColorAndOpacity(FSlateColor::UseForeground())
			]
		]
	];
}

EVisibility SFlowGraphNode_YapDialogueWidget::FragmentControls_Visibility(uint8 FragmentIndex) const
{
	//return (IsHovered()) ? EVisibility::Visible : EVisibility::Collapsed;
	return GetIsSelected() ? EVisibility::Visible : EVisibility::Collapsed;
}

EVisibility SFlowGraphNode_YapDialogueWidget::MoveFragmentUpButton_Visibility(uint8 FragmentIndex) const
{
	return (FragmentIndex == 0) ? EVisibility::Hidden : EVisibility::Visible;		
}

FReply SFlowGraphNode_YapDialogueWidget::MoveFragmentUpButton_OnClicked(uint8 FragmentIndex)
{
	FFlowYapTransactions::BeginModify(LOCTEXT("DialogueNode", "Move Fragment"), GetFlowYapDialogueNodeMutable());

	MoveFragmentUp(FragmentIndex);
	
	FFlowYapTransactions::EndModify();

	return FReply::Handled();
}

EVisibility SFlowGraphNode_YapDialogueWidget::DeleteFragmentButton_Visibility(uint8 FragmentIndex) const
{
	if (GetFlowYapDialogueNode()->GetNumFragments() <= 1)
	{
		return EVisibility::Collapsed;
	}
	
	return EVisibility::Visible;
}

FReply SFlowGraphNode_YapDialogueWidget::DeleteFragmentButton_OnClicked(uint8 FragmentIndex)
{
	FFlowYapTransactions::BeginModify(LOCTEXT("DialogueDeleteFragment", "Delete Fragment"), GetFlowYapDialogueNodeMutable() /*GetFlowYapDialogueNode()*/);

	DeleteFragment(FragmentIndex);

	FFlowYapTransactions::EndModify();

	return FReply::Handled();
}

EVisibility SFlowGraphNode_YapDialogueWidget::MoveFragmentDownButton_Visibility(uint8 FragmentIndex) const
{
	return (FragmentIndex == GetFlowYapDialogueNode()->GetNumFragments() - 1) ? EVisibility::Hidden : EVisibility::Visible;
}

FReply SFlowGraphNode_YapDialogueWidget::MoveFragmentDownButton_OnClicked(uint8 FragmentIndex)
{
	FFlowYapTransactions::BeginModify(LOCTEXT("DialogueNode", "Move Fragment"), GetFlowYapDialogueNodeMutable());

	MoveFragmentDown(FragmentIndex);
	
	FFlowYapTransactions::EndModify();

	return FReply::Handled();
}

// ================================================================================================
// INPUT NODE BOX (UPPER HALF OF LEFT SIDE PANE)
// ------------------------------------------------------------------------------------------------

TSharedRef<SBox> SFlowGraphNode_YapDialogueWidget::CreateLeftSideNodeBox()
{
	TSharedRef<SVerticalBox> LeftSideNodeBox = SNew(SVerticalBox);

	FragmentInputBoxes.Add(LeftSideNodeBox);

	return SNew(SBox)
	.MinDesiredHeight(16)
	[
		LeftSideNodeBox
	];
}

// ================================================================================================
// RIGHT PANE OF FRAGMENT ROW
// ------------------------------------------------------------------------------------------------

TSharedRef<SBox> SFlowGraphNode_YapDialogueWidget::CreateRightFragmentPane()
{
	TSharedRef<SVerticalBox> RightSideNodeBox = SNew(SVerticalBox)
	+ SVerticalBox::Slot()
	.HAlign(HAlign_Center)
	.VAlign(VAlign_Fill)
	[
		SNew(SSpacer)
	];
	
	FragmentOutputBoxes.Add(RightSideNodeBox);
	
	return SNew(SBox)
	.MinDesiredWidth(32) // Unreal's input nodes are larger than its output nodes. The left side will be 32 px wide. The right side will be about 27 px wide without this. Thanks Epic.
	.VAlign(VAlign_Fill)
	[
		RightSideNodeBox
	];
}

// ================================================================================================
// BOTTOM BAR
// ------------------------------------------------------------------------------------------------

TSharedRef<SHorizontalBox> SFlowGraphNode_YapDialogueWidget::CreateBottomBarWidget()
{
	return SNew(SHorizontalBox)
	+ SHorizontalBox::Slot()
	.AutoWidth()
	.HAlign(HAlign_Left)
	.Padding(0, 0, 6, 0)
	[
		SNew(SBox)
		//.WidthOverride(40)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		.Padding(0, 0)
		[
			SNew(SButton)
			.ButtonStyle(FAppStyle::Get(), "SimpleButton")
			.ContentPadding(FMargin(0, 1, 0, 1))
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
	.Padding(0, -2, 0, -2)
	[
		SAssignNew(BypassOutputBox, SBox)
		.HAlign(HAlign_Center)
		//.WidthOverride(40)
		.Padding(0)
	];
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
	FFlowYapTransactions::BeginModify(LOCTEXT("DialogueNodeChangeSequencing", "Change dialogue node sequencing setting"), GetFlowYapDialogueNodeMutable());

	GetFlowYapDialogueNodeMutable()->CycleFragmentSequencingMode();
	
	return FReply::Handled();
}

const FSlateBrush* SFlowGraphNode_YapDialogueWidget::FragmentSequencingButton_Image() const
{
	switch (GetFlowYapDialogueNode()->GetMultipleFragmentSequencing())
	{
	case EFlowYapMultipleFragmentSequencing::Sequential:
		{
			return FAppStyle::Get().GetBrush("Icons.SortDown"); 
		}
	case EFlowYapMultipleFragmentSequencing::SelectOne:
		{
			return FAppStyle::Get().GetBrush("LevelEditor.Profile"); 
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
	case EFlowYapMultipleFragmentSequencing::SelectOne:
		{
			return LOCTEXT("DialogueNodeSequence", "Starting from the top, attempts to run a single fragment only. Use this to easily display slightly different dialogue on second, third runs.");
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
			return YapColor::DarkGray;
		}
	case EFlowYapMultipleFragmentSequencing::SelectOne:
		{
			return YapColor::Orange;
		}
	default:
		{
			return YapColor::White;
		}
	}
}

EVisibility SFlowGraphNode_YapDialogueWidget::BottomAddFragmentButton_Visibility() const
{
	return IsHovered() ? EVisibility::Visible : EVisibility::Hidden;
}


FReply SFlowGraphNode_YapDialogueWidget::BottomAddFragmentButton_OnClicked()
{
	FFlowYapTransactions::BeginModify(LOCTEXT("DialogueAddFragment", "Add Fragment"), GetFlowYapDialogueNodeMutable());
	
	GetFlowYapDialogueNodeMutable()->AddFragment();

	UpdateGraphNode();

	FFlowYapTransactions::EndModify();
	
	return FReply::Handled();
}

// ------------------------------------------
// PUBLIC API & THEIR HELPERS

void SFlowGraphNode_YapDialogueWidget::DeleteFragment(uint8 FragmentIndex)
{
	FFlowYapTransactions::BeginModify(LOCTEXT("DialogueDeleteFragment", "Delete Fragment"), GetFlowYapDialogueNodeMutable());

	FlowGraphNode_YapDialogue->UpdatePinsForFragmentDeletion(FragmentIndex);

	GetFlowYapDialogueNodeMutable()->DeleteFragmentByIndex(FragmentIndex);

	UpdateGraphNode();

	FFlowYapTransactions::EndModify();
}

void SFlowGraphNode_YapDialogueWidget::MoveFragmentUp(uint8 FragmentIndex)
{
	check(FragmentIndex > 0);
	MoveFragment(FragmentIndex, -1);

	SetFlashFragment(FragmentIndex - 1);
}

void SFlowGraphNode_YapDialogueWidget::MoveFragmentDown(uint8 FragmentIndex)
{
	check(FragmentIndex < GetFlowYapDialogueNodeMutable()->GetNumFragments() - 1);
	MoveFragment(FragmentIndex, +1);

	SetFlashFragment(FragmentIndex + 1);
}

void SFlowGraphNode_YapDialogueWidget::MoveFragment(uint8 FragmentIndex, int16 By)
{
	FlowGraphNode_YapDialogue->SwapFragmentPinConnections(FragmentIndex, FragmentIndex + By);

	GetFlowYapDialogueNodeMutable()->SwapFragments(FragmentIndex, FragmentIndex + By);

	if (FocusedFragmentIndex.IsSet())
	{
		FocusedFragmentIndex = FocusedFragmentIndex.GetValue() + By;
	}
}

void SFlowGraphNode_YapDialogueWidget::SetFocusedFragmentIndex(uint8 InFragment)
{
	if (FocusedFragmentIndex != InFragment)
	{
		TSharedPtr<SFlowGraphEditor> GraphEditor = FFlowGraphUtils::GetFlowGraphEditor(this->FlowGraphNode->GetGraph());
		GraphEditor->SetNodeSelection(FlowGraphNode, true);
		
		FocusedFragmentIndex = InFragment;
	}

	SetTypingFocus();
}

void SFlowGraphNode_YapDialogueWidget::ClearFocusedFragmentIndex(uint8 FragmentIndex)
{
	if (FocusedFragmentIndex == FragmentIndex)
	{
		FocusedFragmentIndex.Reset();
	}
}

const bool SFlowGraphNode_YapDialogueWidget::GetFocusedFragmentIndex(uint8& OutFragmentIndex) const
{
	if (FocusedFragmentIndex.IsSet())
	{
		OutFragmentIndex = FocusedFragmentIndex.GetValue();
		return true;
	}

	return false;
}

void SFlowGraphNode_YapDialogueWidget::SetTypingFocus()
{
	bKeyboardFocused = true;
}

void SFlowGraphNode_YapDialogueWidget::ClearTypingFocus()
{
	bKeyboardFocused = false;
}

UFlowNode_YapDialogue* SFlowGraphNode_YapDialogueWidget::GetFlowYapDialogueNodeMutable()
{
	return Cast<UFlowNode_YapDialogue>(FlowGraphNode->GetFlowNode());
}

const UFlowNode_YapDialogue* SFlowGraphNode_YapDialogueWidget::GetFlowYapDialogueNode() const
{
	return Cast<UFlowNode_YapDialogue>(FlowGraphNode->GetFlowNode());
}

void SFlowGraphNode_YapDialogueWidget::SetFlashFragment(uint8 FragmentIndex)
{
	FlashFragmentIndex = FragmentIndex;
	FlashHighlight = 1.0;
}

void SFlowGraphNode_YapDialogueWidget::OnDialogueEnd(uint8 FragmentIndex)
{
}

void SFlowGraphNode_YapDialogueWidget::OnDialogueStart(uint8 FragmentIndex)
{
	SetFlashFragment(FragmentIndex);
}

void SFlowGraphNode_YapDialogueWidget::OnDialogueInterrupt(uint8 FragmentIndex)
{
	SetFlashFragment(FragmentIndex);
}

// ------------------------------------------
// OVERRIDES & THEIR HELPERS

void SFlowGraphNode_YapDialogueWidget::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SFlowGraphNode::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);
	
	TSharedPtr<SFlowGraphEditor> GraphEditor = FFlowGraphUtils::GetFlowGraphEditor(this->FlowGraphNode->GetGraph());

	if (!GraphEditor)
	{
		return;
	}
	
	bIsSelected = GraphEditor->GetSelectedFlowNodes().Contains(FlowGraphNode);

	bShiftPressed = GEditor->GetEditorSubsystem<UFlowYapEditorSubsystem>()->GetInputTracker()->GetShiftPressed();
	bCtrlPressed = GEditor->GetEditorSubsystem<UFlowYapEditorSubsystem>()->GetInputTracker()->GetControlPressed();
	
	if (bIsSelected && bShiftPressed && !bKeyboardFocused)
	{
		bShiftHooked = true;
	}

	if (!bIsSelected)
	{
		bShiftHooked = false;
		FocusedFragmentIndex.Reset();
		bKeyboardFocused = false;
	}

	FlashHighlight = FMath::Max(FlashHighlight, FlashHighlight -= 2.0 * InDeltaTime);

	if (FlashHighlight <= 0)
	{
		FlashFragmentIndex.Reset();
	}
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
	
	if (bSelected)
	{
		return FAppStyle::GetBrush(TEXT("Graph.Node.ShadowSelected"));
	}
	
	if (GetFlowYapDialogueNode()->GetIsPlayerPrompt())
	{
		// TODO a custom brush style could be less obtuse
		return FAppStyle::GetBrush(TEXT("Graph.ReadOnlyBorder"));
	}

	return FAppStyle::GetBrush(TEXT("Graph.Node.Shadow"));
}

void SFlowGraphNode_YapDialogueWidget::AddPin(const TSharedRef<SGraphPin>& PinToAdd)
{
	if (GetFlowYapDialogueNodeMutable()->GetFragments().Num() == 0)
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
	
	FName PinName = PinToAdd->GetPinObj()->GetFName();
	
	if (PinToAdd->GetDirection() == EEdGraphPinDirection::EGPD_Input)
	{
		if (PinName.IsEqual(FName("In"), ENameCase::IgnoreCase, false))
		{
			AddInPin(PinToAdd);
		}
		else if (PinName.IsEqual("Condition", ENameCase::IgnoreCase, false))
		{
			AddConditionPin(PinToAdd);
		}
		else
		{
			UE_LOGFMT(FlowYap, Warning, "Invalid input pin name: {0}", PinName);
			return;
		}
		
		InputPins.Add(PinToAdd);
	}
	else
	{
		if (PinName == FName("Out"))
		{
			AddOutPin(PinToAdd);
		}
		else if (PinName == FName("Bypass"))
		{
			AddBypassPin(PinToAdd);
		}
		else if (PinName.IsEqual("FragmentEnd", ENameCase::IgnoreCase, false))
		{
			AddFragmentEndPin(PinToAdd);
		}
		else if (PinName.IsEqual("FragmentStart", ENameCase::IgnoreCase, false))
		{
			AddFragmentStartPin(PinToAdd);
		}
		else
		{
			UE_LOGFMT(FlowYap, Warning, "Invalid output pin name: {0}", PinName);
			return;
		}

		OutputPins.Add(PinToAdd);
	}
}


void SFlowGraphNode_YapDialogueWidget::AddInPin(const TSharedRef<SGraphPin> PinToAdd)
{
	const UEdGraphPin* PinObj = PinToAdd->GetPinObj();

	// Always add the main "In" pin to the first fragment
	if (!FragmentInputBoxes.IsValidIndex(0))
	{
		UE_LOG(FlowYap, Warning, TEXT("COULD NOT ADD INPUT PIN: %s, perhaps node is corrupt?"
							"Copy paste this node and delete the old one. "
							"First Node dialogue entry: %s"),
							*PinObj->GetName(),
							*GetFlowYapDialogueNodeMutable()->GetFragments()[0].Bit.GetDialogueText().ToString());
		return;
	}
		
	FMargin LeftMargins = Settings->GetInputPinPadding();

	LeftMargins.Top = 0;
	//LeftMargins.Right = 0;
	LeftMargins.Bottom = 0;
	
	FragmentInputBoxes[0]->InsertSlot(0)
	.HAlign(HAlign_Left)
	.VAlign(VAlign_Top)
	.AutoHeight()
	.Padding(0)
	[
		PinToAdd
	];

	PinToAdd->SetToolTipText(LOCTEXT("Dialogue", "In"));
}

void SFlowGraphNode_YapDialogueWidget::AddConditionPin(const TSharedRef<SGraphPin> PinToAdd)
{
	int32 Index = InputPins.Num() - 1; // Assume In pin is always added first
	const UEdGraphPin* PinObj = PinToAdd->GetPinObj();

	if (!FragmentInputBoxes.IsValidIndex(Index))
	{
		UE_LOG(FlowYap, Warning, TEXT("COULD NOT ADD INPUT PIN: %s, perhaps node is corrupt?"
							"Copy paste this node and delete the old one. "
							"First Node dialogue entry: %s"),
							*PinObj->GetName(),
							*GetFlowYapDialogueNodeMutable()->GetFragments()[0].Bit.GetDialogueText().ToString());
		return;
	}
		
	FMargin LeftMargins = Settings->GetInputPinPadding();

	LeftMargins.Top = 0;
	//LeftMargins.Right = 0;
	LeftMargins.Bottom = 0;
	
	FragmentInputBoxes[Index]->AddSlot()
	.HAlign(HAlign_Left)
	.VAlign(VAlign_Fill)
	[
		SNew(SSpacer)
	];

	FragmentInputBoxes[Index]->AddSlot()
	.HAlign(HAlign_Left)
	.VAlign(VAlign_Bottom)
	.AutoHeight()
	.Padding(0)
	[
		PinToAdd
	];

	PinToAdd->SetToolTipText(LOCTEXT("Dialogue", "Condition"));
	PinToAdd->SetColorAndOpacity(PinToAdd->IsConnected() ? ConnectedConditionPinColor : DisconnectedConditionPinColor);
}

void SFlowGraphNode_YapDialogueWidget::AddOutPin(const TSharedRef<SGraphPin>& PinToAdd)
{
	FMargin RightMargins = Settings->GetInputPinPadding();

	//RightMargins.Left = 0;
	RightMargins.Bottom = 0;
	RightMargins.Top = 0;
	
	FragmentOutputBoxes[0]->InsertSlot(0)
	.HAlign(HAlign_Right)
	.VAlign(VAlign_Top)
	.AutoHeight()
	.Padding(0)
	[
		PinToAdd
	];
	
	PinToAdd->SetToolTipText(LOCTEXT("Dialogue", "Out"));
	PinToAdd->SetColorAndOpacity(YapColor::White);
}

void SFlowGraphNode_YapDialogueWidget::AddBypassPin(const TSharedRef<SGraphPin>& PinToAdd)
{
	BypassOutputBox->SetContent(PinToAdd);
	PinToAdd->SetToolTipText(LOCTEXT("Dialogue", "Bypass, triggers instead of normal outputs if this node has reached activation limits"));
	PinToAdd->SetColorAndOpacity(PinToAdd->IsConnected() ? ConnectedBypassPinColor : DisconnectedBypassPinColor);
}

void SFlowGraphNode_YapDialogueWidget::AddFragmentStartPin(const TSharedRef<SGraphPin>& PinToAdd)
{
	const UEdGraphPin* PinObj = PinToAdd->GetPinObj();

	uint8 BasePins = GetFlowYapDialogueNodeMutable()->GetIsPlayerPrompt() ? 1 : 2;
	
	int32 FragmentIndex = FMath::DivideAndRoundDown(OutputPins.Num() - BasePins, NUM_PINS_PER_FRAGMENT);

	if (!FragmentOutputBoxes.IsValidIndex(FragmentIndex))
	{
		UE_LOG(FlowYap, Warning, TEXT("COULD NOT ADD OUTPUT PIN: %s, perhaps node is corrupt? "
						"Copy paste this node and delete the old one. "
						"First Node dialogue entry: %s"),
						*PinObj->GetName(),
						*GetFlowYapDialogueNodeMutable()->GetFragments()[0].Bit.GetDialogueText().ToString());

		return;
	}
	
	FMargin RightMargins = Settings->GetInputPinPadding();
	//RightMargins.Left = 0;
	RightMargins.Bottom = 0;
	RightMargins.Top = 0;

	FragmentOutputBoxes[FragmentIndex]->AddSlot()
	.HAlign(HAlign_Right)
	.VAlign(VAlign_Bottom)
	.AutoHeight()
	.Padding(0)
	[
		PinToAdd
	];

	FLinearColor Color = PinToAdd->IsConnected() ? ConnectedStartPinColor : DisconnectedStartPinColor;
	FText ToolTipText = LOCTEXT("Fragment", "On Start");
	
	PinToAdd->SetColorAndOpacity(Color);
	PinToAdd->SetToolTipText(ToolTipText);
}

void SFlowGraphNode_YapDialogueWidget::AddFragmentEndPin(const TSharedRef<SGraphPin>& PinToAdd)
{
	const UEdGraphPin* PinObj = PinToAdd->GetPinObj();

	uint8 BasePins = GetFlowYapDialogueNodeMutable()->GetIsPlayerPrompt() ? 1 : 2; // Bypass or Bypass + Out
	
	int32 FragmentIndex = FMath::DivideAndRoundDown(OutputPins.Num() - BasePins, NUM_PINS_PER_FRAGMENT);

	if (!FragmentOutputBoxes.IsValidIndex(FragmentIndex))
	{
		UE_LOG(FlowYap, Warning, TEXT("COULD NOT ADD OUTPUT PIN: %s, perhaps node is corrupt? "
						"Copy paste this node and delete the old one. "
						"First Node dialogue entry: %s"),
						*PinObj->GetName(),
						*GetFlowYapDialogueNodeMutable()->GetFragments()[0].Bit.GetDialogueText().ToString());

		return;
	}
	
	FMargin RightMargins = Settings->GetInputPinPadding();
	RightMargins.Left = 0;
	RightMargins.Bottom = 0;
	RightMargins.Top = 0;

	if (GetFlowYapDialogueNodeMutable()->GetIsPlayerPrompt())
	{
		FragmentOutputBoxes[FragmentIndex]->InsertSlot(0)
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Top)
		.AutoHeight()
		.Padding(0)
		[
			PinToAdd
		];
	}
	else
	{
		FragmentOutputBoxes[FragmentIndex]->AddSlot()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Top)
		.AutoHeight()
		.Padding(0)
		[
			PinToAdd
		];
	}

	FLinearColor Color = PinToAdd->IsConnected() ? ConnectedEndPinColor : (GetFlowYapDialogueNodeMutable()->GetIsPlayerPrompt() ? DisconnectedEndPinColor_Prompt : DisconnectedEndPinColor);
	FText ToolTipText = LOCTEXT("Fragment", "On End");
	
	PinToAdd->SetColorAndOpacity(Color);
	PinToAdd->SetToolTipText(ToolTipText);
}

void SFlowGraphNode_YapDialogueWidget::CreateStandardPinWidget(UEdGraphPin* Pin)
{
	FName PinName = Pin->GetFName();

	TSharedPtr<SGraphPin> NewPin;

	TArray<FName> NormalPins;

	if (GetFlowYapDialogueNodeMutable()->GetIsPlayerPrompt())
	{
		NormalPins = { "In", "Bypass", "FragmentEnd" };
	}
	else
	{
		NormalPins = { "In", "Out", "Bypass" };
	}

	if (NormalPins.ContainsByPredicate([&](FName OtherPin)
	{
		return OtherPin.IsEqual(PinName, ENameCase::IgnoreCase, false);
	}))
	{
		NewPin = SNew(SFlowGraphPinExec, Pin);
	}
	else // if (PinName.IsEqual(FName("DialogueStart"), ENameCase::IgnoreCase, false))
	{
		NewPin = SNew(SFlowYapGraphPinExec, Pin);
	}
	
	this->AddPin(NewPin.ToSharedRef());
}

const FFlowYapFragment* SFlowGraphNode_YapDialogueWidget::GetFragment(uint8 FragmentIndex) const
{
	return GetFlowYapDialogueNode()->GetFragmentByIndex(FragmentIndex);
}

FFlowYapFragment* SFlowGraphNode_YapDialogueWidget::GetFragmentMutable(uint8 FragmentIndex)
{
	return GetFlowYapDialogueNodeMutable()->GetFragmentByIndexMutable(FragmentIndex);
}

#undef LOCTEXT_NAMESPACE
