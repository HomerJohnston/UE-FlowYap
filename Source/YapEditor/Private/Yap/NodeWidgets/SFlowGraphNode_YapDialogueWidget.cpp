// Copyright Ghost Pepper Games, Inc. All Rights Reserved.

#include "Yap/NodeWidgets/SFlowGraphNode_YapDialogueWidget.h"

#include "FlowEditorStyle.h"
#include "NodeFactory.h"
#include "PropertyCustomizationHelpers.h"
#include "Graph/FlowGraphEditor.h"
#include "Graph/FlowGraphSettings.h"
#include "Graph/FlowGraphUtils.h"
#include "Logging/StructuredLog.h"
#include "Math/BigInt.h"
#include "Widgets/SCanvas.h"
#include "Widgets/SVirtualWindow.h"
#include "Widgets/Images/SLayeredImage.h"
#include "Yap/YapBit.h"
#include "Yap/YapColors.h"
#include "Yap/YapCondition.h"
#include "Yap/YapEditorSettings.h"
#include "Yap/YapEditorSubsystem.h"
#include "Yap/YapFragment.h"
#include "Yap/YapInputTracker.h"
#include "Yap/YapLog.h"
#include "Yap/YapProjectSettings.h"
#include "Yap/YapTransactions.h"
#include "Yap/YapEditorStyle.h"
#include "Yap/GraphNodes/FlowGraphNode_YapDialogue.h"
#include "Yap/Helpers/YapWidgetHelper.h"
#include "Yap/Nodes/FlowNode_YapDialogue.h"
#include "Yap/NodeWidgets/SConditionEntryWidget.h"
#include "Yap/NodeWidgets/SConditionDetailsViewWidget.h"
#include "Yap/NodeWidgets/SYapGraphPinExec.h"

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

	DialogueButtonsColor = YapColor::DarkGray;

	ConnectedBypassPinColor = YapColor::LightBlue;
	DisconnectedBypassPinColor = YapColor::Red;
	
	ConnectedFragmentPinColor = YapColor::White;
	DisconnectedFragmentPinColor = YapColor::Red;
	
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

