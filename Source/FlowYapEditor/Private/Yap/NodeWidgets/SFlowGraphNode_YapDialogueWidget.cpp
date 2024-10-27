// Copyright Ghost Pepper Games, Inc. All Rights Reserved.

#include "Yap/NodeWidgets/SFlowGraphNode_YapDialogueWidget.h"

#include "FlowEditorStyle.h"
#include "Graph/FlowGraphEditor.h"
#include "Graph/FlowGraphSettings.h"
#include "Graph/FlowGraphUtils.h"
#include "Logging/StructuredLog.h"
#include "Widgets/Images/SLayeredImage.h"
#include "Yap/FlowYapBit.h"
#include "Yap/FlowYapColors.h"
#include "Yap/FlowYapCondition.h"
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

constexpr int32 YAP_MIN_NODE_WIDTH = 275;
constexpr int32 YAP_DEFAULT_NODE_WIDTH = 400;

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

	Pins = GetFlowYapDialogueNode()->GetContextOutputs();

	UpdateGraphNode();
}

void SFlowGraphNode_YapDialogueWidget::ForceUpdateGraphNode()
{
	GetFlowYapDialogueNode()->OnReconstructionRequested.ExecuteIfBound();
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

void SFlowGraphNode_YapDialogueWidget::OnActivationLimitChanged(const FText& Text, ETextCommit::Type Arg)
{
	GetFlowYapDialogueNodeMutable()->NodeActivationLimit = FCString::Atoi(*Text.ToString());
}

FGameplayTag SFlowGraphNode_YapDialogueWidget::Value_DialogueTag() const
{
	return GetFlowYapDialogueNode()->GetDialogueTag();
}

void SFlowGraphNode_YapDialogueWidget::OnTagChanged_DialogueTag(FGameplayTag GameplayTag)
{
	FFlowYapTransactions::BeginModify(LOCTEXT("Fragment", "Change Fragment Tag"), GetFlowYapDialogueNodeMutable());

	GetFlowYapDialogueNodeMutable()->DialogueTag = GameplayTag;

	FFlowYapTransactions::EndModify();

	UpdateGraphNode();
}

int32 SFlowGraphNode_YapDialogueWidget::GetMaxNodeWidth() const
{
	return FMath::Max(YAP_MIN_NODE_WIDTH + UFlowYapProjectSettings::Get()->GetPortraitSize(), YAP_DEFAULT_NODE_WIDTH + UFlowYapProjectSettings::Get()->GetDialogueWidthAdjustment());
}

// ------------------------------------------
// WIDGETS

// ================================================================================================
// TITLE WIDGET
// ------------------------------------------------------------------------------------------------

TSharedRef<SWidget> SFlowGraphNode_YapDialogueWidget::CreateTitleWidget(TSharedPtr<SNodeTitle> NodeTitle)
{
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

	const int32 TITLE_LEFT_RIGHT_EXTRA_WIDTH = 44;

	TSharedRef<SWidget> Widget = SNew(SBox)
	.MaxDesiredWidth(GetMaxNodeWidth() - TITLE_LEFT_RIGHT_EXTRA_WIDTH)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(-8, -5, 16, -6)
		[
			SNew(SActivationCounterWidget, FOnTextCommitted::CreateSP(this, &SFlowGraphNode_YapDialogueWidget::OnActivationLimitChanged))
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
		.VAlign(VAlign_Fill)
		[
			FFlowYapWidgetHelper::CreateFilteredTagWidget(
				TAttribute<FGameplayTag>::CreateSP(this, &SFlowGraphNode_YapDialogueWidget::Value_DialogueTag),
				GetDefault<UFlowYapProjectSettings>()->DialogueTagsParent.ToString(),
				TDelegate<void(const FGameplayTag)>::CreateSP(this, &SFlowGraphNode_YapDialogueWidget::OnTagChanged_DialogueTag))
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
	.WidthOverride(GetMaxNodeWidth())
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

FSlateColor SFlowGraphNode_YapDialogueWidget::ColorAndOpacity_NodeHeader() const
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

FText SFlowGraphNode_YapDialogueWidget::Text_FragmentSequencingButton() const
{
	switch (GetFlowYapDialogueNode()->GetMultipleFragmentSequencing())
	{
		case EFlowYapMultipleFragmentSequencing::SelectOne:
		{
			return INVTEXT("Select one");
		}
		case EFlowYapMultipleFragmentSequencing::Sequential:
		{
			return INVTEXT("Run all");
		}
		default:
		{
			return INVTEXT("Error");
		}
	}
}

FReply SFlowGraphNode_YapDialogueWidget::OnClicked_TogglePlayerPrompt()
{
	FFlowYapTransactions::BeginModify(LOCTEXT("DialogueNode", "Toggle Player Prompt"), GetFlowYapDialogueNodeMutable());
	
	GetFlowYapDialogueNodeMutable()->bIsPlayerPrompt = !GetFlowYapDialogueNode()->bIsPlayerPrompt;

	UFlowGraphNode* T = Cast<UFlowGraphNode>(GetFlowYapDialogueNode()->GetGraphNode());
	T->RefreshContextPins(true);
	
	FFlowYapTransactions::EndModify();	

	return FReply::Handled();
}

TSharedRef<SWidget> SFlowGraphNode_YapDialogueWidget::CreateContentHeader()
{
	TSharedRef<SHorizontalBox> Box = SNew(SHorizontalBox)
	+ SHorizontalBox::Slot()
	.AutoWidth()
	.Padding(4, 0, 0, 0)
	[
		SAssignNew(DialogueInputBoxArea, SVerticalBox)
	]
	+ SHorizontalBox::Slot()
	.AutoWidth()
	.Padding(-2, 0, 0, 0)
	[
		SNew(SButton)
		.ButtonStyle(FYapEditorStyle::Get(), YapStyles.ButtonStyle_SequencingSelector)
		.ContentPadding(FMargin(4, 0, 4, 0))
		.OnClicked(this, &SFlowGraphNode_YapDialogueWidget::OnClicked_TogglePlayerPrompt)
		[
			SNew(STextBlock)
			.TextStyle(FYapEditorStyle::Get(), YapStyles.TextBlockStyle_NodeHeader)
			.Text(this, &SFlowGraphNode_YapDialogueWidget::Text_NodeHeader)
			.ColorAndOpacity(this, &SFlowGraphNode_YapDialogueWidget::ColorAndOpacity_NodeHeader)
		]
	]
	+ SHorizontalBox::Slot()
	.AutoWidth()
	.Padding(8, 0, 8, 0)
	.VAlign(VAlign_Fill)
	[
		SAssignNew(FragmentSequencingButton_Box, SBox)
		.Visibility(Visibility_FragmentSequencingButton())
		.VAlign(VAlign_Fill)
		.WidthOverride(90)
		.Padding(0)
		[
			SAssignNew(FragmentSequencingButton_Button, SButton)
			.ButtonStyle(FAppStyle::Get(), "SimpleButton")
			.ContentPadding(FMargin(2, 1, 2, 1))
			.OnClicked(this, &SFlowGraphNode_YapDialogueWidget::OnClicked_FragmentSequencingButton)
			.ToolTipText(ToolTipText_FragmentSequencingButton())
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.Padding(0, 0, 4, 0)
				.VAlign(VAlign_Center)
				.AutoWidth()
				[
					SAssignNew(FragmentSequencingButton_Image, SImage)
					.ColorAndOpacity(ColorAndOpacity_FragmentSequencingButton())
					.DesiredSizeOverride(FVector2D(16, 16))
					.Image(Image_FragmentSequencingButton())
				]
				+ SHorizontalBox::Slot()
				.Padding(4, 0, 0, 0)
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Center)
				[
					SAssignNew(FragmentSequencingButton_Text, STextBlock)
					.TextStyle(FYapEditorStyle::Get(), YapStyles.TextBlockStyle_NodeSequencing)
					.Text(Text_FragmentSequencingButton())
					.Justification(ETextJustify::Left)
					.ColorAndOpacity(ColorAndOpacity_FragmentSequencingButton())
				]
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
		SAssignNew(DialogueOutputBoxArea, SVerticalBox)
	];

	return Box;
}

TSharedRef<SWidget> SFlowGraphNode_YapDialogueWidget::CreateFragmentBoxes()
{
	bool bFirstFragment = true;

	TSharedRef<SVerticalBox> FragmentBoxes = SNew(SVerticalBox);

	FragmentWidgets.Empty();
	
	for (uint8 FragmentIndex = 0; FragmentIndex < GetFlowYapDialogueNode()->GetNumFragments(); ++FragmentIndex)
	{
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

FText SFlowGraphNode_YapDialogueWidget::Text_NodeHeader() const
{
	if (GetFlowYapDialogueNode()->GetIsPlayerPrompt())
	{
		return INVTEXT("PLAYER PROMPT");
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

	GetFlowYapDialogueNodeMutable()->AddFragment(Index);

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
		.Visibility(this, &SFlowGraphNode_YapDialogueWidget::Visibility_BottomAddFragmentButton)
		.OnClicked(this, &SFlowGraphNode_YapDialogueWidget::OnClicked_BottomAddFragmentButton)
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

EVisibility SFlowGraphNode_YapDialogueWidget::Visibility_FragmentSequencingButton() const
{
	if (GetFlowYapDialogueNode()->GetIsPlayerPrompt())
	{
		return EVisibility::Hidden; // Should be Collapsed but that destroys the parent widget layout for some reason
	}
	
	return (GetFlowYapDialogueNode()->GetNumFragments() > 1) ? EVisibility::Visible : EVisibility::Hidden;
}

FReply SFlowGraphNode_YapDialogueWidget::OnClicked_FragmentSequencingButton()
{
	FFlowYapTransactions::BeginModify(LOCTEXT("DialogueNodeChangeSequencing", "Change dialogue node sequencing setting"), GetFlowYapDialogueNodeMutable());

	GetFlowYapDialogueNodeMutable()->CycleFragmentSequencingMode();

	FragmentSequencingButton_Box->SetVisibility(Visibility_FragmentSequencingButton());

	FragmentSequencingButton_Button->SetToolTipText(ToolTipText_FragmentSequencingButton());
	
	FragmentSequencingButton_Image->SetImage(Image_FragmentSequencingButton());
	FragmentSequencingButton_Image->SetColorAndOpacity(ColorAndOpacity_FragmentSequencingButton());

	FragmentSequencingButton_Text->SetText(Text_FragmentSequencingButton());
	FragmentSequencingButton_Text->SetColorAndOpacity(ColorAndOpacity_FragmentSequencingButton());
	
	FFlowYapTransactions::EndModify();
	
	return FReply::Handled();
}

const FSlateBrush* SFlowGraphNode_YapDialogueWidget::Image_FragmentSequencingButton() const
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

FText SFlowGraphNode_YapDialogueWidget::ToolTipText_FragmentSequencingButton() const
{
	switch (GetFlowYapDialogueNode()->GetMultipleFragmentSequencing())
	{
	case EFlowYapMultipleFragmentSequencing::Sequential:
		{
			return LOCTEXT("DialogueNodeSequence", "Starting from the top, attempt to run all fragments");
		}
	case EFlowYapMultipleFragmentSequencing::SelectOne:
		{
			return LOCTEXT("DialogueNodeSequence", "Starting from the top, attempt to run all fragments, stopping if one succeeds");
		}
	default:
		{
			return LOCTEXT("DialogueNodeSequence", "ERROR");
		}
	}
}

FSlateColor SFlowGraphNode_YapDialogueWidget::ColorAndOpacity_FragmentSequencingButton() const
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

EVisibility SFlowGraphNode_YapDialogueWidget::Visibility_BottomAddFragmentButton() const
{
	for (TSharedPtr<SFlowGraphNode_YapFragmentWidget> FragmentWidget : FragmentWidgets)
	{
		if (FragmentWidget->IsBeingEdited())
		{
			return EVisibility::Hidden;
		}
	}
	return IsHovered() ? EVisibility::Visible : EVisibility::Hidden;
}


FReply SFlowGraphNode_YapDialogueWidget::OnClicked_BottomAddFragmentButton()
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
	.BorderImage(FYapEditorStyle::GetImageBrush(YapBrushes.Box_SolidWhite_Deburred))
	.BorderBackgroundColor(YapColor::DeepGray)
	.ColorAndOpacity(YapColor::White)//this, &SFlowGraphNode_YapFragmentWidget::FragmentTagPreview_ColorAndOpacity)
	.VAlign(VAlign_Center)
	.HAlign(HAlign_Center)
	[
		SNew(STextBlock)
		.Text(this, &SFlowGraphNode_YapDialogueWidget::Text_DialogueTagPreview)
		.IsEnabled(false)
		.Font(FCoreStyle::GetDefaultFontStyle("Bold", 8))
	];
}

FText SFlowGraphNode_YapDialogueWidget::Text_DialogueTagPreview() const
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

EVisibility SFlowGraphNode_YapDialogueWidget::Visibility_DialogueTagPreview() const
{
	return GetFlowYapDialogueNode()->DialogueTag.IsValid() ? EVisibility::Visible : EVisibility::Collapsed;
}

EVisibility SFlowGraphNode_YapDialogueWidget::Visibility_ConditionWidgets() const
{
	return (GetFlowYapDialogueNode()->GetConditions().Num() > 0) ? EVisibility::Visible : EVisibility::Hidden;
}

TSharedRef<SWidget> SFlowGraphNode_YapDialogueWidget::CreateConditionWidgets()
{
	TSharedRef<SScrollBox> Box = SNew(SScrollBox)
	.Visibility(this, &SFlowGraphNode_YapDialogueWidget::Visibility_ConditionWidgets)
	.ScrollBarVisibility(EVisibility::Collapsed)
	.ConsumeMouseWheel(EConsumeMouseWheel::Always)
	.AllowOverscroll(EAllowOverscroll::No)
	.AnimateWheelScrolling(true)
	.Orientation(Orient_Horizontal);

	for (UFlowYapCondition* Condition : GetFlowYapDialogueNode()->GetConditions())
	{
		Box->AddSlot()
		.Padding(0, 0, 4, 0)
		[
			FFlowYapWidgetHelper::CreateConditionWidget(GetFlowYapDialogueNodeMutable(), Condition)
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
	.BorderImage(FYapEditorStyle::GetImageBrush(YapBrushes.Box_SolidWhite_Deburred))
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
	
	if (FocusedFragmentIndex.IsSet())
	{
		FocusedFragmentIndex = FocusedFragmentIndex.GetValue() + By;
	}
}

void SFlowGraphNode_YapDialogueWidget::SetSelected()
{
	TSharedPtr<SFlowGraphEditor> GraphEditor = FFlowGraphUtils::GetFlowGraphEditor(this->FlowGraphNode->GetGraph());

	if (!GraphEditor)
	{
		return;
	}

	GraphEditor->SelectSingleNode(GraphNode);
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
	return Cast<UFlowNode_YapDialogue>(FlowGraphNode->GetFlowNodeBase());
}

const UFlowNode_YapDialogue* SFlowGraphNode_YapDialogueWidget::GetFlowYapDialogueNode() const
{
	return Cast<UFlowNode_YapDialogue>(FlowGraphNode->GetFlowNodeBase());
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

	if (GetFlowYapDialogueNode()->SelectedCondition && !GetIsSelected())
	{
		///GetFlowYapDialogueNodeMutable()->SelectedCondition = nullptr;
	}
	
	// TODO cleanup
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
	
	if (PinName == FName("In"))
	{
		AddInPin(PinToAdd);
		InputPins.Add(PinToAdd);
	}
	else if (PinName == FName("Out"))
	{
		AddOutPin(PinToAdd);
		OutputPins.Add(PinToAdd);
	}
	else if (PinName == FName("Bypass"))
	{
		AddBypassPin(PinToAdd);
		OutputPins.Add(PinToAdd);
	}
	else
	{
		// TODO
		const FGuid* FragmentGuid = nullptr;// GetFlowYapDialogueNode()->FragmentPinMap.Find(PinName);

		if (!FragmentGuid)
		{
			UE_LOGFMT(FlowYap, Warning, "Could not find fragment for pin: {0}", PinName);
			return;
		}
		
		int32 Index = GetFlowYapDialogueNode()->GetFragmentIndex(*FragmentGuid);

		if (Index < 0)
		{
			// TODO this still happens upon copy pasting a node.
			UE_LOGFMT(FlowYap, Warning, "Invalid output pin name: {0}", PinName);
			return;
		}

		AddFragmentPin(PinToAdd, Index);
		OutputPins.Add(PinToAdd);
	}
}

void SFlowGraphNode_YapDialogueWidget::AddInPin(const TSharedRef<SGraphPin> PinToAdd)
{
	const UEdGraphPin* PinObj = PinToAdd->GetPinObj();
		
	DialogueInputBoxArea->AddSlot()
	[
		PinToAdd
	];

	PinToAdd->SetToolTipText(LOCTEXT("Dialogue", "In"));
}

void SFlowGraphNode_YapDialogueWidget::AddOutPin(const TSharedRef<SGraphPin>& PinToAdd)
{
	DialogueOutputBoxArea->AddSlot()
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

void SFlowGraphNode_YapDialogueWidget::AddFragmentPin(const TSharedRef<SGraphPin>& PinToAdd, int32 FragmentIndex)
{
	const UEdGraphPin* PinObj = PinToAdd->GetPinObj();
	
	EVerticalAlignment Alignment = VAlign_Bottom;
	int BottomPadding; 

	FName PinName = PinObj->GetFName();

	FString PinNameString = PinName.ToString();
	
	if (PinNameString.StartsWith("Start_"))
	{
		PinToAdd->SetColorAndOpacity(PinToAdd->IsConnected() ? YapColor::White : YapColor::DarkRed);
		PinToAdd->SetToolTipText(INVTEXT("On Start"));
		BottomPadding = 4;
	}
	else if (PinNameString.StartsWith("End_"))
	{
		PinToAdd->SetColorAndOpacity(PinToAdd->IsConnected() ? YapColor::White : YapColor::DarkRed);
		PinToAdd->SetToolTipText(INVTEXT("On End, runs before any end-padding time begins"));
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

	TSharedPtr<SVerticalBox> Box = FragmentWidget->GetPinContainer();

	Box->AddSlot()
	.HAlign(HAlign_Right)
	.VAlign(Alignment)
	.Padding(0, 0, 4, BottomPadding)
	[
		PinToAdd
	];
}

void SFlowGraphNode_YapDialogueWidget::CreatePinWidgets()
{	
	TArray<TSet<FFlowPin>> FragmentPins;
	FragmentPins.SetNum(GetFlowYapDialogueNode()->GetFragments().Num());

	TSet<FFlowPin> OptionalPins;
	
	TMap<FFlowPin, int32> PinFragmentIndices;

	for (int32 i = 0; i < GetFlowYapDialogueNode()->GetFragments().Num(); ++i)
	{
		const FFlowYapFragment& Fragment = GetFlowYapDialogueNode()->GetFragments()[i];

		if (Fragment.GetShowOnStartPin())
		{
			FFlowPin StartPin = Fragment.GetStartPin();
			
			FragmentPins[i].Add(StartPin);
			PinFragmentIndices.Add(StartPin, i);
			OptionalPins.Add(StartPin);
		}

		if (Fragment.GetShowOnEndPin())
		{
			FFlowPin EndPin = Fragment.GetEndPin();

			FragmentPins[i].Add(EndPin);
			PinFragmentIndices.Add(EndPin, i);
			OptionalPins.Add(EndPin);
		}

		if (GetFlowYapDialogueNode()->GetIsPlayerPrompt())
		{
			FFlowPin PromptPin = Fragment.GetPromptPin();

			FragmentPins[i].Add(PromptPin);
			PinFragmentIndices.Add(PromptPin, i);
		}
	}
	
	// Create Pin widgets for each of the pins.
	for (int32 PinIndex = 0; PinIndex < GraphNode->Pins.Num(); ++PinIndex)
	{
		UEdGraphPin* Pin = GraphNode->Pins[PinIndex];

		if ( !ensureMsgf(Pin->GetOuter() == GraphNode
			, TEXT("Graph node ('%s' - %s) has an invalid %s pin: '%s'; (with a bad %s outer: '%s'); skiping creation of a widget for this pin.")
			, *GraphNode->GetNodeTitle(ENodeTitleType::ListView).ToString()
			, *GraphNode->GetPathName()
			, (Pin->Direction == EEdGraphPinDirection::EGPD_Input) ? TEXT("input") : TEXT("output")
			,  Pin->PinFriendlyName.IsEmpty() ? *Pin->PinName.ToString() : *Pin->PinFriendlyName.ToString()
			,  Pin->GetOuter() ? *Pin->GetOuter()->GetClass()->GetName() : TEXT("UNKNOWN")
			,  Pin->GetOuter() ? *Pin->GetOuter()->GetPathName() : TEXT("NULL")) )
		{
			continue;
		}
		
		const TSharedPtr<SGraphPin> NewPin = SNew(SFlowGraphPinExec, Pin);

		const TSharedRef<SGraphPin>& NewPinRef = NewPin.ToSharedRef();

		if (OptionalPins.Contains(Pin->GetFName()))
		{
			if (TSharedPtr<SLayeredImage> PinImage = StaticCastSharedPtr<SLayeredImage>(NewPin->GetPinImageWidget()))
			{
				PinImage->SetLayerBrush(0, FYapEditorStyle::GetImageBrush(YapBrushes.Pin_OptionalOutput));
			}
		}
		
		NewPinRef->SetOwner(SharedThis(this));
		NewPinRef->SetShowLabel(false);

		const bool bAdvancedParameter = Pin && Pin->bAdvancedView;
		if (bAdvancedParameter)
		{
			NewPinRef->SetVisibility(TAttribute<EVisibility>(NewPinRef, &SGraphPin::IsPinVisibleAsAdvanced));
		}

		TSharedPtr<SVerticalBox> OutputBox = DialogueOutputBoxArea;

		int32* FragmentIndex = PinFragmentIndices.Find(Pin->GetFName());

		if (FragmentIndex)
		{
			OutputBox = FragmentWidgets[*FragmentIndex]->PinContainer;
		}
		
		if (NewPinRef->GetDirection() == EEdGraphPinDirection::EGPD_Input)
		{
			DialogueInputBoxArea->AddSlot()
			.AutoHeight()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Center)
			.Padding(Settings->GetInputPinPadding())
			[
				NewPinRef
			];
			
			InputPins.Add(NewPinRef);
		}
		else // Direction == EEdGraphPinDirection::EGPD_Output
		{
			OutputBox->AddSlot()
			.AutoHeight()
			.HAlign(HAlign_Right)
			.VAlign(VAlign_Center)
			.Padding(Settings->GetOutputPinPadding())
			[
				NewPinRef
			];
			
			OutputPins.Add(NewPinRef);
		}
	}
}

void SFlowGraphNode_YapDialogueWidget::CreateOptionalPinWidget(UEdGraphPin* Pin)
{
	const bool bShowPin = ShouldPinBeHidden(Pin);

	if (bShowPin)
	{
		TSharedPtr<SGraphPin> NewPin = CreatePinWidget(Pin);
		check(NewPin.IsValid());
		NewPin->SetColorAndOpacity(TAttribute<FLinearColor>::CreateSP(this, &SFlowGraphNode_YapDialogueWidget::TestColor));

		AddPin(NewPin.ToSharedRef());
	}
}

const FFlowYapFragment& SFlowGraphNode_YapDialogueWidget::GetFragment(uint8 FragmentIndex) const
{
	return GetFlowYapDialogueNode()->GetFragmentByIndex(FragmentIndex);
}

FFlowYapFragment& SFlowGraphNode_YapDialogueWidget::GetFragmentMutable(uint8 FragmentIndex)
{
	return GetFlowYapDialogueNodeMutable()->GetFragmentByIndexMutable(FragmentIndex);
}

FLinearColor SFlowGraphNode_YapDialogueWidget::TestColor() const
{
	if (GetFlowYapDialogueNode()->ActivationLimitsMet())
	{
		return YapColor::Red;
	}
	
	return YapColor::White;
}

#undef LOCTEXT_NAMESPACE
