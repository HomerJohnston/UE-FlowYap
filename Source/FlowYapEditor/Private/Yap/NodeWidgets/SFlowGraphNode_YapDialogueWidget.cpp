// Copyright Ghost Pepper Games, Inc. All Rights Reserved.

#include "Yap/NodeWidgets/SFlowGraphNode_YapDialogueWidget.h"

#include "FlowEditorStyle.h"
#include "Graph/FlowGraphEditor.h"
#include "Graph/FlowGraphSettings.h"
#include "Graph/FlowGraphUtils.h"
#include "Logging/StructuredLog.h"
#include "Yap/FlowYapBit.h"
#include "Yap/FlowYapColors.h"
#include "Yap/FlowYapEditorSettings.h"
#include "Yap/FlowYapEditorSubsystem.h"
#include "Yap/FlowYapFragment.h"
#include "Yap/FlowYapInputTracker.h"
#include "Yap/FlowYapLog.h"
#include "Yap/FlowYapTransactions.h"
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
	ConnectedInterruptPinColor_Disabled = ConnectedInterruptPinColor.Desaturate(0.75);
	DisconnectedInterruptPinColor_Disabled = DisconnectedInterruptPinColor.Desaturate(0.75);
	
	ConnectedBypassPinColor = YapColor::LightBlue;
	DisconnectedBypassPinColor = YapColor::DarkGray;

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
	.Padding(2,-2,-24,-2)
	[
		SNew(SBox)
		.WidthOverride(32)
		.HAlign(HAlign_Center)
		[
			SAssignNew(InterruptibleCheckBox, SCheckBox)
			.Style(&InterruptibleCheckBoxStyle)
			.Type(ESlateCheckBoxType::ToggleButton)
			.Padding(FMargin(6, 0))
			.CheckBoxContentUsesAutoWidth(true)
			.ToolTipText(LOCTEXT("DialogueNode_Tooltip", "Toggle whether this dialogue can be skipped by the player. Hold CTRL while clicking to reset to project defaults."))
			.IsChecked(this, &SFlowGraphNode_YapDialogueWidget::InterruptibleToggle_IsChecked)
			.OnCheckStateChanged(this, &SFlowGraphNode_YapDialogueWidget::InterruptibleToggle_OnCheckStateChanged)
			.Content()
			[
				SNew(SBox)
				.WidthOverride(16)
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
						.Visibility_Lambda([this](){return ( GetFlowYapDialogueNode()->Interruptible == EFlowYapInterruptible::NotInterruptible) ? EVisibility::HitTestInvisible : EVisibility::Collapsed; })
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
	FFlowYapTransactions::BeginModify(LOCTEXT("Dialogue", "Toggle player prompt node"), GetFlowYapDialogueNode());

	GetFlowYapDialogueNode()->SetIsPlayerPrompt(CheckBoxState == ECheckBoxState::Checked ? true : false);

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

	bool bSingleFragment = GetFlowYapDialogueNode()->GetNumFragments() == 1;
	bool bFirstFragment = true;
	bool bLastFragment = false;

	uint8 Spacing = GetDefault<UFlowYapEditorSettings>()->GetDialogueRowSpacing(); // TODO put this into project settings
	uint8 Padding = 1 + Spacing * 8;
	
	for (uint8 FragmentIndex = 0; FragmentIndex < GetFlowYapDialogueNode()->GetNumFragments(); ++FragmentIndex)
	{
		FFlowYapFragment& Fragment = GetFlowYapDialogueNode()->GetFragmentsMutable()[FragmentIndex];

		if (FragmentIndex == GetFlowYapDialogueNode()->GetNumFragments() - 1)
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
			.Padding(34, -6)
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

TSharedRef<SWidget> SFlowGraphNode_YapDialogueWidget::CreateFragmentSeparatorWidget(int FragmentIndex)
{
	TSharedRef<SOverlay> Overlay = SNew(SOverlay)
	+ SOverlay::Slot()
	.HAlign(HAlign_Fill)
	[
		SNew(SButton)
		.ButtonStyle(FAppStyle::Get(), "SimpleButton")
		.ToolTipText(LOCTEXT("DialogueNode", "Insert new fragment"))
		.ContentPadding(0)
		.OnClicked(this, &SFlowGraphNode_YapDialogueWidget::FragmentSeparator_OnClicked, FragmentIndex)
		[
			SNew(SImage)
			.Image(FAppStyle::GetBrush("Menu.Separator"))
			.DesiredSizeOverride(FVector2D(1, 1))
			.ColorAndOpacity(this, &SFlowGraphNode_YapDialogueWidget::FragmentSeparator_ColorAndOpacity)
		]
	];

	return Overlay;
}

FSlateColor SFlowGraphNode_YapDialogueWidget::FragmentSeparator_ColorAndOpacity() const
{
	return YapColor::Black;
}

FReply SFlowGraphNode_YapDialogueWidget::FragmentSeparator_OnClicked(int Index)
{
	FFlowYapTransactions::BeginModify(LOCTEXT("DialogueAddFragment", "Add Fragment"), GetFlowYapDialogueNode());

	GetFlowYapDialogueNode()->InsertFragment(Index);

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
	.WidthOverride(40)
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
			.VAlign(VAlign_Top)
			.AutoHeight()
			.HAlign(HAlign_Center)
			[
				CreateLeftSideNodeBox()
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			[
				CreateActivationLimiterWidget(FragmentIndex)
			]
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
	FFlowYapTransactions::BeginModify(LOCTEXT("DialogueNode", "Move Fragment"), GetFlowYapDialogueNode());

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
	FFlowYapTransactions::BeginModify(LOCTEXT("DialogueDeleteFragment", "Delete Fragment"), GetFlowYapDialogueNode() /*GetFlowYapDialogueNode()*/);

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
	FFlowYapTransactions::BeginModify(LOCTEXT("DialogueNode", "Move Fragment"), GetFlowYapDialogueNode());

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
// ACTIVATION LIMITER WIDGET (LOWER HALF OF LEFT SIDE PANE)
// ------------------------------------------------------------------------------------------------

TSharedRef<SBox> SFlowGraphNode_YapDialogueWidget::CreateActivationLimiterWidget(uint8 FragmentIndex)
{
	FFlowYapFragment* Fragment = GetFragmentMutable(FragmentIndex);
	
	TSharedRef<SVerticalBox> ActivationLimiter_VerticalBox = SNew(SVerticalBox);

	int32 ActivationCount = Fragment->GetActivationCount();
	int32 ActivationLimit = Fragment->GetActivationLimit();

	if (ActivationLimit <= 3) // TODO 3 twice should be a gvar
	{
		for (int i = 0; i < 1; ++i)
		{
			int32 Size = 24;// i < ActivationCount ? 16 : 12;
			ActivationLimiter_VerticalBox->AddSlot()
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Center)
			.AutoHeight()
			.Padding(0)
			[
				SNew(SBox)
				.WidthOverride(24)
				.HeightOverride(22)
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
					.OnClicked(this, &SFlowGraphNode_YapDialogueWidget::ActivationDot_OnClicked, FragmentIndex, i)
					.ToolTipText(FText::Format(LOCTEXT("DialogueNode_Tooltip", "Set Activation Limit to {0}"), i + 1))
					[
						SNew(SImage)
						.DesiredSizeOverride(FVector2D(Size, Size))
						.Image(FAppStyle::GetBrush("Icons.BulletPoint"))
						.ColorAndOpacity(this, &SFlowGraphNode_YapDialogueWidget::ActivationDot_ColorAndOpacity, FragmentIndex, i)
					]
				]
			];
		}
	}
	else
	{			
		ActivationLimiter_VerticalBox->AddSlot()
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Center)
		.AutoHeight()
		[
			SNew(STextBlock)
			.Text(FText::Join(FText::FromString("/"), FText::AsNumber(ActivationCount), FText::AsNumber(Fragment->GetActivationLimit())))
			.ColorAndOpacity(YapColor::White)
			.TextStyle(&NormalText)
			.Justification(ETextJustify::Center)
		];
	}

	return SNew(SBox)
	.Visibility(this, &SFlowGraphNode_YapDialogueWidget::ActivationLimiter_Visibility, FragmentIndex)
	[
		ActivationLimiter_VerticalBox
	];
}

EVisibility SFlowGraphNode_YapDialogueWidget::ActivationLimiter_Visibility(uint8 FragmentIndex) const
{
	const FFlowYapFragment* Fragment = GetFragment(FragmentIndex);
	
	if (GEditor->PlayWorld)
	{
		return (Fragment->ActivationLimit > 0 || Fragment->ActivationCount > 0) ? EVisibility::Visible : EVisibility::Collapsed;		
	}
	else
	{
		if (IsHovered())
		{
			return EVisibility::Visible;
		}

		return (Fragment->ActivationLimit > 0) ? EVisibility::Visible : EVisibility::Collapsed;	
	}
}

FSlateColor SFlowGraphNode_YapDialogueWidget::ActivationDot_ColorAndOpacity(uint8 FragmentIndex, int32 ActivationIndex) const
{
	const FFlowYapFragment* Fragment = GetFragment(FragmentIndex);
	
	int32 ActivationCount = Fragment->ActivationCount;
	int32 ActivationLimit = Fragment->ActivationLimit;
	int32 Current = ActivationIndex + 1;

	if (GEditor->PlayWorld)
	{
		if (ActivationLimit == 0)
		{
			return Current <= ActivationCount ? YapColor::White : YapColor::Black;
		}

		if (Current > ActivationLimit)
		{
			return YapColor::Transparent;
		}
		
		if (ActivationCount >= ActivationLimit)
		{
			return (Current <= ActivationCount) ? YapColor::Red : YapColor::Black; 
		}
		else
		{
			return (Current <= ActivationCount) ? YapColor::White : YapColor::Black; 
		}
	}

	if (Current <= ActivationLimit)
	{
		return YapColor::Orange;
	}

	return YapColor::Black;
}

FReply SFlowGraphNode_YapDialogueWidget::ActivationDot_OnClicked(uint8 FragmentIndex, int ActivationIndex)
{
	FFlowYapFragment* Fragment = GetFragmentMutable(FragmentIndex);
	
	FFlowYapTransactions::BeginModify(LOCTEXT("Dialogue", "Change activation limit"), FlowGraphNode_YapDialogue);
	
	// TODO ignore input during PIE?
	if (Fragment->ActivationLimit == ActivationIndex + 1)
	{
		Fragment->ActivationLimit = 0;
	}
	else
	{
		Fragment->ActivationLimit = ActivationIndex + 1;
	}

	FFlowYapTransactions::EndModify();
	
	return FReply::Handled();
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
	.MinDesiredWidth(40)
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
	FFlowYapTransactions::BeginModify(LOCTEXT("DialogueAddFragment", "Add Fragment"), GetFlowYapDialogueNode());
	
	GetFlowYapDialogueNode()->AddFragment();

	UpdateGraphNode();

	FFlowYapTransactions::EndModify();
	
	return FReply::Handled();
}

// ------------------------------------------
// PUBLIC API & THEIR HELPERS

void SFlowGraphNode_YapDialogueWidget::DeleteFragment(uint8 FragmentIndex)
{
	FFlowYapTransactions::BeginModify(LOCTEXT("DialogueDeleteFragment", "Delete Fragment"), GetFlowYapDialogueNode());

	FlowGraphNode_YapDialogue->UpdatePinsForFragmentDeletion(FragmentIndex);

	GetFlowYapDialogueNode()->DeleteFragmentByIndex(FragmentIndex);

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
	check(FragmentIndex < GetFlowYapDialogueNode()->GetNumFragments() - 1);
	MoveFragment(FragmentIndex, +1);

	SetFlashFragment(FragmentIndex + 1);
}

void SFlowGraphNode_YapDialogueWidget::MoveFragment(uint8 FragmentIndex, int16 By)
{
	FlowGraphNode_YapDialogue->SwapPinConnections(FragmentIndex, FragmentIndex + By);

	GetFlowYapDialogueNode()->SwapFragments(FragmentIndex, FragmentIndex + By);

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

UFlowNode_YapDialogue* SFlowGraphNode_YapDialogueWidget::GetFlowYapDialogueNode()
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

	bool bShiftPressed = GEditor->GetEditorSubsystem<UFlowYapEditorSubsystem>()->GetInputTracker()->GetShiftPressed();
	
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

	LeftMargins.Top = 0;
	LeftMargins.Right = 0;
	LeftMargins.Bottom = 0;

	RightMargins.Left = 0;
	RightMargins.Bottom = 0;
	RightMargins.Top = 0;
	
	if (PinToAdd->GetDirection() == EEdGraphPinDirection::EGPD_Input)
	{
		int32 Index = InputPins.Num();

		if (!FragmentInputBoxes.IsValidIndex(Index))
		{
			UE_LOG(FlowYap, Warning, TEXT("COULD NOT ADD INPUT PIN: %s, perhaps node is corrupt?"
								"Copy paste this node and delete the old one. "
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

		PinToAdd->SetToolTipText(FText::FromName(PinToAdd->GetPinObj()->GetFName()));

		InputPins.Add(PinToAdd);
	}
	else
	{
		if (PinToAdd->GetPinObj()->GetFName() == FName("Out"))
		{
			AddOutPin(PinToAdd);
		}
		else if (PinToAdd->GetPinObj()->GetFName() == FName("Bypass"))
		{
			AddBypassPin(PinToAdd);
		}
		else
		{
			uint8 BasePins = GetFlowYapDialogueNode()->GetIsPlayerPrompt() ? 1 : 2;
			// Subtract 2 to account for Out and Bypass pins
			int32 FragmentIndex = FMath::DivideAndRoundDown(OutputPins.Num() - BasePins, NUM_PINS_PER_FRAGMENT);

			if (!FragmentOutputBoxes.IsValidIndex(FragmentIndex))
			{
				UE_LOG(FlowYap, Warning, TEXT("COULD NOT ADD OUTPUT PIN: %s, perhaps node is corrupt? "
								"Copy paste this node and delete the old one. "
								"First Node dialogue entry: %s"),
								*PinObj->GetName(),
								*GetFlowYapDialogueNode()->GetFragments()[0].Bit.GetDialogueText().ToString());

				return;
			}
			
			FName PinName = PinToAdd->GetPinObj()->GetFName();

			if (GetFlowYapDialogueNode()->GetIsPlayerPrompt() && PinName.IsEqual("FragmentEnd", ENameCase::IgnoreCase, false))
			{
				FragmentOutputBoxes[FragmentIndex]->InsertSlot(0)
				.HAlign(HAlign_Right)
				.VAlign(VAlign_Top)
				.AutoHeight()
				.Padding(RightMargins)
				[
					PinToAdd
				];
			}
			else
			{				
				FragmentOutputBoxes[FragmentIndex]->AddSlot()
				.HAlign(HAlign_Right)
				.VAlign(VAlign_Bottom)
				.AutoHeight()
				.Padding(RightMargins)
				[
					PinToAdd
				];
			}
			

			FLinearColor Color;
			FText ToolTipText;
			if (PinName.IsEqual("FragmentEnd", ENameCase::IgnoreCase, false))
			{
				Color = PinToAdd->IsConnected() ? ConnectedEndPinColor : (GetFlowYapDialogueNode()->GetIsPlayerPrompt() ? DisconnectedEndPinColor_Prompt : DisconnectedEndPinColor);
				ToolTipText = LOCTEXT("Fragment", "On End");
			}
			else if (PinName.IsEqual("FragmentStart", ENameCase::IgnoreCase, false))
			{
				Color = PinToAdd->IsConnected() ? ConnectedStartPinColor : DisconnectedStartPinColor;
				ToolTipText = LOCTEXT("Fragment", "On Start");
			}
			else
			{
				Color = YapColor::Error;
				ToolTipText = LOCTEXT("Fragment", "Error");
			}
			/*
			else // if (PinName.IsEqual(FName("DialogueInterrupt"), ENameCase::IgnoreCase, false))
			{
				Color = PinToAdd->IsConnected() ? ConnectedInterruptPinColor : DisconnectedInterruptPinColor;
				ToolTipText = LOCTEXT("Fragment", "On Interrupt");
			}
			*/
			
			PinToAdd->SetColorAndOpacity(Color);
			PinToAdd->SetToolTipText(ToolTipText);

			OutputPins.Add(PinToAdd);	
		}
	}
}

void SFlowGraphNode_YapDialogueWidget::CreateStandardPinWidget(UEdGraphPin* Pin)
{
	FName PinName = Pin->GetFName();

	TSharedPtr<SGraphPin> NewPin;

	TArray<FName> NormalPins;

	if (GetFlowYapDialogueNode()->GetIsPlayerPrompt())
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
	
	//const TSharedPtr<SGraphPin> NewPin = SNew(SFlowYapGraphPin, Pin);

	if (!UFlowGraphSettings::Get()->bShowDefaultPinNames && FlowGraphNode->GetFlowNode())
	{
		if (Pin->Direction == EGPD_Input)
		{
		}
		else
		{
		}
	}

	this->AddPin(NewPin.ToSharedRef());
}

void SFlowGraphNode_YapDialogueWidget::AddOutPin(const TSharedRef<SGraphPin>& PinToAdd)
{
	FMargin RightMargins = Settings->GetInputPinPadding();

	RightMargins.Left = 0;
	RightMargins.Bottom = 0;
	RightMargins.Top = 0;
	
	FragmentOutputBoxes[0]->InsertSlot(0)
	.HAlign(HAlign_Right)
	.VAlign(VAlign_Top)
	.AutoHeight()
	.Padding(RightMargins)
	[
		PinToAdd
	];
	
	PinToAdd->SetToolTipText(LOCTEXT("Dialogue", "Out"));
	PinToAdd->SetColorAndOpacity(YapColor::White);
	OutputPins.Add(PinToAdd);
}

void SFlowGraphNode_YapDialogueWidget::AddBypassPin(const TSharedRef<SGraphPin>& PinToAdd)
{
	BypassOutputBox->SetContent(PinToAdd);
	PinToAdd->SetToolTipText(LOCTEXT("Dialogue", "Bypass, executes immediately when this node (or all fragments) have reached activation limits"));
	PinToAdd->SetColorAndOpacity(PinToAdd->IsConnected() ? ConnectedBypassPinColor : DisconnectedBypassPinColor);
	OutputPins.Add(PinToAdd);
}

const FFlowYapFragment* SFlowGraphNode_YapDialogueWidget::GetFragment(uint8 FragmentIndex) const
{
	return GetFlowYapDialogueNode()->GetFragmentByIndex(FragmentIndex);
}

FFlowYapFragment* SFlowGraphNode_YapDialogueWidget::GetFragmentMutable(uint8 FragmentIndex)
{
	return GetFlowYapDialogueNode()->GetFragmentByIndexMutable(FragmentIndex);
}

#undef LOCTEXT_NAMESPACE