EVisibility SFlowGraphNode_YapDialogueWidget::Visibility_InterruptibleToggleIconOff() const
{
	switch (GetFlowYapDialogueNode()->GetInterruptibleSetting())
	{
		case EFlowYapInterruptible::UseProjectDefaults:
		{
			return UYapProjectSettings::Get()->GetDialogueInterruptibleByDefault() ? EVisibility::Collapsed : EVisibility::Visible;
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

void SFlowGraphNode_YapDialogueWidget::OnTextCommitted_DialogueActivationLimit(const FText& Text, ETextCommit::Type Arg)
{
	FYapTransactions::BeginModify(LOCTEXT("Dialogue", "Change Dialogue Activation Limit"), GetFlowYapDialogueNodeMutable());

	GetFlowYapDialogueNodeMutable()->SetNodeActivationLimit(FCString::Atoi(*Text.ToString()));

	FYapTransactions::EndModify();
}

FGameplayTag SFlowGraphNode_YapDialogueWidget::Value_DialogueTag() const
{
	return GetFlowYapDialogueNode()->GetDialogueTag();
}

void SFlowGraphNode_YapDialogueWidget::OnTagChanged_DialogueTag(FGameplayTag GameplayTag)
{
	if (GetFlowYapDialogueNodeMutable()->DialogueTag == GameplayTag)
	{
		return;
	}
	
	FYapTransactions::BeginModify(LOCTEXT("Fragment", "Change Fragment Tag"), GetFlowYapDialogueNodeMutable());

	GetFlowYapDialogueNodeMutable()->DialogueTag = GameplayTag;

	GetFlowYapDialogueNodeMutable()->InvalidateFragmentTags();

	FYapTransactions::EndModify();

	UpdateGraphNode();
}

int32 SFlowGraphNode_YapDialogueWidget::GetMaxNodeWidth() const
{
	return FMath::Max(YAP_MIN_NODE_WIDTH + UYapEditorSettings::Get()->GetPortraitSize(), YAP_DEFAULT_NODE_WIDTH + UYapEditorSettings::Get()->GetDialogueWidthAdjustment());
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
	.IsEnabled_Lambda([]() { return GEditor->PlayWorld == nullptr; })
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.HAlign(HAlign_Left)
		.AutoWidth()
		.Padding(-8, -5, 16, -6)
		[
			SNew(SActivationCounterWidget, FOnTextCommitted::CreateSP(this, &SFlowGraphNode_YapDialogueWidget::OnTextCommitted_DialogueActivationLimit))
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
			SNew(SBox)
			[
				FYapWidgetHelper::CreateFilteredTagWidget(
					TAttribute<FGameplayTag>::CreateSP(this, &SFlowGraphNode_YapDialogueWidget::Value_DialogueTag),
					GetDefault<UYapProjectSettings>()->DialogueTagsParent.ToString(),
					TDelegate<void(const FGameplayTag)>::CreateSP(this, &SFlowGraphNode_YapDialogueWidget::OnTagChanged_DialogueTag),
					INVTEXT("Dialogue tag"))
			]
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
				.IsChecked(this, &SFlowGraphNode_YapDialogueWidget::IsChecked_InterruptibleToggle)
				.OnCheckStateChanged(this, &SFlowGraphNode_YapDialogueWidget::OnCheckStateChanged_InterruptibleToggle)
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
							.ColorAndOpacity(this, &SFlowGraphNode_YapDialogueWidget::ColorAndOpacity_InterruptibleToggleIcon)
							.DesiredSizeOverride(FVector2D(16, 16))
							.Image(FAppStyle::Get().GetBrush("Icons.Rotate180"))
						]
						+ SOverlay::Slot()
						[
							SNew(SImage)
							.ColorAndOpacity(this, &SFlowGraphNode_YapDialogueWidget::ColorAndOpacity_InterruptibleToggleIcon)
							.DesiredSizeOverride(FVector2D(16, 16))
							.Image(FAppStyle::Get().GetBrush("SourceControl.StatusIcon.Off"))
							.Visibility(this, &SFlowGraphNode_YapDialogueWidget::Visibility_InterruptibleToggleIconOff)
						]
					]
				]
			]
		]
	];
	
	return Widget;
}

ECheckBoxState SFlowGraphNode_YapDialogueWidget::IsChecked_InterruptibleToggle() const
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

void SFlowGraphNode_YapDialogueWidget::OnCheckStateChanged_InterruptibleToggle(ECheckBoxState CheckBoxState)
{
	FYapTransactions::BeginModify(LOCTEXT("YapDialogue", "Toggle Interruptible"), GetFlowYapDialogueNodeMutable());

	if (GEditor->GetEditorSubsystem<UYapEditorSubsystem>()->GetInputTracker()->GetControlPressed())
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

	FYapTransactions::EndModify();
}

FSlateColor SFlowGraphNode_YapDialogueWidget::ColorAndOpacity_InterruptibleToggleIcon() const
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
	.IsEnabled_Lambda([]() { return GEditor->PlayWorld == nullptr; })
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

FSlateColor SFlowGraphNode_YapDialogueWidget::ColorAndOpacity_NodeHeaderButton() const
{
	if (GetFlowYapDialogueNode()->ActivationLimitsMet() && GetFlowYapDialogueNode()->GetActivationState() != EFlowNodeState::Active)
	{
		return YapColor::Red;
	}

	return YapColor::DarkGray;
}

FSlateColor SFlowGraphNode_YapDialogueWidget::ColorAndOpacity_NodeHeader() const
{
	if (GetFlowYapDialogueNode()->ActivationLimitsMet() && GetFlowYapDialogueNode()->GetActivationState() != EFlowNodeState::Active)
	{
		//return YapColor::Red;
	}

	return YapColor::White;
}

