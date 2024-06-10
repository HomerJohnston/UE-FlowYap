// Copyright Ghost Pepper Games, Inc. All Rights Reserved.

#include "Widgets/SFlowGraphNode_YapDialogueWidget.h"

#include "FlowEditorStyle.h"
#include "FlowYapColors.h"
#include "FlowYapEditorSettings.h"
#include "FlowYapEditorSubsystem.h"
#include "FlowYapInputTracker.h"
#include "FlowYapTransactions.h"
#include "GraphEditorSettings.h"
#include "Blueprint/SlateBlueprintLibrary.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "FlowYap/FlowYapLog.h"
#include "FlowYap/Nodes/FlowNode_YapDialogue.h"
#include "Graph/FlowGraphEditor.h"
#include "Graph/FlowGraphSettings.h"
#include "Graph/FlowGraphUtils.h"
#include "GraphNodes/FlowGraphNode_YapDialogue.h"
#include "Widgets/SFlowGraphNode_YapFragmentWidget.h"

#define LOCTEXT_NAMESPACE "FlowYap"

// ------------------------------------------
// CONSTRUCTION
void SFlowGraphNode_YapDialogueWidget::Construct(const FArguments& InArgs, UFlowGraphNode* InNode)
{	
	GraphNode = InNode;
	FlowGraphNode = InNode;
	FlowGraphNode_YapDialogue = Cast<UFlowGraphNode_YapDialogue>(InNode);

	DialogueButtonsColor = FlowYapColor::DimGray;

	ConnectedEndPinColor = FlowYapColor::White;
	DisconnectedEndPinColor = FlowYapColor::DarkGray;
	DisconnectedEndPinColor_Prompt = FlowYapColor::Red;
	
	ConnectedStartPinColor = FlowYapColor::LightGreen;
	DisconnectedStartPinColor = FlowYapColor::DarkGray;
	
	ConnectedInterruptPinColor = FlowYapColor::LightRed;
	DisconnectedInterruptPinColor = FlowYapColor::DarkGray;
	ConnectedInterruptPinColor_Disabled = ConnectedInterruptPinColor.Desaturate(0.75);
	DisconnectedInterruptPinColor_Disabled = DisconnectedInterruptPinColor.Desaturate(0.75);
	
	ConnectedBypassPinColor = FlowYapColor::LightBlue;
	DisconnectedBypassPinColor = FlowYapColor::DarkGray;

	FocusedFragmentIndex.Reset();
	
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
			.WidthOverride(66)
			.HeightOverride(20)
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
						.ColorAndOpacity(FlowYapColor::LightRed)
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
		return FlowYapColor::Red;
	}
	else if (GetFlowYapDialogueNode()->Interruptible == EFlowYapInterruptible::Interruptible)
	{
		return FlowYapColor::Green;
	}
	else
	{
		return FlowYapColor::DarkGray;
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
	
	for (uint8 f = 0; f < GetFlowYapDialogueNode()->GetNumFragments(); ++f)
	{
		FFlowYapFragment& Fragment = GetFlowYapDialogueNode()->GetFragmentsMutable()[f];

		if (f == GetFlowYapDialogueNode()->GetNumFragments() - 1)
		{
			bLastFragment = true;
		}
		
		TSharedPtr<SFlowGraphNode_YapFragmentWidget> NewFragmentWidget = MakeShared<SFlowGraphNode_YapFragmentWidget>();

		FragmentWidgets.Add(NewFragmentWidget);

		FragmentBox->AddSlot()
		.AutoHeight()
		.Padding(0, 0, 0, 0)
		[
			CreateFragmentSeparatorWidget(f)
		];	
		
		FragmentBox->AddSlot()
		.AutoHeight()
		.Padding(0, bFirstFragment ? 1 : Padding, 0, bSingleFragment || bLastFragment ? 2 : Padding)
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			[
				CreateFragmentRowWidget(Fragment)
			]
			+ SOverlay::Slot()
			[
				SNew(SBorder)
				.Padding(0)
				.BorderImage(FAppStyle::GetBrush("Menu.Background"))
				.Visibility(this, &SFlowGraphNode_YapDialogueWidget::FragmentRowHighlight_Visibility, f)
				.BorderBackgroundColor(this, &SFlowGraphNode_YapDialogueWidget::FragmentRowHighlight_BorderBackgroundColor, f)
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
	if (FlashFragmentIndex == f)
	{
		return EVisibility::HitTestInvisible;
	}

	return EVisibility::Collapsed;
}

FSlateColor SFlowGraphNode_YapDialogueWidget::FragmentRowHighlight_BorderBackgroundColor(uint8 f) const
{
	if (FlashFragmentIndex == f)
	{
		return FlashHighlight * FlowYapColor::White_Trans;
	}

	return FlowYapColor::Transparent;
}

TSharedRef<SWidget> SFlowGraphNode_YapDialogueWidget::CreateFragmentSeparatorWidget(int FragmentIndex)
{
	return SNew(SButton)
	.ButtonStyle(FAppStyle::Get(), "SimpleButton")
	.ToolTipText(LOCTEXT("DialogueNode", "Insert new fragment"))
	.ContentPadding(0)
	.OnClicked(this, &SFlowGraphNode_YapDialogueWidget::FragmentSeparator_OnClicked, FragmentIndex)
	[
		SNew(SImage)
		.Image(FAppStyle::GetBrush("Menu.Separator"))
		.DesiredSizeOverride(FVector2D(1, 1))
		.ColorAndOpacity(this, &SFlowGraphNode_YapDialogueWidget::FragmentSeparator_ColorAndOpacity)
	];
}

FSlateColor SFlowGraphNode_YapDialogueWidget::FragmentSeparator_ColorAndOpacity() const
{
	return FlowYapColor::Black;
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

TSharedRef<SWidget> SFlowGraphNode_YapDialogueWidget::CreateFragmentRowWidget(FFlowYapFragment& Fragment)
{
	return SNew(SHorizontalBox)
	// LEFT PANE
	+ SHorizontalBox::Slot()
	.AutoWidth()
	.Padding(0, 0, 0, 0)
	.VAlign(VAlign_Fill)
	[
		CreateLeftFragmentPane(Fragment)
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
		CreateRightFragmentPane()
	];
}

// ================================================================================================
// LEFT SIDE PANE
// ------------------------------------------------------------------------------------------------

TSharedRef<SBox> SFlowGraphNode_YapDialogueWidget::CreateLeftFragmentPane(FFlowYapFragment& Fragment)
{
	return SNew(SBox)
	.WidthOverride(40)
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
			CreateActivationLimiterWidget(Fragment)
		]
	];
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

TSharedRef<SBox> SFlowGraphNode_YapDialogueWidget::CreateActivationLimiterWidget(FFlowYapFragment& Fragment)
{
	TSharedRef<SVerticalBox> ActivationLimiter_VerticalBox = SNew(SVerticalBox);

	int32 ActivationCount = Fragment.GetActivationCount();
	int32 ActivationLimit = Fragment.GetActivationLimit();

	if (ActivationLimit <= 3) // TODO 3 twice should be a gvar
	{
		for (int i = 0; i < 3; ++i)
		{
			int32 Size = 16;// i < ActivationCount ? 16 : 12;
			ActivationLimiter_VerticalBox->AddSlot()
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
		ActivationLimiter_VerticalBox->AddSlot()
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

	return SNew(SBox)
	.Visibility(this, &SFlowGraphNode_YapDialogueWidget::ActivationLimiter_Visibility, this, &Fragment)
	[
		ActivationLimiter_VerticalBox
	];
}

EVisibility SFlowGraphNode_YapDialogueWidget::ActivationLimiter_Visibility(SFlowGraphNode_YapDialogueWidget* FlowGraphNode_YapDialogueWidget, FFlowYapFragment* FlowYapFragment) const
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
	TSharedRef<SVerticalBox> RightSideNodeBox = SNew(SVerticalBox);
	
	FragmentOutputBoxes.Add(RightSideNodeBox);
	
	return SNew(SBox)
	.MinDesiredWidth(40)
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

void SFlowGraphNode_YapDialogueWidget::SetFocusedFragment(uint8 InFragment)
{
	if (FocusedFragmentIndex != InFragment)
	{
		TSharedPtr<SFlowGraphEditor> GraphEditor = FFlowGraphUtils::GetFlowGraphEditor(this->FlowGraphNode->GetGraph());
		GraphEditor->SetNodeSelection(FlowGraphNode, true);
		
		FocusedFragmentIndex = InFragment;
	}

	SetTypingFocus();
}

void SFlowGraphNode_YapDialogueWidget::ClearFocusedFragment(uint8 FragmentIndex)
{
	if (FocusedFragmentIndex == FragmentIndex)
	{
		FocusedFragmentIndex.Reset();
	}
}

const TSharedPtr<SFlowGraphNode_YapFragmentWidget> SFlowGraphNode_YapDialogueWidget::GetFocusedFragment() const
{
	if (FocusedFragmentIndex.IsSet())
	{
		return FragmentWidgets[FocusedFragmentIndex.GetValue()];
	}
	else
	{
		return nullptr;
	}
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

		PinToAdd->SetToolTipText(FText::FromName(PinToAdd->GetPinObj()->GetFName()));

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
				Color = PinToAdd->IsConnected() ? ConnectedEndPinColor : (GetFlowYapDialogueNode()->GetIsPlayerPrompt() ? DisconnectedEndPinColor_Prompt : DisconnectedEndPinColor);
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

#undef LOCTEXT_NAMESPACE