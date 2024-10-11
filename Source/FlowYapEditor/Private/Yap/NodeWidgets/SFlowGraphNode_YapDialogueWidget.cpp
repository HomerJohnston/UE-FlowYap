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
#include "Yap/FlowYapCondition.h"
#include "Yap/FlowYapEditorSettings.h"
#include "Yap/FlowYapEditorSubsystem.h"
#include "Yap/FlowYapFragment.h"
#include "Yap/FlowYapInputTracker.h"
#include "Yap/FlowYapLog.h"
#include "Yap/FlowYapProjectSettings.h"
#include "Yap/FlowYapTransactions.h"
#include "Yap/YapEditorStyle.h"
#include "Yap/GraphNodes/FlowGraphNode_YapDialogue.h"
#include "Yap/Helpers/FlowYapWidgetHelper.h"
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

	FlowGraphNode->OnSignalModeChanged.BindRaw(this, &SFlowGraphNode_YapDialogueWidget::UpdateGraphNode);
	FlowGraphNode_YapDialogue->OnYapNodeChanged.BindRaw(this, &SFlowGraphNode_YapDialogueWidget::UpdateGraphNode);
	
	UpdateGraphNode();
}

int32 SFlowGraphNode_YapDialogueWidget::GetDialogueActivationCount() const
{
	return GetFlowYapDialogueNode()->GetNodeActivationCount();
}

int32 SFlowGraphNode_YapDialogueWidget::GetDialogueActivationLimit() const
{
	return GetFlowYapDialogueNode()->GetNodeActivationLimit();
}

EVisibility SFlowGraphNode_YapDialogueWidget::InterruptibleToggleIconOff_Visibility() const
{
	switch (GetFlowYapDialogueNode()->GetInterruptibleSetting())
	{
		case EFlowYapInterruptible::UseProjectDefaults:
		{
			return UFlowYapProjectSettings::Get()->GetDialogueInterruptibleByDefault() ? EVisibility::Collapsed : EVisibility::Visible;
		}
		case EFlowYapInterruptible::Interruptible:
		{
			return EVisibility::Collapsed;
		}
		case EFlowYapInterruptible::NotInterruptible:
		{
			return EVisibility::Visible;
		}
	}
	return (GetFlowYapDialogueNode()->Interruptible == EFlowYapInterruptible::NotInterruptible) ? EVisibility::HitTestInvisible : EVisibility::Collapsed;
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

	TSharedRef<SWidget> Widget = SNew(SBox)
	.WidthOverride(400)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(-8, -5, 16, -6)
		[
			SNew(SActivationCounterWidget)
			.ActivationCount(this, &SFlowGraphNode_YapDialogueWidget::GetDialogueActivationCount)
			.ActivationLimit(this, &SFlowGraphNode_YapDialogueWidget::GetDialogueActivationLimit)
			.FontHeight(8)
		]
		+ SHorizontalBox::Slot()
		.HAlign(HAlign_Fill)
		.Padding(-8,0,2,0)
		[
			CreateConditionWidgets()
		]
		+ SHorizontalBox::Slot()
		.HAlign(HAlign_Right)
		.Padding(2,0,2,0)
		.AutoWidth()
		[
			FFlowYapWidgetHelper::CreateTagPreviewWidget(this, &SFlowGraphNode_YapDialogueWidget::DialogueTagPreview_Text, &SFlowGraphNode_YapDialogueWidget::DialogueTagPreview_Visibility)
		]
		+ SHorizontalBox::Slot()
		.HAlign(HAlign_Right)
		.AutoWidth()
		.Padding(2, -2, -25, -2)
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
							.ColorAndOpacity(this, &SFlowGraphNode_YapDialogueWidget::InterruptibleToggleIcon_ColorAndOpacity)
							.DesiredSizeOverride(FVector2D(16, 16))
							.Image(FAppStyle::Get().GetBrush("SourceControl.StatusIcon.Off"))
							.Visibility(this, &SFlowGraphNode_YapDialogueWidget::InterruptibleToggleIconOff_Visibility)
							//.Visibility_Lambda([this](){return ( GetFlowYapDialogueNodeMutable()->Interruptible == EFlowYapInterruptible::NotInterruptible) ? EVisibility::HitTestInvisible : EVisibility::Collapsed; })
						]
					]
				]
			]
		]
	];
	
	return Widget;
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
	FFlowYapTransactions::BeginModify(LOCTEXT("YapDialogue", "Toggle Interruptible"), GetFlowYapDialogueNodeMutable());

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

	FFlowYapTransactions::EndModify();
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
	TSharedPtr<SVerticalBox> Content; 
	
	return SNew(SBox)
	.WidthOverride(400)
	[
		SAssignNew(Content, SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 5, 0, 5)
		[
			CreateContentHeader()
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			CreateFragmentBoxes()
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			CreateContentFooter()
		]
	];
}