FText SFlowGraphNode_YapDialogueWidget::Text_FragmentSequencingButton() const
{
	switch (GetFlowYapDialogueNode()->GetMultipleFragmentSequencing())
	{
		case EFlowYapMultipleFragmentSequencing::RunAll:
		{
			return INVTEXT("Run all");
		}
		case EFlowYapMultipleFragmentSequencing::RunUntilFailure:
		{
			return INVTEXT("Run til failure");
		}
		case EFlowYapMultipleFragmentSequencing::SelectOne:
		{
			return INVTEXT("Select one");
		}
		default:
		{
			return INVTEXT("Error");
		}
	}
}

FReply SFlowGraphNode_YapDialogueWidget::OnClicked_TogglePlayerPrompt()
{
	FYapTransactions::BeginModify(LOCTEXT("DialogueNode", "Toggle Player Prompt"), GetFlowYapDialogueNodeMutable());
	
	GetFlowYapDialogueNodeMutable()->bIsPlayerPrompt = !GetFlowYapDialogueNode()->bIsPlayerPrompt;

	GetFlowYapDialogueNodeMutable()->ForceReconstruction();
	
	FYapTransactions::EndModify();	

	return FReply::Handled();
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
	.Padding(-2, 0, 0, 0)
	[
		SNew(SButton)
		.ButtonStyle(FYapEditorStyle::Get(), YapStyles.ButtonStyle_HeaderButton)
		.ContentPadding(FMargin(4, 0, 4, 0))
		.ButtonColorAndOpacity(this, &SFlowGraphNode_YapDialogueWidget::ColorAndOpacity_NodeHeaderButton)
		.ForegroundColor(this, &SFlowGraphNode_YapDialogueWidget::ColorAndOpacity_NodeHeader)
		.OnClicked(this, &SFlowGraphNode_YapDialogueWidget::OnClicked_TogglePlayerPrompt)
		.ToolTipText(INVTEXT("Toggle between player prompt or normal speech"))
		[
			SNew(STextBlock)
			.TextStyle(FYapEditorStyle::Get(), YapStyles.TextBlockStyle_NodeHeader)
			.Text(this, &SFlowGraphNode_YapDialogueWidget::Text_NodeHeader)
			.ColorAndOpacity(FSlateColor::UseForeground())
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
		.WidthOverride(110)
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
		SAssignNew(DialogueOutputBoxArea, SBox)
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
		.ColorAndOpacity(this, &SFlowGraphNode_YapDialogueWidget::ColorAndOpacity_FragmentSeparator)
	];

	return Box;
}


EVisibility SFlowGraphNode_YapDialogueWidget::FragmentSeparator_Visibility() const
{
	return GetIsSelected() ? EVisibility::Visible : EVisibility::Hidden;
}

FSlateColor SFlowGraphNode_YapDialogueWidget::ColorAndOpacity_FragmentSeparator() const
{
	return YapColor::Gray; 
}

FReply SFlowGraphNode_YapDialogueWidget::FragmentSeparator_OnClicked(uint8 Index)
{
	FYapTransactions::BeginModify(LOCTEXT("DialogueAddFragment", "Add Fragment"), GetFlowYapDialogueNodeMutable());

	GetFlowYapDialogueNodeMutable()->AddFragment(Index);

	UpdateGraphNode();

	FYapTransactions::EndModify();
		
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
	.IsEnabled_Lambda([]() { return GEditor->PlayWorld == nullptr; })
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
	.IsEnabled_Lambda([]() { return GEditor->PlayWorld == nullptr; })
	+ SHorizontalBox::Slot()
	.HAlign(HAlign_Fill)
	.VAlign(VAlign_Fill)
	.Padding(31, 4, 7, 4)
	[
		SNew(SBox)
		.Visibility(this, &SFlowGraphNode_YapDialogueWidget::Visibility_BottomAddFragmentButton)
		[
			SNew(SButton)
			.HAlign(HAlign_Center)
			.ButtonStyle(FAppStyle::Get(), "SimpleButton")
			.ToolTipText(LOCTEXT("DialogueAddFragment_Tooltip", "Add Fragment"))
			.OnClicked(this, &SFlowGraphNode_YapDialogueWidget::OnClicked_BottomAddFragmentButton)
			.ContentPadding(0)
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
	]
	+ SHorizontalBox::Slot()
	.AutoWidth()
	.HAlign(HAlign_Right)
	.Padding(0, 2, 1, 2)
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
	FYapTransactions::BeginModify(LOCTEXT("DialogueNodeChangeSequencing", "Change dialogue node sequencing setting"), GetFlowYapDialogueNodeMutable());

	GetFlowYapDialogueNodeMutable()->CycleFragmentSequencingMode();

	FragmentSequencingButton_Box->SetVisibility(Visibility_FragmentSequencingButton());

	FragmentSequencingButton_Button->SetToolTipText(ToolTipText_FragmentSequencingButton());
	
	FragmentSequencingButton_Image->SetImage(Image_FragmentSequencingButton());
	FragmentSequencingButton_Image->SetColorAndOpacity(ColorAndOpacity_FragmentSequencingButton());

	FragmentSequencingButton_Text->SetText(Text_FragmentSequencingButton());
	FragmentSequencingButton_Text->SetColorAndOpacity(ColorAndOpacity_FragmentSequencingButton());

	UpdateGraphNode();
	
	FYapTransactions::EndModify();
	
	return FReply::Handled();
}

const FSlateBrush* SFlowGraphNode_YapDialogueWidget::Image_FragmentSequencingButton() const
{
	switch (GetFlowYapDialogueNode()->GetMultipleFragmentSequencing())
	{
		case EFlowYapMultipleFragmentSequencing::RunAll:
		{
			return FAppStyle::Get().GetBrush("Icons.SortDown"); 
		}
		case EFlowYapMultipleFragmentSequencing::RunUntilFailure:
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
		case EFlowYapMultipleFragmentSequencing::RunAll:
		{
			return LOCTEXT("DialogueNodeSequence", "Starting from the top, attempt to run all fragments");
		}
		case EFlowYapMultipleFragmentSequencing::RunUntilFailure:
		{
			return LOCTEXT("DialogueNodeSequence", "Starting from the top, attempt to run all fragments, stopping if one fails"); 
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
		case EFlowYapMultipleFragmentSequencing::RunAll:
		{
			return YapColor::LightBlue;
		}
		case EFlowYapMultipleFragmentSequencing::RunUntilFailure:
		{
			return YapColor::LightGreen;
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
	if (GEditor->PlayWorld)
	{
		return EVisibility::Hidden;
	}

	return EVisibility::Visible;
}


FReply SFlowGraphNode_YapDialogueWidget::OnClicked_BottomAddFragmentButton()
{
	FYapTransactions::BeginModify(LOCTEXT("DialogueAddFragment", "Add Fragment"), GetFlowYapDialogueNodeMutable());
	
	GetFlowYapDialogueNodeMutable()->AddFragment();

	UpdateGraphNode();

	FYapTransactions::EndModify();
	
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
	FText Text = FText::FromString(UYapProjectSettings::GetTrimmedGameplayTagString(EYap_TagFilter::Prompts, GetFlowYapDialogueNode()->GetDialogueTag()));

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

void SFlowGraphNode_YapDialogueWidget::OnClick_ConditionEntryButton(UYapCondition* Condition, TSharedRef<SConditionEntryWidget> ConditionEntryWidget, int32 ConditionIndexInArray)
{
	if (ConditionDetailsPane != nullptr && EditedCondition == Condition)
	{
		ConditionDetailsPane = nullptr;
		EditedCondition = nullptr;
		return;
	}
	
	
	if (ConditionDetailsPane == nullptr)
	{
		ConditionDetailsPane = SNew(SConditionDetailsViewWidget)
			.Dialogue(GetFlowYapDialogueNodeMutable())
			.Condition(Condition)
			.ConditionIndexInArray(ConditionIndexInArray);
	}

	EditedCondition = Condition;

	FVector2D LTA = ConditionEntryWidget->GetPaintSpaceGeometry().LocalToAbsolute(FVector2D(0, 0));
	FVector2D OwnerLTA = GetPaintSpaceGeometry().LocalToAbsolute(FVector2D(0, 0));
	
	ConditionDetailsPaneOffset = LTA - OwnerLTA;
}

bool SFlowGraphNode_YapDialogueWidget::IsEnabled_ConditionWidgetsScrollBox() const
{
	return (EditedCondition != nullptr);
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
	
	for (int32 i = 0; i < GetFlowYapDialogueNode()->GetConditions().Num(); ++i)
	{
		UYapCondition* Condition = GetFlowYapDialogueNode()->GetConditions()[i];
		TSharedRef<SConditionEntryWidget> Widget = SNew(SConditionEntryWidget)
			.Condition(Condition)
			.DialogueNode(GetFlowYapDialogueNodeMutable());

		Widget->OnClick.BindSP(this, &SFlowGraphNode_YapDialogueWidget::OnClick_ConditionEntryButton, i);
		
		Box->AddSlot()
		.Padding(0, 0, 4, 0)
		[
			Widget
		];
	}
	
	return Box;
}

TArray<FOverlayWidgetInfo> SFlowGraphNode_YapDialogueWidget::GetOverlayWidgets(bool bSelected, const FVector2D& WidgetSize) const
{
	TArray<FOverlayWidgetInfo> Widgets;

	if (ConditionDetailsPane)
	{
		FOverlayWidgetInfo Info;
		Info.OverlayOffset = ConditionDetailsPaneOffset + FVector2D(0, 16);
		Info.Widget = ConditionDetailsPane;

		Widgets.Add(Info);	
	}

	return Widgets;
}

// ------------------------------------------
// PUBLIC API & THEIR HELPERS

TSharedRef<SWidget> SFlowGraphNode_YapDialogueWidget::CreateConditionWidget(const UYapCondition* Condition)
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
	
	// TODO cleanup
	bIsSelected = GraphEditor->GetSelectedFlowNodes().Contains(FlowGraphNode);

	bShiftPressed = GEditor->GetEditorSubsystem<UYapEditorSubsystem>()->GetInputTracker()->GetShiftPressed();
	bCtrlPressed = GEditor->GetEditorSubsystem<UYapEditorSubsystem>()->GetInputTracker()->GetControlPressed();
	
	if (bIsSelected && bShiftPressed && !bKeyboardFocused)
	{
		bShiftHooked = true;
	}
	
	if (!bIsSelected)
	{
		bShiftHooked = false;
		FocusedFragmentIndex.Reset();
		bKeyboardFocused = false;
		ConditionDetailsPane = nullptr;
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

void SFlowGraphNode_YapDialogueWidget::CreatePinWidgets()
{	
	TArray<TSet<FFlowPin>> FragmentPins;
	FragmentPins.SetNum(GetFlowYapDialogueNode()->GetFragments().Num());

	TSet<FFlowPin> OptionalPins;
	
	TMap<FFlowPin, int32> FragmentPinsFragmentIndex;

	TSet<FFlowPin> PromptOutPins;
	
	for (int32 i = 0; i < GetFlowYapDialogueNode()->GetFragments().Num(); ++i)
	{
		const FYapFragment& Fragment = GetFlowYapDialogueNode()->GetFragments()[i];

		if (Fragment.UsesStartPin())
		{
			FFlowPin StartPin = Fragment.GetStartPin();
			
			FragmentPins[i].Add(StartPin);
			FragmentPinsFragmentIndex.Add(StartPin, i);
			OptionalPins.Add(StartPin);
		}

		if (Fragment.UsesEndPin())
		{
			FFlowPin EndPin = Fragment.GetEndPin();

			FragmentPins[i].Add(EndPin);
			FragmentPinsFragmentIndex.Add(EndPin, i);
			OptionalPins.Add(EndPin);
		}

		if (GetFlowYapDialogueNode()->GetIsPlayerPrompt())
		{
			FFlowPin PromptPin = Fragment.GetPromptPin();

			FragmentPins[i].Add(PromptPin);
			FragmentPinsFragmentIndex.Add(PromptPin, i);
			PromptOutPins.Add(PromptPin);
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

		const TSharedRef<SGraphPin> NewPinRef = OptionalPins.Contains(Pin->GetFName()) ? SNew(SYapGraphPinExec, Pin) : FNodeFactory::CreatePinWidget(Pin).ToSharedRef();

		NewPinRef->SetOwner(SharedThis(this));
		NewPinRef->SetShowLabel(false);
		NewPinRef->SetPadding(FMargin(-4, -2, 2, -2));
		NewPinRef->SetColorAndOpacity(YapColor::White);

		FString PinToolTIpText = Pin->GetName();

		int32 UnderscoreIndex;
		if (PinToolTIpText.FindLastChar('_', UnderscoreIndex))
		{
			PinToolTIpText = PinToolTIpText.Left(UnderscoreIndex);
		}
		
		NewPinRef->SetToolTipText(FText::FromString(PinToolTIpText));
		
		if (OptionalPins.Contains(Pin->GetFName()))
		{
			NewPinRef->SetPadding(FMargin(-4, -2, 16, -2));
		}

		NewPinRef->SetHAlign(HAlign_Right);

		const bool bAdvancedParameter = Pin && Pin->bAdvancedView;
		if (bAdvancedParameter)
		{
			NewPinRef->SetVisibility(TAttribute<EVisibility>(NewPinRef, &SGraphPin::IsPinVisibleAsAdvanced));
		}

		TSharedPtr<SBox> PinBox = nullptr;

		if (Pin->GetFName() == "Out")
		{
			PinBox = DialogueOutputBoxArea;
			NewPinRef->SetColorAndOpacity(YapColor::White);
			NewPinRef->SetPadding(FMargin(-4, -2, 2, -2));
		}
		else if (Pin->GetFName() == "Bypass")
		{
			PinBox = BypassOutputBox;
			NewPinRef->SetColorAndOpacity(NewPinRef->IsConnected() ? ConnectedBypassPinColor : DisconnectedBypassPinColor);
			NewPinRef->SetPadding(FMargin(-4, -2, 2, -2));
		}
		else if (int32* FragmentIndex = FragmentPinsFragmentIndex.Find(Pin->GetFName()))
		{
			PinBox = FragmentWidgets[*FragmentIndex]->GetPinContainer(Pin->GetFName());
			NewPinRef->SetColorAndOpacity(NewPinRef->IsConnected() ? ConnectedFragmentPinColor : DisconnectedFragmentPinColor);
		}
		else if (NewPinRef->GetDirection() == EEdGraphPinDirection::EGPD_Input)
		{
			PinBox = DialogueInputBoxArea;
			NewPinRef->SetPadding(FMargin(4, -2, 0, -2));
		}

		if (PinBox.IsValid())
		{
			PinBox->SetContent(NewPinRef);

			if (NewPinRef->GetDirection() == EEdGraphPinDirection::EGPD_Input)
			{
				InputPins.Add(NewPinRef);
			}
			else
			{
				OutputPins.Add(NewPinRef);
			}
		}
		else
		{
			// TODO error handling
			UE_LOG(LogYap, Warning, TEXT("Could not find pin box for pin %s"), *Pin->GetFName().ToString());
		}
	}
}

const FYapFragment& SFlowGraphNode_YapDialogueWidget::GetFragment(uint8 FragmentIndex) const
{
	return GetFlowYapDialogueNode()->GetFragmentByIndex(FragmentIndex);
}

FYapFragment& SFlowGraphNode_YapDialogueWidget::GetFragmentMutable(uint8 FragmentIndex)
{
	return GetFlowYapDialogueNodeMutable()->GetFragmentByIndexMutable(FragmentIndex);
}

void SFlowGraphNode_YapDialogueWidget::CreateStandardPinWidget(UEdGraphPin* Pin)
{
	const bool bShowPin = ShouldPinBeHidden(Pin);

	if (bShowPin)
	{
		TSharedPtr<SGraphPin> NewPin = FNodeFactory::CreatePinWidget(Pin);
		check(NewPin.IsValid());

		this->AddPin(NewPin.ToSharedRef());
	}
}

#undef LOCTEXT_NAMESPACE