FSlateColor SFlowGraphNode_YapDialogueWidget::NodeHeader_Color() const
{
	if (GetFlowYapDialogueNode()->ActivationLimitsMet())
	{
		return YapColor::Red;
	}

	if (GetFlowYapDialogueNode()->GetIsPlayerPrompt())
	{
		return YapColor::White;
	}

	switch (GetFlowYapDialogueNode()->GetMultipleFragmentSequencing())
	{
		case EFlowYapMultipleFragmentSequencing::SelectOne:
		{
			return YapColor::White;
		}
		case EFlowYapMultipleFragmentSequencing::Sequential:
		{
			return YapColor::White;
		}
	}
	
	return YapColor::Error;
}

TSharedRef<SWidget> SFlowGraphNode_YapDialogueWidget::CreateContentHeader()
{
	TSharedRef<SHorizontalBox> Box = SNew(SHorizontalBox)
	+ SHorizontalBox::Slot()
	.AutoWidth()
	.Padding(4, 0, 0, 0)
	[
		SAssignNew(DialogueInputBoxArea, SBox)
	]
	+ SHorizontalBox::Slot()
	.AutoWidth()
	[
		SNew(STextBlock)
		.TextStyle(FYapEditorStyle::Get(), "Text.NodeHeader")
		.Text(this, &SFlowGraphNode_YapDialogueWidget::NodeHeader_Text)
		.ColorAndOpacity(this, &SFlowGraphNode_YapDialogueWidget::NodeHeader_Color)
	]
	+ SHorizontalBox::Slot()
	.AutoWidth()
	.Padding(4, 0)
	[
		SNew(SBox)
		.WidthOverride(24)
		.HeightOverride(24)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		.Padding(0)
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
	[
		SNew(SSpacer)
	]
	+ SHorizontalBox::Slot()
	.HAlign(HAlign_Right)
	.AutoWidth()
	.Padding(0, 0, 4, 0)
	[
		SAssignNew(DialogueOutputBoxArea, SBox)
	];

	return Box;
}

TSharedRef<SWidget> SFlowGraphNode_YapDialogueWidget::CreateFragmentBoxes()
{
	bool bSingleFragment = GetFlowYapDialogueNode()->GetNumFragments() == 1;
	bool bFirstFragment = true;
	bool bLastFragment = false;

	TSharedRef<SVerticalBox> FragmentBoxes = SNew(SVerticalBox);

	FragmentWidgets.Empty();
	
	for (uint8 FragmentIndex = 0; FragmentIndex < GetFlowYapDialogueNode()->GetNumFragments(); ++FragmentIndex)
	{
		const FFlowYapFragment& Fragment = GetFlowYapDialogueNode()->GetFragments()[FragmentIndex];

		if (FragmentIndex == GetFlowYapDialogueNode()->GetNumFragments() - 1)
		{
			bLastFragment = true;
		}
		
		FragmentBoxes->AddSlot()
		.AutoHeight()
		.Padding(0, bFirstFragment ? 0 : 15, 0, 15)
		[
			CreateFragmentSeparatorWidget(FragmentIndex)
		];
		
		FragmentBoxes->AddSlot()
		.AutoHeight()
		.Padding(0, 0, 0, 0)
		[
			CreateFragmentRowWidget(FragmentIndex)
		];
		
		bFirstFragment = false;
	};

	return FragmentBoxes;
}

FText SFlowGraphNode_YapDialogueWidget::NodeHeader_Text() const
{
	if (GetFlowYapDialogueNode()->GetIsPlayerPrompt())
	{
		return INVTEXT("PROMPT");
	}
	else
	{
		return INVTEXT("TALK");
	}
}

EVisibility SFlowGraphNode_YapDialogueWidget::FragmentRowHighlight_Visibility(uint8 f) const
{
	if (FlashFragmentIndex == f || (GetFlowYapDialogueNode()->GetRunningFragmentIndex() == f && GetFlowYapDialogueNode()->FragmentStartedTime > GetFlowYapDialogueNode()->FragmentEndedTime))
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


TSharedRef<SWidget> SFlowGraphNode_YapDialogueWidget::CreateFragmentSeparatorWidget(uint8 FragmentIndex) const
{
	TSharedRef<SHorizontalBox> Box = SNew(SHorizontalBox);
	
	Box->AddSlot()
	.HAlign(HAlign_Fill)
	.Padding(0, 0, 0, 0)
	[
		SNew(SImage)
		.Image(FAppStyle::GetBrush("Menu.Separator"))
		.DesiredSizeOverride(FVector2D(1, 1))
		.ColorAndOpacity(this, &SFlowGraphNode_YapDialogueWidget::FragmentSeparator_ColorAndOpacity)
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
	TSharedPtr<SFlowGraphNode_YapFragmentWidget> FragmentWidget = SNew(SFlowGraphNode_YapFragmentWidget, this, FragmentIndex);

	FragmentWidgets.Add(FragmentWidget);
	
	return FragmentWidget.ToSharedRef();
	/*
	return SNew(SHorizontalBox)
	// MIDDLE PANE
	+ SHorizontalBox::Slot()
	.VAlign(VAlign_Center)
	.HAlign(HAlign_Fill)
	.Padding(0, 0, 32, 0)
	[
		SNew(SFlowGraphNode_YapFragmentWidget, this, FragmentIndex)
	]
	// RIGHT PANE
	+ SHorizontalBox::Slot()
	.AutoWidth()
	.VAlign(VAlign_Fill)
	[
		CreateRightFragmentPane(FragmentIndex)
	];
	*/
}

// ================================================================================================
// LEFT SIDE PANE
// ------------------------------------------------------------------------------------------------

TSharedRef<SBox> SFlowGraphNode_YapDialogueWidget::CreateLeftFragmentPane(uint8 FragmentIndex)
{
	return SNew(SBox)
	.WidthOverride(32)
	[
		SNew(SOverlay)
		/*
		+ SOverlay::Slot()
		.Padding(-72, 0, 0, 0)
		[
			CreateFragmentControlsWidget(FragmentIndex)
		]
		*/
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
		]
	];
}

// ================================================================================================
// INPUT NODE BOX (UPPER HALF OF LEFT SIDE PANE)
// ------------------------------------------------------------------------------------------------

TSharedRef<SBox> SFlowGraphNode_YapDialogueWidget::CreateLeftSideNodeBox()
{
	TSharedRef<SVerticalBox> LeftSideNodeBox = SNew(SVerticalBox);

	return SNew(SBox)
	.MinDesiredHeight(16)
	[
		LeftSideNodeBox
	];
}

// ================================================================================================
// RIGHT PANE OF FRAGMENT ROW
// ------------------------------------------------------------------------------------------------


EVisibility SFlowGraphNode_YapDialogueWidget::EnableOnStartPinButton_Visibility(uint8 FragmentIndex) const
{
	if (GEditor->IsPlayingSessionInEditor())
	{
		return EVisibility::Collapsed;
	}
	
	const FFlowYapFragment* Fragment = GetFragment(FragmentIndex);

	if (Fragment)
	{
		return Fragment->GetShowOnStartPin() ? EVisibility::Collapsed : EVisibility::Visible;
	}

	return EVisibility::Collapsed;
}

EVisibility SFlowGraphNode_YapDialogueWidget::EnableOnEndPinButton_Visibility(uint8 FragmentIndex) const
{
	if (GEditor->IsPlayingSessionInEditor())
	{
		return EVisibility::Collapsed;
	}
		
	const FFlowYapFragment* Fragment = GetFragment(FragmentIndex);

	if (Fragment)
	{
		return Fragment->GetShowOnEndPin() ? EVisibility::Collapsed : EVisibility::Visible;
	}

	return EVisibility::Collapsed;
}

FReply SFlowGraphNode_YapDialogueWidget::EnableOnStartPinButton_OnClicked(uint8 FragmentIndex)
{
	FFlowYapTransactions::BeginModify(LOCTEXT("YapDialogue", "Enable OnStart Pin"), GetFlowYapDialogueNodeMutable());

	GetFragmentMutable(FragmentIndex)->bShowOnStartPin = true;

	GetFlowYapDialogueNode()->OnReconstructionRequested.Execute();

	FFlowYapTransactions::EndModify();

	return FReply::Handled();
}

FReply SFlowGraphNode_YapDialogueWidget::EnableOnEndPinButton_OnClicked(uint8 FragmentIndex)
{
	FFlowYapTransactions::BeginModify(LOCTEXT("YapDialogue", "Enable OnEnd Pin"), GetFlowYapDialogueNodeMutable());
	
	GetFragmentMutable(FragmentIndex)->bShowOnEndPin = true;

	GetFlowYapDialogueNode()->OnReconstructionRequested.Execute();

	FFlowYapTransactions::EndModify();
	
	return FReply::Handled();
}

TSharedRef<SBox> SFlowGraphNode_YapDialogueWidget::CreateRightFragmentPane(uint8 FragmentIndex)
{
	TSharedRef<SOverlay> RightSideNodeBox = SNew(SOverlay)
	+ SOverlay::Slot()
	.HAlign(HAlign_Center)
	.VAlign(VAlign_Bottom)
	.Padding(4, 0, 2, 6)
	[
		SNew(SBox)
		.WidthOverride(10)
		.HeightOverride(10)
		.Visibility(this, &SFlowGraphNode_YapDialogueWidget::EnableOnStartPinButton_Visibility, FragmentIndex)
		[
			SNew(SButton)
			.OnClicked(this, &SFlowGraphNode_YapDialogueWidget::EnableOnStartPinButton_OnClicked, FragmentIndex)
			.ButtonColorAndOpacity(YapColor::LightGray_Trans)
			.ToolTipText(INVTEXT("Click to enable 'On Start' Pin"))
		]
	]
	+ SOverlay::Slot()
	.HAlign(HAlign_Center)
	.VAlign(VAlign_Bottom)
	.Padding(4, 0, 2, 26)
	[
		SNew(SBox)
		.WidthOverride(10)
		.HeightOverride(10)
		.Visibility(this, &SFlowGraphNode_YapDialogueWidget::EnableOnEndPinButton_Visibility, FragmentIndex)
		[
			SNew(SButton)
			.OnClicked(this, &SFlowGraphNode_YapDialogueWidget::EnableOnEndPinButton_OnClicked, FragmentIndex)
			.ButtonColorAndOpacity(YapColor::LightGray_Trans)
			.ToolTipText(INVTEXT("Click to enable 'On End' Pin"))
		]
	];
	
	//FragmentOutputBoxes.Add(RightSideNodeBox);
	
	return SNew(SBox)
	.MinDesiredWidth(32) // Unreal's input nodes are larger than its output nodes. The left side will be 32 px wide. The right side will be about 27 px wide without this. Thanks Epic.
	.VAlign(VAlign_Fill)
	.Padding(0, 0, 0, 0)
	[
		RightSideNodeBox
	];
}

// ================================================================================================
// BOTTOM BAR
// ------------------------------------------------------------------------------------------------

TSharedRef<SHorizontalBox> SFlowGraphNode_YapDialogueWidget::CreateContentFooter()
{
	return SNew(SHorizontalBox)
	+ SHorizontalBox::Slot()
	.HAlign(HAlign_Fill)
	.VAlign(VAlign_Fill)
	.Padding(32, 2, 2, 2)
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
	.Padding(0, 2, 3, 2)
	[
		SAssignNew(BypassOutputBox, SBox)
		.HAlign(HAlign_Center)
		.WidthOverride(24)
		.HeightOverride(24)
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

	FFlowYapTransactions::EndModify();
	
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
			return YapColor::LightBlue;
		}
	case EFlowYapMultipleFragmentSequencing::SelectOne:
		{
			return YapColor::LightOrange;
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

TSharedRef<SWidget> SFlowGraphNode_YapDialogueWidget::CreateDialogueTagPreviewWidget() const
{	
	return SNew(SBorder)
	.BorderImage(FYapEditorStyle::Get().GetBrush("ImageBrush.Box.SolidWhite.DeburredCorners"))
	.BorderBackgroundColor(YapColor::DeepGray)
	.ColorAndOpacity(YapColor::White)//this, &SFlowGraphNode_YapFragmentWidget::FragmentTagPreview_ColorAndOpacity)
	.VAlign(VAlign_Center)
	.HAlign(HAlign_Center)
	[
		SNew(STextBlock)
		.Text(this, &SFlowGraphNode_YapDialogueWidget::DialogueTagPreview_Text)
		.IsEnabled(false)
		.Font(FCoreStyle::GetDefaultFontStyle("Bold", 8))
	];
}

FText SFlowGraphNode_YapDialogueWidget::DialogueTagPreview_Text() const
{
	FText Text = FText::FromString(UFlowYapProjectSettings::GetTrimmedGameplayTagString(EFlowYap_TagFilter::Prompts, GetFlowYapDialogueNode()->GetDialogueTag()));

	if (Text.IsEmptyOrWhitespace())
	{
		return INVTEXT("<None>");
	}
	else
	{
		return Text;
	}
}

EVisibility SFlowGraphNode_YapDialogueWidget::DialogueTagPreview_Visibility() const
{
	return GetFlowYapDialogueNode()->DialogueTag.IsValid() ? EVisibility::Visible : EVisibility::Collapsed;
}

EVisibility SFlowGraphNode_YapDialogueWidget::ConditionWidgets_Visibility() const
{
	return (GetFlowYapDialogueNode()->GetConditions().Num() > 0) ? EVisibility::Visible : EVisibility::Hidden;
}

TSharedRef<SWidget> SFlowGraphNode_YapDialogueWidget::CreateConditionWidgets()
{
	TSharedRef<SScrollBox> Box = SNew(SScrollBox)
	.Visibility(this, &SFlowGraphNode_YapDialogueWidget::ConditionWidgets_Visibility)
	.ScrollBarVisibility(EVisibility::Collapsed)
	.ConsumeMouseWheel(EConsumeMouseWheel::Always)
	.AllowOverscroll(EAllowOverscroll::No)
	.AnimateWheelScrolling(true)
	.Orientation(Orient_Horizontal);

	for (const UFlowYapCondition* Condition : GetFlowYapDialogueNode()->GetConditions())
	{
		Box->AddSlot()
		.Padding(0, 0, 4, 0)
		[
			FFlowYapWidgetHelper::CreateConditionWidget(Condition)
		];	
	}
	
	return Box;
}

// ------------------------------------------
// PUBLIC API & THEIR HELPERS

TSharedRef<SWidget> SFlowGraphNode_YapDialogueWidget::CreateConditionWidget(const UFlowYapCondition* Condition)
{
	FString Description = IsValid(Condition) ? Condition->GetDescription() : "<Null Condition>";
	
	return SNew(SBorder)
	.BorderImage(FYapEditorStyle::Get().GetBrush("ImageBrush.Box.SolidWhite.DeburredCorners"))
	.BorderBackgroundColor(YapColor::DarkOrangeRed)
	.VAlign(VAlign_Center)
	.HAlign(HAlign_Center)
	[
		SNew(STextBlock)
		.Text(FText::FromString(Description))
		.ColorAndOpacity(YapColor::White)
		.Font(FCoreStyle::GetDefaultFontStyle("Bold", 8))
	];
}

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
	GetFlowYapDialogueNodeMutable()->SwapFragments(FragmentIndex, FragmentIndex + By);
	
	FlowGraphNode_YapDialogue->SwapFragmentPinConnections(FragmentIndex, FragmentIndex + By);


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

	/*
	if (GetFlowYapDialogueNode()->GetIsPlayerPrompt())
	{
		// TODO a custom brush style
		return FAppStyle::GetBrush(TEXT("Graph.ReadOnlyBorder"));
	}
	*/
	
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
		AddInPin(PinToAdd);

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
		else
		{
			const FGuid* FragmentGuid = GetFlowYapDialogueNode()->FragmentPinMap.Find(PinName);

			if (!FragmentGuid)
			{
				UE_LOGFMT(FlowYap, Warning, "Could not find fragment for pin: {0}", PinName);
				return;
			}
			
			int32 Index = GetFlowYapDialogueNode()->GetFragmentIndex(*FragmentGuid);

			if (Index < 0)
			{
				UE_LOGFMT(FlowYap, Warning, "Invalid output pin name: {0}", PinName);
				return;
			}

			AddFragmentPin(PinToAdd, Index);
		}
	}

	OutputPins.Add(PinToAdd);
}

void SFlowGraphNode_YapDialogueWidget::AddInPin(const TSharedRef<SGraphPin> PinToAdd)
{
	const UEdGraphPin* PinObj = PinToAdd->GetPinObj();
		
	FMargin LeftMargins = Settings->GetInputPinPadding();

	LeftMargins.Top = 0;
	//LeftMargins.Right = 0;
	LeftMargins.Bottom = 0;

	DialogueInputBoxArea->SetContent(PinToAdd);

	PinToAdd->SetToolTipText(LOCTEXT("Dialogue", "In"));
}

void SFlowGraphNode_YapDialogueWidget::AddOutPin(const TSharedRef<SGraphPin>& PinToAdd)
{
	FMargin RightMargins = Settings->GetInputPinPadding();

	RightMargins.Bottom = 0;
	RightMargins.Top = 0;

	DialogueOutputBoxArea->SetContent(PinToAdd);
	
	PinToAdd->SetToolTipText(LOCTEXT("Dialogue", "Out"));
	PinToAdd->SetColorAndOpacity(YapColor::White);
}

void SFlowGraphNode_YapDialogueWidget::AddBypassPin(const TSharedRef<SGraphPin>& PinToAdd)
{
	BypassOutputBox->SetContent(PinToAdd);
	PinToAdd->SetToolTipText(LOCTEXT("Dialogue", "Bypass, triggers instead of normal outputs if this node has reached activation limits"));
	PinToAdd->SetColorAndOpacity(PinToAdd->IsConnected() ? ConnectedBypassPinColor : DisconnectedBypassPinColor);
}

void SFlowGraphNode_YapDialogueWidget::AddFragmentPin(const TSharedRef<SGraphPin>& PinToAdd, int32 FragmentIndex)
{
	const UEdGraphPin* PinObj = PinToAdd->GetPinObj();

	/*
	if (!FragmentOutputBoxes.IsValidIndex(FragmentIndex))
	{
		UE_LOG(FlowYap, Warning, TEXT("COULD NOT ADD OUTPUT PIN: %s, perhaps node is corrupt? "
						"Copy paste this node and delete the old one. "
						"First Node dialogue entry: %s"),
						*PinObj->GetName(),
						*GetFlowYapDialogueNodeMutable()->GetFragments()[0].Bit.GetDialogueText().ToString());
		return;
	}
	*/
	
	FMargin RightMargins = Settings->GetInputPinPadding();
	RightMargins.Bottom = 0;
	RightMargins.Top = 0;

	EVerticalAlignment Alignment = VAlign_Bottom;
	int BottomPadding = 0; 

	FName PinName = PinObj->GetFName();
	const UFlowNode_YapDialogue* Node = GetFlowYapDialogueNode();
	
	if (Node->OnStartPins.Contains(PinName))
	{
		PinToAdd->SetColorAndOpacity(PinToAdd->IsConnected() ? YapColor::White : YapColor::DarkRed);
		PinToAdd->SetToolTipText(INVTEXT("On Start"));
		BottomPadding = 4;
	}
	else if (Node->OnEndPins.Contains(PinName))
	{
		PinToAdd->SetColorAndOpacity(PinToAdd->IsConnected() ? YapColor::White : YapColor::DarkRed);
		PinToAdd->SetToolTipText(INVTEXT("On End"));
		BottomPadding = 24;
	}
	else
	{
		PinToAdd->SetColorAndOpacity(PinToAdd->IsConnected() ? YapColor::White : YapColor::DarkRed);
		PinToAdd->SetToolTipText(INVTEXT("Prompt Out"));
		Alignment = VAlign_Top;
		BottomPadding = 44;
	}

	TSharedPtr<SFlowGraphNode_YapFragmentWidget> FragmentWidget = FragmentWidgets[FragmentIndex];

	TSharedPtr<SOverlay> Box = FragmentWidget->GetPinContainer();

	Box->AddSlot()
	//FragmentOutputBoxes[FragmentIndex]->AddSlot()
	.HAlign(HAlign_Right)
	.VAlign(Alignment)
	.Padding(0, 0, 4, BottomPadding)
	[
		PinToAdd
	];
}

void SFlowGraphNode_YapDialogueWidget::CreateStandardPinWidget(UEdGraphPin* Pin)
{
	FName PinName = Pin->GetFName();

	TSharedPtr<SGraphPin> NewPin;

	TArray<FName> NormalPins;

	if (GetFlowYapDialogueNode()->OptionalPins.Contains(PinName))
	{
		NewPin = SNew(SFlowYapGraphPinExec, Pin);
	}
	else
	{
		NewPin = SNew(SFlowGraphPinExec, Pin);
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
