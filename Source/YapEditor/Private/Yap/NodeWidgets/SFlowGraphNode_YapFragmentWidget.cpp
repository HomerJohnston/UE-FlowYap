// Copyright Ghost Pepper Games, Inc. All Rights Reserved.


#define LOCTEXT_NAMESPACE "FlowYap"
#include "Yap/NodeWidgets/SFlowGraphNode_YapFragmentWidget.h"

#include "PropertyCustomizationHelpers.h"
#include "Graph/FlowGraphEditor.h"
#include "Graph/FlowGraphUtils.h"
#include "Slate/DeferredCleanupSlateBrush.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "Widgets/Input/SSlider.h"
#include "Widgets/Notifications/SProgressBar.h"
#include "Yap/YapCharacter.h"
#include "Yap/YapColors.h"
#include "Yap/YapEditorSubsystem.h"
#include "Yap/YapFragment.h"
#include "Yap/YapProjectSettings.h"
#include "Yap/YapTransactions.h"
#include "Yap/YapUtil.h"
#include "Yap/YapEditorStyle.h"
#include "Yap/Enums/YapErrorLevel.h"
#include "Yap/Nodes/FlowNode_YapDialogue.h"
#include "Yap/NodeWidgets/SFlowGraphNode_YapDialogueWidget.h"
#include "Yap/SlateWidgets/SGameplayTagComboFiltered.h"
#include "Yap/YapBitReplacement.h"
#include "Yap/YapCondition.h"
#include "Yap/Helpers/YapWidgetHelper.h"
#include "Yap/Helpers/SYapTextPropertyEditableTextBox.h"
#include "Yap/Helpers/YapEditableTextPropertyHandle.h"

TSharedPtr<SWidget> SFlowGraphNode_YapFragmentWidget::CreateCentreDialogueWidget()
{	
	return SNew(SVerticalBox)
	+ SVerticalBox::Slot()
	.Padding(0, 0, 0, 0)
	.VAlign(VAlign_Fill)
	.HAlign(HAlign_Fill)
	[
		CreateDialogueWidget()
	]
	+ SVerticalBox::Slot()
	.Padding(0, 4, 0, 0)
	.AutoHeight()
	[
		CreateTitleTextWidget()
	];
}

TSharedPtr<SWidget> SFlowGraphNode_YapFragmentWidget::CreateCentreSettingsWidget()
{
	return SNew(SVerticalBox)
	+ SVerticalBox::Slot()
	.Padding(0, 0, 0, 1)
	.VAlign(VAlign_Top)
	.HAlign(HAlign_Fill)
	.AutoHeight()
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.HAlign(HAlign_Fill)
		[
			SNew(SObjectPropertyEntryBox)
			.DisplayBrowse(true)
			.DisplayThumbnail(false)
			.DisplayUseSelected(false)
			.AllowedClass(UYapCharacter::StaticClass())
			.EnableContentPicker(true)
			.ObjectPath(this, &SFlowGraphNode_YapFragmentWidget::ObjectPath_CharacterSelect)
			.OnObjectChanged(this, &SFlowGraphNode_YapFragmentWidget::OnObjectChanged_CharacterSelect)
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			CreateMoodKeySelectorWidget()
		]
	]
	+ SVerticalBox::Slot()
	.Padding(0, 1, 0, 1)
	.VAlign(VAlign_Top)
	.HAlign(HAlign_Fill)
	.AutoHeight()
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.HAlign(HAlign_Fill)
		[
			CreateAudioAssetWidget()
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			CreateAudioPreviewWidget()
		]
	]
	+ SVerticalBox::Slot()
	.VAlign(VAlign_Top)
	.HAlign(HAlign_Right)
	.Padding(0, 1, 0, 0)
	[
		CreateBottomRowWidget()
	];
}

void SFlowGraphNode_YapFragmentWidget::Construct(const FArguments& InArgs, SFlowGraphNode_YapDialogueWidget* InOwner, uint8 InFragmentIndex)
{
	Owner = InOwner;
	FragmentIndex = InFragmentIndex;
	
	ChildSlot
	[
		CreateFragmentWidget()
	];
}

bool SFlowGraphNode_YapFragmentWidget::IsBeingEdited()
{
	return bDialogueExpanded || bTitleTextExpanded || bShowSettings;
}

EVisibility SFlowGraphNode_YapFragmentWidget::Visibility_UpperFragmentBar() const
{
	return (Visibility_ConditionWidgets() != EVisibility::Visible && Visibility_FragmentTagWidget() != EVisibility::Visible) ? EVisibility::Collapsed : EVisibility::Visible;
}

int32 SFlowGraphNode_YapFragmentWidget::GetFragmentActivationCount() const
{
	return GetFragment().GetActivationCount();
}

int32 SFlowGraphNode_YapFragmentWidget::GetFragmentActivationLimit() const
{
	return GetFragment().GetActivationLimit();
}

EVisibility SFlowGraphNode_YapFragmentWidget::Visibility_FragmentControlsWidget() const
{
	return GetFlowYapDialogueNode()->GetNumFragments() > 1 ? EVisibility::Visible : EVisibility::Collapsed;
}

EVisibility SFlowGraphNode_YapFragmentWidget::Visibility_FragmentShiftWidget(EYapFragmentControlsDirection YapFragmentControlsDirection) const
{
	if (FragmentIndex == 0 && YapFragmentControlsDirection == EYapFragmentControlsDirection::Up)
	{
		return EVisibility::Hidden;
	}

	if (FragmentIndex == GetFlowYapDialogueNode()->GetNumFragments() - 1 && YapFragmentControlsDirection == EYapFragmentControlsDirection::Down)
	{
		return EVisibility::Hidden;
	}

	return EVisibility::Visible;
}

FReply SFlowGraphNode_YapFragmentWidget::OnClicked_FragmentShift(EYapFragmentControlsDirection YapFragmentControlsDirection)
{
	int32 OtherIndex = YapFragmentControlsDirection == EYapFragmentControlsDirection::Up ? FragmentIndex - 1 : FragmentIndex + 1;
	
	GetFlowYapDialogueNode()->SwapFragments(FragmentIndex, OtherIndex);

	return FReply::Handled();
}

FReply SFlowGraphNode_YapFragmentWidget::OnClicked_FragmentDelete()
{
	GetFlowYapDialogueNode()->DeleteFragmentByIndex(FragmentIndex);

	return FReply::Handled();
}

TSharedRef<SWidget> SFlowGraphNode_YapFragmentWidget::CreateFragmentControlsWidget()
{
	return SNew(SBox)
	.Visibility(this, &SFlowGraphNode_YapFragmentWidget::Visibility_FragmentControlsWidget)
	[
		SNew(SVerticalBox)
		// UP
		+ SVerticalBox::Slot()
		.AutoHeight()
		.VAlign(VAlign_Top)
		.HAlign(HAlign_Center)
		.Padding(0, 2)
		[
			SNew(SButton)
			.ButtonStyle(FYapEditorStyle::Get(), YapStyles.ButtonStyle_FragmentControls)
			.ContentPadding(FMargin(3, 4))
			.ToolTipText(LOCTEXT("DialogueMoveFragmentUp_Tooltip", "Move Fragment Up"))
			.Visibility(this, &SFlowGraphNode_YapFragmentWidget::Visibility_FragmentShiftWidget, EYapFragmentControlsDirection::Up)
			.OnClicked(this, &SFlowGraphNode_YapFragmentWidget::OnClicked_FragmentShift, EYapFragmentControlsDirection::Up)
			[
				SNew(SImage)
				.Image(FYapEditorStyle::GetImageBrush(YapBrushes.Icon_UpArrow))
				.DesiredSizeOverride(FVector2D(12, 12))
				.ColorAndOpacity(FSlateColor::UseSubduedForeground())
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
			.ButtonStyle(FYapEditorStyle::Get(), YapStyles.ButtonStyle_FragmentControls)
			.ContentPadding(FMargin(3, 4))
			.ToolTipText(LOCTEXT("DialogueDeleteFragment_Tooltip", "Delete Fragment"))
			.OnClicked(this, &SFlowGraphNode_YapFragmentWidget::OnClicked_FragmentDelete)
			[
				SNew(SImage)
				.Image(FYapEditorStyle::GetImageBrush(YapBrushes.Icon_Delete))
				.DesiredSizeOverride(FVector2D(12, 12))
				.ColorAndOpacity(FSlateColor::UseStyle())
			]
		]
		// DOWN
		+ SVerticalBox::Slot()
		.AutoHeight()
		.VAlign(VAlign_Bottom)
		.HAlign(HAlign_Center)
		.Padding(0, 2)
		[
			SNew(SButton)
			.ButtonStyle(FYapEditorStyle::Get(), YapStyles.ButtonStyle_FragmentControls)
			.ContentPadding(FMargin(3, 4))
			.ToolTipText(LOCTEXT("DialogueMoveFragmentDown_Tooltip", "Move Fragment Down"))
			.Visibility(this, &SFlowGraphNode_YapFragmentWidget::Visibility_FragmentShiftWidget, EYapFragmentControlsDirection::Down)
			.OnClicked(this, &SFlowGraphNode_YapFragmentWidget::OnClicked_FragmentShift, EYapFragmentControlsDirection::Down)
			[
				SNew(SImage)
				.Image(FYapEditorStyle::GetImageBrush(YapBrushes.Icon_DownArrow))
				.DesiredSizeOverride(FVector2D(12, 12))
				.ColorAndOpacity(FSlateColor::UseForeground())
			]
		]
	];
}

TSharedRef<SWidget> SFlowGraphNode_YapFragmentWidget::CreateAudioPreviewWidget()
{
	return CreateAudioPreviewWidget(EVisibility::Visible);
}

bool SFlowGraphNode_YapFragmentWidget::Enabled_AudioPreviewButton() const
{
	return GetFragment().GetBit().HasDialogueAudioAsset();
}

TSharedRef<SWidget> SFlowGraphNode_YapFragmentWidget::CreateAudioPreviewWidget(TAttribute<EVisibility> Attribute)
{
	return SNew(SButton)
	.ButtonColorAndOpacity(YapColor::Gray)
	.ContentPadding(0)
	.ButtonStyle(FYapEditorStyle::Get(), YapStyles.ButtonStyle_ActivationLimit)
	.Visibility(Attribute)
	.IsEnabled(this, &SFlowGraphNode_YapFragmentWidget::Enabled_AudioPreviewButton)
	.ToolTipText(INVTEXT("Play audio"))
	[
		SNew(SBox)
		.Padding(4, 4)
		[
			SNew(SImage)
			.DesiredSizeOverride(FVector2D(16, 16))
			.Image(FYapEditorStyle::GetImageBrush(YapBrushes.Icon_Speaker))
			.ColorAndOpacity(FSlateColor::UseForeground())
		]
	];
}

TSharedRef<SWidget> SFlowGraphNode_YapFragmentWidget::CreateFragmentHighlightWidget()
{
	return SNew(SBorder)
	.BorderImage(FAppStyle::GetBrush("Graph.StateNode.Body")) // Filled, rounded nicely
	.Visibility(this, &SFlowGraphNode_YapFragmentWidget::Visibility_RowHighlight)
	.BorderBackgroundColor(this, &SFlowGraphNode_YapFragmentWidget::BorderBackgroundColor_RowHighlight);
}

void SFlowGraphNode_YapFragmentWidget::OnTextCommitted_FragmentActivationLimit(const FText& Text, ETextCommit::Type Arg)
{
	FYapTransactions::BeginModify(LOCTEXT("Dialogue", "Change Dialogue Activation Limit"), GetFlowYapDialogueNode());

	GetFragment().ActivationLimit = FCString::Atoi(*Text.ToString());

	GetFlowYapDialogueNode()->OnReconstructionRequested.Execute();
	
	FYapTransactions::EndModify();
}

EVisibility SFlowGraphNode_YapFragmentWidget::Visibility_FragmentRowNormalControls() const
{
	if (bDialogueExpanded || bTitleTextExpanded)
	{
		return EVisibility::Hidden;
	}
	
	return EVisibility::Visible;
}

TSharedRef<SWidget> SFlowGraphNode_YapFragmentWidget::CreateUpperFragmentBar()
{
	return SNew(SBox)
	.Visibility(this, &SFlowGraphNode_YapFragmentWidget::Visibility_UpperFragmentBar)
	.Padding(0, 0, 0, 4)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.HAlign(HAlign_Fill)
		.Padding(32, 0, 4, 0)
		[
			SNew(SBox)
			.ToolTipText(INVTEXT("Conditions"))
			[
				CreateConditionWidgets()
			]
		]
		+ SHorizontalBox::Slot()
		.HAlign(HAlign_Right)
		.AutoWidth()
		.VAlign(VAlign_Fill)
		.Padding(4, 0, 32, 0)
		[
			SNew(SBox)
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.Visibility(EVisibility::Visible)
			[
				CreateFragmentTagWidget()
			]
		]
	];
}

EVisibility SFlowGraphNode_YapFragmentWidget::Visibility_FragmentTagWidget() const
{
	return GetFlowYapDialogueNode()->GetDialogueTag().IsValid() ? EVisibility::Visible : EVisibility::Collapsed;
}

// ================================================================================================
// FRAGMENT WIDGET
// ------------------------------------------------------------------------------------------------

TSharedRef<SWidget> SFlowGraphNode_YapFragmentWidget::CreateFragmentWidget()
{
	PortraitWidget = CreatePortraitWidget();

	CentreBox = SNew(SBox);

	CentreDialogueWidget = CreateCentreDialogueWidget();
	
	CentreBox->SetContent(CentreDialogueWidget.ToSharedRef());

	return SAssignNew(FragmentWidgetOverlay, SOverlay)
	+ SOverlay::Slot()
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 0, 0, 0)
		[
			CreateUpperFragmentBar()
		]
		+ SVerticalBox::Slot()
		.Padding(0, 0, 0, 0)
		.AutoHeight()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Fill)
			[
				SNew(SBox)
				.WidthOverride(32)
				.HeightOverride(32)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					SNew(SActivationCounterWidget, FOnTextCommitted::CreateSP(this, &SFlowGraphNode_YapFragmentWidget::OnTextCommitted_FragmentActivationLimit))
					.ActivationCount(this, &SFlowGraphNode_YapFragmentWidget::GetFragmentActivationCount)
					.ActivationLimit(this, &SFlowGraphNode_YapFragmentWidget::GetFragmentActivationLimit)
					.FontHeight(10)
				]
			]
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Fill)
			[
				SAssignNew(FragmentOverlay, SOverlay)
				+ SOverlay::Slot()
				[
					SNew(SBox)
					.Visibility(this, &SFlowGraphNode_YapFragmentWidget::Visibility_FragmentRowNormalControls)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Top)
						.AutoWidth()
						.Padding(0, 0, 2, 0)
						[
							SNew(SOverlay)
							+ SOverlay::Slot()
							[
								// TODO profile is this still slow
								PortraitWidget.ToSharedRef()
							]
							+ SOverlay::Slot()
							.VAlign(VAlign_Bottom)
							.HAlign(HAlign_Right)
							.Padding(0, 0, -4, -4)
							[
								CreateAudioPreviewWidget(TAttribute<EVisibility>::CreateSP(this, &SFlowGraphNode_YapFragmentWidget::Visibility_AudioButton))
							]
						]
						+ SHorizontalBox::Slot()
						.HAlign(HAlign_Fill)
						.VAlign(VAlign_Fill)
						.FillWidth(1.0f)
						.Padding(2, 0, 0, 0)
						[
							CentreBox.ToSharedRef()
						]
					]
				]
				+ SOverlay::Slot()
				[
					// TODO can I defer creation of this until later?
					CreateFragmentHighlightWidget()
				]
			]
			+SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SBox)
				.WidthOverride(32)
				[
					CreateRightFragmentPane()
				]
			]
		]
	];
}

FReply SFlowGraphNode_YapFragmentWidget::OnClicked_DialogueExpandButton()
{
	if (bDialogueExpanded)
	{
		return FReply::Handled();
	}
	
	if (ExpandedDialogueWidget == nullptr)
	{
		TSharedRef<IEditableTextProperty> EditableTextProperty = MakeShareable(new FYapEditableTextPropertyHandle(GetFragment().GetBitMutable().GetDialogueTextMutable()));
		
		ExpandedDialogueWidget = SNew(SOverlay)
		+ SOverlay::Slot()
		[
			SNew(SYapTextPropertyEditableTextBox, EditableTextProperty)
		]
		+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Bottom)
		.Padding(0, 0, 28, -8)
		[
			SNew(SSlider)
			.Value(this, &SFlowGraphNode_YapFragmentWidget::Value_FragmentTimePadding)
			.OnValueChanged(this, &SFlowGraphNode_YapFragmentWidget::OnValueChanged_FragmentTimePadding)
			.SliderBarColor(this, &SFlowGraphNode_YapFragmentWidget::FragmentTimePadding_FillColorAndOpacity)
			.ToolTipText(this, &SFlowGraphNode_YapFragmentWidget::ToolTipText_FragmentTimePadding)
		];
	}

	FragmentOverlay->AddSlot()
	.Padding(-1, 0, -1, 0)
	[
		ExpandedDialogueWidget.ToSharedRef()
	];

	bDialogueExpanded = true;

	SetNodeSelected();
	
	return FReply::Handled();
}

EVisibility SFlowGraphNode_YapFragmentWidget::Visibility_DialogueEdit() const
{
	return DialogueEditButtonWidget->IsHovered() ? EVisibility::HitTestInvisible : EVisibility::Collapsed;
}

TSharedRef<SWidget> SFlowGraphNode_YapFragmentWidget::CreateTextEditButtonWidget(TAttribute<EVisibility> InVisibility)
{
	return SNew(SOverlay)
	.Visibility(InVisibility)
	+ SOverlay::Slot()
	[
		SNew(SImage)
		.DesiredSizeOverride(FVector2D(24, 24))
		.Image(FAppStyle::GetBrush("Icons.FilledCircle"))
		.ColorAndOpacity(YapColor::Black)
		.Visibility(EVisibility::HitTestInvisible)
	]
	+ SOverlay::Slot()
	.VAlign(VAlign_Center)
	.HAlign(HAlign_Center)
	.Padding(0,0)
	[
		SNew(SImage)
		.Image(FYapEditorStyle::GetImageBrush(YapBrushes.Icon_Edit))
		.ColorAndOpacity(YapColor::White)
		.Visibility(EVisibility::HitTestInvisible)
	];
}

// ================================================================================================
// DIALOGUE WIDGET
// ------------------------------------------------------------------------------------------------

TSharedRef<SWidget> SFlowGraphNode_YapFragmentWidget::CreateDialogueWidget()
{
	DialogueEditButtonWidget = SNew(SButton)
	.ToolTipText(this, &SFlowGraphNode_YapFragmentWidget::ToolTipText_Dialogue)
	.ButtonStyle(FYapEditorStyle::Get(), YapStyles.ButtonStyle_ActivationLimit)
	.OnClicked(this, &SFlowGraphNode_YapFragmentWidget::OnClicked_DialogueExpandButton)
	[
		SNew(SOverlay)
		+ SOverlay::Slot()
		[
			SNew(SScrollBox)
			.Orientation(Orient_Horizontal)
			.ScrollBarVisibility(EVisibility::Collapsed)
			.ConsumeMouseWheel(EConsumeMouseWheel::Always)
			.AllowOverscroll(EAllowOverscroll::No)
			.AnimateWheelScrolling(true)
			+ SScrollBox::Slot()
			.Padding(0,0,0,0)
			.FillSize(1.0)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.TextStyle(FYapEditorStyle::Get(), YapStyles.TextBlockStyle_Dialogue)
				.Text(this, &SFlowGraphNode_YapFragmentWidget::Dialogue_Text)
			]
		]
		+ SOverlay::Slot()
		.VAlign(VAlign_Bottom)
		.HAlign(HAlign_Fill)
		.Padding(0, 0, 0, 2)
		[
			SNew(SBox)
			.HeightOverride(2)
			.ToolTipText(this, &SFlowGraphNode_YapFragmentWidget::ToolTipText_FragmentTimePadding)
			[
				SNew(SProgressBar)
				.BorderPadding(0)
				.Percent(this, &SFlowGraphNode_YapFragmentWidget::FragmentTimePadding_Percent)
				.Style(FYapEditorStyle::Get(), YapStyles.ProgressBarStyle_FragmentTimePadding)
				.FillColorAndOpacity(this, &SFlowGraphNode_YapFragmentWidget::FragmentTimePadding_FillColorAndOpacity)
				.BarFillType(EProgressBarFillType::FillFromCenterHorizontal)
			]
		]
		/*
		+ SOverlay::Slot()
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Center)
		.Padding(-4)
		[
			CreateTextEditButtonWidget(TAttribute<EVisibility>::CreateSP(this, &SFlowGraphNode_YapFragmentWidget::Visibility_DialogueEdit))
		]
		*/
	];

	return DialogueEditButtonWidget.ToSharedRef();
}

FVector2D SFlowGraphNode_YapFragmentWidget::DialogueScrollBar_Thickness() const
{
	if (FragmentFocused())
	{
		return FVector2D(8, 8);
	}
	
	return FVector2D(8, 8);
}

FOptionalSize SFlowGraphNode_YapFragmentWidget::Dialogue_MaxDesiredHeight() const
{
	if (FragmentFocused())
	{
		int16 DeadSpace = 15;
		int16 LineHeight = 15;
		int16 FocusedLines = 9;
	
		return DeadSpace + FocusedLines * LineHeight;
	}

	return 58; // TODO fluctuate with portrait widget height
}

FText SFlowGraphNode_YapFragmentWidget::Dialogue_Text() const
{
	return GetFragment().Bit.GetDialogueText();
}

void SFlowGraphNode_YapFragmentWidget::OnTextCommitted_Dialogue(const FText& CommittedText, ETextCommit::Type CommitType)
{
	FYapTransactions::BeginModify(LOCTEXT("NodeDialogueTextChanged", "Dialogue Text Changed"), GetFlowYapDialogueNode());

	if (CommitType == ETextCommit::OnEnter)
	{
		GetFragment().Bit.SetDialogueText(CommittedText);
	}

	FYapTransactions::EndModify();
}

FText SFlowGraphNode_YapFragmentWidget::ToolTipText_Dialogue() const
{
	double PaddingTime = GetFragment().GetPaddingToNextFragment();

	FNumberFormattingOptions Formatting;
	Formatting.MaximumFractionalDigits = 2;

	const FText& DialogueText = GetFragment().GetBit().GetDialogueText();

	FText DialogueStr = Dialogue_Text().IsEmpty() ? INVTEXT("No dialogue") : Dialogue_Text(); 
	
	if (PaddingTime > 0)
	{
		return FText::Format(INVTEXT("{0}\nPadding: {1}"), DialogueStr, FText::AsNumber(PaddingTime, &Formatting));
	}
	else
	{
		return FText::Format(INVTEXT("{0}"), DialogueStr);
	}
}

FSlateColor SFlowGraphNode_YapFragmentWidget::BackgroundColor_Dialogue() const
{
	if (GetFragment().GetBitReplaced())
	{
		return YapColor::Yellow;
	}
	
	return GetFlowYapDialogueNode()->GetIsPlayerPrompt() ? YapColor::White : YapColor::Noir;
}

FSlateColor SFlowGraphNode_YapFragmentWidget::ForegroundColor_Dialogue() const
{
	FLinearColor Color = YapColor::Error;
	
	Color = GetFlowYapDialogueNode()->GetIsPlayerPrompt() ? YapColor::White : YapColor::LightGray;
	
	if (GEditor->PlayWorld)
	{
		if (GetFlowYapDialogueNode()->GetRunningFragmentIndex() == GetFragment().IndexInDialogue)
		{
			Color = YapColor::White;
		}
		
		Color = (GetFragment().IsActivationLimitMet()) ? YapColor::DarkRed : YapColor::LightGray;
	}

	if (GetFragment().GetBitReplaced())
	{
		Color *= YapColor::LightBlue;
	}

	return Color;
}

EVisibility SFlowGraphNode_YapFragmentWidget::Visibility_DialogueBackground() const
{
	return GetFragment().GetBitReplaced() ? EVisibility::Visible : EVisibility::Collapsed;
}

FSlateColor SFlowGraphNode_YapFragmentWidget::BorderBackgroundColor_Dialogue() const
{
	return YapColor::LightYellow_SuperGlass;
}

// ================================================================================================
// FRAGMENT TAG OVERLAY WIDGET (OVER DIALOGUE, PREVIEW PURPOSE ONLY)
// ------------------------------------------------------------------------------------------------

FText SFlowGraphNode_YapFragmentWidget::FragmentTagPreview_Text() const
{
	// Pass tag from the properties

	FString Filter = GetFlowYapDialogueNode()->GetDialogueTag().ToString();

	FText Text = FText::FromString(YapUtil::GetFilteredSubTag(Filter, GetFragment().FragmentTag));

	if (Text.IsEmptyOrWhitespace())
	{
		return INVTEXT("");
	}
	else
	{
		return Text;
	}
}

TSharedRef<SWidget> SFlowGraphNode_YapFragmentWidget::CreateConditionWidgets() const
{
	TSharedRef<SScrollBox> Box = SNew(SScrollBox)
	.Visibility(this, &SFlowGraphNode_YapFragmentWidget::Visibility_ConditionWidgets)
	.ScrollBarVisibility(EVisibility::Collapsed)
	.ConsumeMouseWheel(EConsumeMouseWheel::Always)
	.AllowOverscroll(EAllowOverscroll::No)
	.AnimateWheelScrolling(true)
	.Orientation(Orient_Horizontal);
	for (UYapCondition* Condition : GetFragment().GetConditionsMutable())
	{
		Box->AddSlot()
		.Padding(0, 0, 4, 0)
		[
			FYapWidgetHelper::CreateConditionWidget(GetFlowYapDialogueNode(), Condition)
		];	
	}
	
	return Box;
}

EVisibility SFlowGraphNode_YapFragmentWidget::Visibility_ConditionWidgets() const
{
	return (GetFragment().GetConditions().Num() > 0) ? EVisibility::Visible : EVisibility::Hidden;
}

// ================================================================================================
// FRAGMENT TIME PADDING WIDGET
// ------------------------------------------------------------------------------------------------

TOptional<float> SFlowGraphNode_YapFragmentWidget::FragmentTimePadding_Percent() const
{
	const float MaxPaddedSetting =  UYapProjectSettings::Get()->GetFragmentPaddingSliderMax();
	const float FragmentPadding = GetFragment().GetPaddingToNextFragment();

	if (GEditor->PlayWorld)
	{
		const TOptional<uint8>& RunningIndex = GetFlowYapDialogueNode()->RunningFragmentIndex;

		if (!RunningIndex.IsSet() || RunningIndex.GetValue() < GetFragment().IndexInDialogue)
		{
			return FragmentPadding / MaxPaddedSetting;
		}

		if (RunningIndex == GetFragment().IndexInDialogue)
		{
			if (GetFlowYapDialogueNode()->FragmentStartedTime < GetFlowYapDialogueNode()->FragmentEndedTime)
			{
				double ElapsedPaddingTime = GEditor->PlayWorld->GetTimeSeconds() - GetFlowYapDialogueNode()->FragmentEndedTime;
				return (1 - (ElapsedPaddingTime / FragmentPadding)) * (FragmentPadding / MaxPaddedSetting);
			}
			else
			{
				return FragmentPadding / MaxPaddedSetting;
			}
		}

		return 0.0;
	}

	return FragmentPadding / MaxPaddedSetting;		
}

float SFlowGraphNode_YapFragmentWidget::Value_FragmentTimePadding() const
{
	const float MaxPaddedSetting =  UYapProjectSettings::Get()->GetFragmentPaddingSliderMax();

	return GetFragment().GetPaddingToNextFragment() / MaxPaddedSetting;
}

void SFlowGraphNode_YapFragmentWidget::OnValueChanged_FragmentTimePadding(float X)
{
	const float MaxPaddedSetting =  UYapProjectSettings::Get()->GetFragmentPaddingSliderMax();
	float NewValue = X * MaxPaddedSetting;
	
	bool bSetCommon = false;
	
	if (!bCtrlPressed)
	{
		const TArray<float>& CommonPaddings = UYapProjectSettings::Get()->GetCommonFragmentPaddings();
		
		if (CommonPaddings.Num() > 0)
		{
			if (NewValue <= (CommonPaddings[CommonPaddings.Num() - 1]))
			{
				int Index = 0;
				
				for (int i = 0; i < CommonPaddings.Num() - 1; ++i)
				{
					float Threshold = 0.5f * (CommonPaddings[i] + CommonPaddings[i + 1]);

					if (NewValue > Threshold)
					{
						Index = i + 1;
					}
				}
				
				GetFragment().GetCommonPaddingSettingMutable() = Index;
				GetFragment().SetPaddingToNextFragment(0);
				
				bSetCommon = true;
			}
		}
	}
	
	if (!bSetCommon)
	{
		GetFragment().SetPaddingToNextFragment(NewValue);
		GetFragment().GetCommonPaddingSettingMutable().Reset();
	}
}

FSlateColor SFlowGraphNode_YapFragmentWidget::FragmentTimePadding_FillColorAndOpacity() const
{
	const float MaxPaddedSetting =  UYapProjectSettings::Get()->GetFragmentPaddingSliderMax();

	if (GetFragment().GetPaddingToNextFragment() > MaxPaddedSetting)
	{
		return YapColor::Blue_Trans;
	}
	
	if (GEditor->PlayWorld)
	{
		const TOptional<uint8>& RunningIndex = GetFlowYapDialogueNode()->RunningFragmentIndex;

		if (!RunningIndex.IsSet() || RunningIndex.GetValue() < GetFragment().IndexInDialogue)
		{
		}

		if (RunningIndex == GetFragment().IndexInDialogue)
		{
			return YapColor::White_Trans;
		}

		return YapColor::DarkGray_Trans;
	}

	return GetFragment().GetCommonPaddingSetting().IsSet() ? YapColor::DimGray_Trans : YapColor::LightBlue_Trans;
}

FText SFlowGraphNode_YapFragmentWidget::ToolTipText_FragmentTimePadding() const
{
	return FText::Format(LOCTEXT("Fragment", "Delay: {0}"), GetFragment().GetPaddingToNextFragment());
}

FSlateColor SFlowGraphNode_YapFragmentWidget::BorderBackgroundColor_PortraitImage() const
{
	return GetFlowYapDialogueNode()->GetIsPlayerPrompt() ? YapColor::YellowGray_Trans : YapColor::Gray_Glass;
}

FText SFlowGraphNode_YapFragmentWidget::ToolTipText_PortraitWidget() const
{
	const FYapBit& Bit = GetFragment().GetBit();

	if (Bit.GetCharacterAsset().IsPending())
	{
		return INVTEXT("Unloaded");
	}
	else if (Bit.GetCharacterAsset().IsNull())
	{
		return INVTEXT("Speaker is unset");
	}
	else
	{
		// TODO
		return INVTEXT("Speaker is set");
		// return FText::Format(INVTEXT("{0}\n\n{1}"), Bit.GetCharacterAsset()->GetEntityName(), FText::FromName(Bit.GetMoodKey()));
	}
}

FReply SFlowGraphNode_YapFragmentWidget::OnClicked_PortraitWidget()
{
	bShowSettings = !bShowSettings;

	if (bShowSettings)
	{
		if (CentreSettingsWidget == nullptr)
		{
			CentreSettingsWidget = CreateCentreSettingsWidget();
		}
		
		CentreBox->SetContent(CentreSettingsWidget.ToSharedRef());

		SetNodeSelected();
	}
	else
	{
		if (CentreDialogueWidget == nullptr)
		{
			CentreDialogueWidget = CreateCentreDialogueWidget();
		}
		CentreBox->SetContent(CentreDialogueWidget.ToSharedRef());
	}

	return FReply::Handled();
}

// ================================================================================================
// PORTRAIT WIDGET
// ------------------------------------------------------------------------------------------------

TSharedRef<SOverlay> SFlowGraphNode_YapFragmentWidget::CreatePortraitWidget()
{
	int32 PortraitSize = UYapProjectSettings::Get()->GetPortraitSize();
	
	return SNew(SOverlay)
	+ SOverlay::Slot()
	.Padding(0, 0, 0, 0)
	[
		SNew(SBox)
		.WidthOverride(PortraitSize + 8)
		.HeightOverride(PortraitSize + 8)
		[
			SNew(SBorder)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			.BorderImage(FYapEditorStyle::GetImageBrush(YapBrushes.Border_DeburredSquare))
			.BorderBackgroundColor(this, &SFlowGraphNode_YapFragmentWidget::BorderBackgroundColor_PortraitImage)
		]
	]
	+ SOverlay::Slot()
	[
		SNew(SButton)
		.ButtonStyle(FAppStyle::Get(), "SimpleButton")
		.ToolTipText(this, &SFlowGraphNode_YapFragmentWidget::ToolTipText_PortraitWidget)
		.ContentPadding(0)
		.OnClicked(this, &SFlowGraphNode_YapFragmentWidget::OnClicked_PortraitWidget)
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Center)
			[
				SNew(SImage)
				.DesiredSizeOverride(FVector2D(PortraitSize, PortraitSize))
				.Image(this, &SFlowGraphNode_YapFragmentWidget::Image_PortraitImage)
			]
			+ SOverlay::Slot()
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Center)
			[
				SNew(STextBlock)
				.Visibility(this, &SFlowGraphNode_YapFragmentWidget::Visibility_MissingPortraitWarning)
				.Text(LOCTEXT("FragmentCharacterMissing", "?"))
				.Justification(ETextJustify::Center)
			]
		]
	];
}

EVisibility SFlowGraphNode_YapFragmentWidget::Visibility_PortraitImage() const
{	
	if (FragmentFocused())
	{
		return EVisibility::Collapsed;
	}
	
	if (!Owner->GetIsSelected())
	{
		return EVisibility::Visible;
	}

	return EVisibility::Visible;
}

const FSlateBrush* SFlowGraphNode_YapFragmentWidget::Image_PortraitImage() const
{
	return GetFragment().GetBit().GetSpeakerPortraitBrush();
}

EVisibility SFlowGraphNode_YapFragmentWidget::Visibility_MissingPortraitWarning() const
{
	const FSlateBrush* Brush = GetFragment().GetBit().GetSpeakerPortraitBrush();

	if (Brush)
	{
		return (Brush->GetResourceObject()) ? EVisibility::Hidden : EVisibility::Visible;
	}
	
	return EVisibility::Visible;
}

EVisibility SFlowGraphNode_YapFragmentWidget::Visibility_CharacterSelect() const
{
	return IsHovered() ? EVisibility::Visible : EVisibility::Collapsed;
}

FString SFlowGraphNode_YapFragmentWidget::ObjectPath_CharacterSelect() const
{	
	const TSoftObjectPtr<UYapCharacter> Asset = GetFragment().Bit.GetCharacterAsset();

	if (Asset.IsPending())
	{
		Asset.LoadSynchronous();
	}
	
	if (!Asset) { return ""; }

	return Asset.ToString();
}

void SFlowGraphNode_YapFragmentWidget::OnObjectChanged_CharacterSelect(const FAssetData& InAssetData)
{
	UObject* Asset = InAssetData.GetAsset();

	UYapCharacter* Character = Cast<UYapCharacter>(Asset);

	if (Character)
	{
		FYapTransactions::BeginModify(LOCTEXT("NodeCharacterChanged", "Character Changed"), GetFlowYapDialogueNode());

		GetFragment().Bit.SetCharacter(Character);

		FYapTransactions::EndModify();
	}
}

// ================================================================================================
// TITLE TEXT WIDGET
// ------------------------------------------------------------------------------------------------

TSharedRef<SWidget> SFlowGraphNode_YapFragmentWidget::CreateTitleTextWidget()
{
	TitleTextEditButtonWidget = SNew(SButton)
	.Visibility(this, &SFlowGraphNode_YapFragmentWidget::Visibility_TitleText)
	.ToolTipText(this, &SFlowGraphNode_YapFragmentWidget::ToolTipText_TitleText)
	.ButtonStyle(FYapEditorStyle::Get(),YapStyles.ButtonStyle_ActivationLimit)
	.OnClicked(this, &SFlowGraphNode_YapFragmentWidget::OnClicked_TitleTextExpandButton)
	[
		SNew(SOverlay)
		+ SOverlay::Slot()
		[
			SNew(SScrollBox)
			.Orientation(Orient_Horizontal)
			.ScrollBarVisibility(EVisibility::Collapsed)
			.ConsumeMouseWheel(EConsumeMouseWheel::Always)
			.AllowOverscroll(EAllowOverscroll::No)
			.AnimateWheelScrolling(true)
			+ SScrollBox::Slot()
			.Padding(0,0,0,0)
			.FillSize(1.0)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.TextStyle(FYapEditorStyle::Get(), YapStyles.TextBlockStyle_TitleText)
				.Text(this, &SFlowGraphNode_YapFragmentWidget::Text_TitleText)
			]
		]
		/*
		+ SOverlay::Slot()
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Center)
		.Padding(-4)
		[
			CreateTextEditButtonWidget(TAttribute<EVisibility>::CreateSP(this, &SFlowGraphNode_YapFragmentWidget::Visibility_TitleTextEdit))
		]
		*/
	];

	return TitleTextEditButtonWidget.ToSharedRef();
}

EVisibility SFlowGraphNode_YapFragmentWidget::Visibility_TitleTextEdit() const
{
	return TitleTextEditButtonWidget->IsHovered() ? EVisibility::HitTestInvisible : EVisibility::Collapsed;
}

FReply SFlowGraphNode_YapFragmentWidget::OnClicked_TitleTextExpandButton()
{
	if (bTitleTextExpanded)
	{
		return FReply::Handled();
	}
	
	if (ExpandedTitleTextWidget == nullptr)
	{
		TSharedRef<IEditableTextProperty> EditableTextProperty = MakeShareable(new FYapEditableTextPropertyHandle(GetFragment().GetBitMutable().GetTitleTextMutable()));
		
		ExpandedTitleTextWidget = SNew(SYapTextPropertyEditableTextBox, EditableTextProperty)
		.Style(FYapEditorStyle::Get(), YapStyles.EditableTextBoxStyle_TitleText);
	}
	
	FragmentOverlay->AddSlot()
	.Padding(-1, 0, -1, 0)
	[
		ExpandedTitleTextWidget.ToSharedRef()
	];

	bTitleTextExpanded = true;

	SetNodeSelected();
	
	return FReply::Handled();
}

EVisibility SFlowGraphNode_YapFragmentWidget::Visibility_TitleText() const
{
	if (GetFlowYapDialogueNode()->GetIsPlayerPrompt())
	{
		return EVisibility::Visible;
	}
	
	return UYapProjectSettings::Get()->GetHideTitleTextOnNPCDialogueNodes() ? EVisibility::Collapsed : EVisibility::Visible;
}

FText SFlowGraphNode_YapFragmentWidget::ToolTipText_TitleText() const
{
	FText TitleText = GetFragment().GetBit().GetTitleText();

	if (TitleText.IsEmpty())
	{
		return INVTEXT("No title text");
	}

	return TitleText;
}

FText SFlowGraphNode_YapFragmentWidget::Text_TitleText() const
{
	return GetFragment().Bit.GetTitleText();
}

void SFlowGraphNode_YapFragmentWidget::TitleText_OnTextCommitted(const FText& CommittedText, ETextCommit::Type CommitType)
{
	FYapTransactions::BeginModify(LOCTEXT("NodeTitleTextChanged", "Title Text Changed"), GetFlowYapDialogueNode());
	
	if (CommitType == ETextCommit::OnEnter)
	{
		GetFragment().Bit.SetTitleText(CommittedText);
	}

	FYapTransactions::EndModify();
}

// ================================================================================================
// FRAGMENT TAG WIDGET
// ------------------------------------------------------------------------------------------------

TSharedRef<SWidget> SFlowGraphNode_YapFragmentWidget::CreateFragmentTagWidget()
{
	auto TagAttribute = TAttribute<FGameplayTag>::CreateSP(this, &SFlowGraphNode_YapFragmentWidget::Value_FragmentTag);
	FString FilterString = GetFlowYapDialogueNode()->GetDialogueTag().ToString();
	auto OnTagChanged = TDelegate<void(const FGameplayTag)>::CreateSP(this, &SFlowGraphNode_YapFragmentWidget::OnTagChanged_FragmentTag);
	
	return FYapWidgetHelper::CreateFilteredTagWidget(TagAttribute, FilterString, OnTagChanged, INVTEXT("Fragment tag"));
}

FGameplayTag SFlowGraphNode_YapFragmentWidget::Value_FragmentTag() const
{
	return GetFragment().FragmentTag;
}

void SFlowGraphNode_YapFragmentWidget::OnTagChanged_FragmentTag(FGameplayTag GameplayTag)
{
	FYapTransactions::BeginModify(LOCTEXT("Fragment", "Change Fragment Tag"), GetFlowYapDialogueNode());

	GetFragment().FragmentTag = GameplayTag;

	FYapTransactions::EndModify();

	Owner->RequestUpdateGraphNode();
}

// ================================================================================================
// BOTTOM ROW WIDGET (AUDIO ASSET, TIME SETTINGS)
// ------------------------------------------------------------------------------------------------

TSharedRef<SBox> SFlowGraphNode_YapFragmentWidget::CreateBottomRowWidget()
{
	FSlateIcon ProjectSettingsIcon(FAppStyle::GetAppStyleSetName(), "ProjectSettings.TabIcon");

	TSharedRef<SBox> Box = SNew(SBox)
	[
		SNew(SHorizontalBox)
		// -------------------
		// TIME SETTINGS
		// -------------------
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.HAlign(HAlign_Right)
		.Padding(6,0,0,0)
		[
			SNew(SBox)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Right)
				.VAlign(VAlign_Center)
				.Padding(2, 0, 2, 0)
				[
					SNew(STextBlock)
					.Font(FCoreStyle::GetDefaultFontStyle("Regular", 8))
					.Text(INVTEXT("Time: "))
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(2, 0, 1, 0)
				[
					// =============================
					// TIME DISPLAY / MANUAL ENTRY FIELD
					// =============================
					SNew(SBox)
					.WidthOverride(49)
					.VAlign(VAlign_Fill)
					[
						SNew(SNumericEntryBox<double>)
						.IsEnabled(this, &SFlowGraphNode_YapFragmentWidget::IsEnabled_TimeEntryBox)
						.Delta(0.1)
						.MinValue(0.0)
						.ToolTipText(LOCTEXT("FragmentTimeEntry_Tooltip", "Time this dialogue fragment will play for"))
						.Justification(ETextJustify::Center)
						.Value(this, &SFlowGraphNode_YapFragmentWidget::Value_TimeEntryBox)
						.OnValueCommitted(this, &SFlowGraphNode_YapFragmentWidget::OnValueCommitted_TimeEntryBox)
					]
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				.Padding(0, 0, 1, 0)
				[
					// =============================
					// USE PROJECT DEFAULTS BUTTON
					// =============================
					SNew(SCheckBox)
					.Style(&UYapEditorSubsystem::GetCheckBoxStyles().ToggleButtonCheckBox_Green)
					.Padding(FMargin(4, 3))
					.CheckBoxContentUsesAutoWidth(true)
					.ToolTipText(LOCTEXT("UseProjectDefaultTimeSettings_Tooltip", "Use time settings from project settings"))
					.IsEnabled(true)
					.IsChecked(this, &SFlowGraphNode_YapFragmentWidget::IsChecked_UseProjectDefaultTimeSettingsButton)
					.OnCheckStateChanged(this, &SFlowGraphNode_YapFragmentWidget::OnCheckStateChanged_UseProjectDefaultTimeSettingsButton)
					.Content()
					[
						SNew(SImage)
						.ColorAndOpacity(FSlateColor::UseForeground())
						.Image(ProjectSettingsIcon.GetIcon())
					]
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				.Padding(1, 0, 1, 0)
				[
					// =============================
					// USE MANUAL TIME ENTRY BUTTON
					// =============================
					SNew(SCheckBox)
					.Style(&FAppStyle::Get().GetWidgetStyle<FCheckBoxStyle>("ToggleButtonCheckBox"))
					.Padding(FMargin(4, 3))
					.CheckBoxContentUsesAutoWidth(true)
					.ToolTipText(LOCTEXT("UseEnteredTime_Tooltip", "Use a manually entered time"))
					.IsEnabled(this, &SFlowGraphNode_YapFragmentWidget::IsEnabled_UseManuallyEnteredTimeButton)
					.IsChecked(this, &SFlowGraphNode_YapFragmentWidget::IsChecked_UseManuallyEnteredTimeButton)
					.OnCheckStateChanged(this, &SFlowGraphNode_YapFragmentWidget::OnCheckStateChanged_UseManuallyEnteredTimeButton)
					[
						SNew(SImage)
						.ColorAndOpacity(FSlateColor::UseForeground())
						.Image(FYapEditorStyle::GetImageBrush(YapBrushes.Icon_Timer))
					]
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				.Padding(1, 0, 1, 0)
				[
					// =============================
					// USE TEXT TIME BUTTON
					// =============================
					SNew(SCheckBox)
					.Style(&FAppStyle::Get().GetWidgetStyle<FCheckBoxStyle>("ToggleButtonCheckBox"))
					.Padding(FMargin(4, 3))
					.CheckBoxContentUsesAutoWidth(true)
					.ToolTipText(LOCTEXT("UseTimeFromText_Tooltip", "Use a time calculated from text length"))
					.IsEnabled(this, &SFlowGraphNode_YapFragmentWidget::IsEnabled_UseTextTimeButton)
					.IsChecked(this, &SFlowGraphNode_YapFragmentWidget::IsChecked_UseTextTimeButton)
					.OnCheckStateChanged(this, &SFlowGraphNode_YapFragmentWidget::OnCheckStateChanged_UseTextTimeButton)
					[
						SNew(SBox)
						[
							SNew(SImage)
							.Image(FYapEditorStyle::GetImageBrush(YapBrushes.Icon_TextTime))
						]
					]
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				.Padding(1, 0, 1, 0)
				[
					// =============================
					// USE AUDIO TIME BUTTON
					// =============================
					SNew(SCheckBox)
					.Style(&FAppStyle::Get().GetWidgetStyle<FCheckBoxStyle>("ToggleButtonCheckBox"))
					.Padding(FMargin(4, 3))
					.CheckBoxContentUsesAutoWidth(true)
					.ToolTipText(LOCTEXT("UseTimeFromAudio_Tooltip", "Use a time read from the audio asset"))
					.IsEnabled(this, &SFlowGraphNode_YapFragmentWidget::IsEnabled_UseAudioTimeButton)
					.IsChecked(this, &SFlowGraphNode_YapFragmentWidget::IsChecked_UseAudioTimeButton)
					.OnCheckStateChanged(this, &SFlowGraphNode_YapFragmentWidget::OnCheckStateChanged_UseAudioTimeButton)
					.HAlign(HAlign_Center)
					[
						SNew(SImage)
						.Image(FYapEditorStyle::GetImageBrush(YapBrushes.Icon_AudioTime))
					]
				]
			]
		]
	];

	
	return Box;
}

// ---------------------
ECheckBoxState SFlowGraphNode_YapFragmentWidget::IsChecked_UseProjectDefaultTimeSettingsButton() const
{
	return GetFragment().Bit.GetUseProjectDefaultTimeSettings() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SFlowGraphNode_YapFragmentWidget::OnCheckStateChanged_UseProjectDefaultTimeSettingsButton(ECheckBoxState CheckBoxState)
{
	FYapTransactions::BeginModify(LOCTEXT("NodeUseProjectDefaultTimeSettings", "Use Project Default Time Settings Changed"), GetFlowYapDialogueNode());

	GetFragment().Bit.SetUseProjectDefaultSettings(CheckBoxState == ECheckBoxState::Checked);
	
	FYapTransactions::EndModify();
}

// ---------------------
bool SFlowGraphNode_YapFragmentWidget::IsEnabled_UseManuallyEnteredTimeButton() const
{
	return GetFragment().Bit.GetUseProjectDefaultTimeSettings() ? false : true;
}

ECheckBoxState SFlowGraphNode_YapFragmentWidget::IsChecked_UseManuallyEnteredTimeButton() const
{
	return GetFragment().Bit.GetTimeMode() == EYapTimeMode::ManualTime ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SFlowGraphNode_YapFragmentWidget::OnCheckStateChanged_UseManuallyEnteredTimeButton(ECheckBoxState CheckBoxState)
{
	FYapTransactions::BeginModify(LOCTEXT("Fragment", "Fragment Time Mode Changed"), GetFlowYapDialogueNode());

	if (CheckBoxState == ECheckBoxState::Checked)
	{
		GetFragment().Bit.SetBitTimeMode(EYapTimeMode::ManualTime);
	}

	FYapTransactions::EndModify();
}

// ---------------------
bool SFlowGraphNode_YapFragmentWidget::IsEnabled_UseTextTimeButton() const
{
	return GetFragment().Bit.GetUseProjectDefaultTimeSettings() ? false : true;
}

ECheckBoxState SFlowGraphNode_YapFragmentWidget::IsChecked_UseTextTimeButton() const
{
	return GetFragment().Bit.GetTimeMode() == EYapTimeMode::TextTime ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SFlowGraphNode_YapFragmentWidget::OnCheckStateChanged_UseTextTimeButton(ECheckBoxState CheckBoxState)
{
	FYapTransactions::BeginModify(LOCTEXT("Fragment", "Fragment Time Mode Changed"), GetFlowYapDialogueNode());

	if (CheckBoxState == ECheckBoxState::Checked)
	{
		GetFragment().Bit.SetBitTimeMode(EYapTimeMode::TextTime);
	}
	
	FYapTransactions::EndModify();
}

// ---------------------
bool SFlowGraphNode_YapFragmentWidget::IsEnabled_UseAudioTimeButton() const
{
	return !GetFragment().Bit.GetUseProjectDefaultTimeSettings();
}

ECheckBoxState SFlowGraphNode_YapFragmentWidget::IsChecked_UseAudioTimeButton() const
{
	return GetFragment().Bit.GetTimeMode() == EYapTimeMode::AudioTime ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SFlowGraphNode_YapFragmentWidget::OnCheckStateChanged_UseAudioTimeButton(ECheckBoxState CheckBoxState)
{
	FYapTransactions::BeginModify(LOCTEXT("NodeTimedModeChanged", "Timed Mode Changed"), GetFlowYapDialogueNode());

	if (CheckBoxState == ECheckBoxState::Checked)
	{
		GetFragment().Bit.SetBitTimeMode(EYapTimeMode::AudioTime);
	}
	
	FYapTransactions::EndModify();
}

bool SFlowGraphNode_YapFragmentWidget::IsEnabled_TimeEntryBox() const
{
	return GetFragment().Bit.GetTimeMode() == EYapTimeMode::ManualTime;
}

TOptional<double> SFlowGraphNode_YapFragmentWidget::Value_TimeEntryBox() const
{
	double Time = GetFragment().Bit.GetTime();
	
	return (Time > 0) ? Time : TOptional<double>();
}

void SFlowGraphNode_YapFragmentWidget::OnValueCommitted_TimeEntryBox(double NewValue, ETextCommit::Type CommitType)
{
	FYapTransactions::BeginModify(LOCTEXT("NodeEnteredTimeChanged", "Entered Time Changed"), GetFlowYapDialogueNode());

	if (CommitType == ETextCommit::OnEnter || CommitType == ETextCommit::OnUserMovedFocus)
	{
		GetFragment().Bit.SetManualTime(NewValue);
	}
	else if (CommitType == ETextCommit::OnCleared)
	{
		GetFragment().Bit.SetManualTime(0.0);
	}

	FYapTransactions::EndModify();
}

// ================================================================================================
// AUDIO ASSET WIDGET
// ------------------------------------------------------------------------------------------------

TSharedRef<SWidget> SFlowGraphNode_YapFragmentWidget::CreateAudioAssetWidget()
{
	UClass* DialogueAssetClass = UYapProjectSettings::Get()->GetDialogueAssetClass();

	if (!DialogueAssetClass)
	{
		DialogueAssetClass = UObject::StaticClass();
	}

	
	GetFragment().GetBit().GetDialogueAudioAsset_SoftPtr<USoundBase>();
	
	TSharedRef<SObjectPropertyEntryBox> AudioAssetProperty = SNew(SObjectPropertyEntryBox)
		.DisplayBrowse(true)
		.DisplayUseSelected(false)
		.DisplayThumbnail(false)
		.AllowedClass(DialogueAssetClass)
		.EnableContentPicker(true)
		.ObjectPath(this, &SFlowGraphNode_YapFragmentWidget::ObjectPath_AudioAsset)
		.OnObjectChanged(this, &SFlowGraphNode_YapFragmentWidget::OnObjectChanged_AudioAsset)
		.ToolTipText(LOCTEXT("DialogueAudioAsset_Tooltip", "Select an audio asset."));
	
	TSharedRef<SWidget> Widget = SNew(SOverlay)
	+ SOverlay::Slot()
	.HAlign(HAlign_Fill)
	.VAlign(VAlign_Fill)
	[
		AudioAssetProperty
	]
	+ SOverlay::Slot()
	.HAlign(HAlign_Fill)
	.VAlign(VAlign_Fill)
	[
		SNew(SImage)
		.Image(FAppStyle::GetBrush("MarqueeSelection"))
		.Visibility(this, &SFlowGraphNode_YapFragmentWidget::Visibility_AudioAssetErrorState)
		.ColorAndOpacity(this, &SFlowGraphNode_YapFragmentWidget::ColorAndOpacity_AudioAssetErrorState)
	];
	
	return Widget;
}

FText SFlowGraphNode_YapFragmentWidget::ObjectPathText_AudioAsset() const
{
	return FText::FromString(ObjectPath_AudioAsset());
}

FString SFlowGraphNode_YapFragmentWidget::ObjectPath_AudioAsset() const
{
	const UObject* Asset = GetFragment().Bit.GetDialogueAudioAsset<UObject>();

	if (!Asset) { return ""; }

	return Asset->GetPathName();
}

void SFlowGraphNode_YapFragmentWidget::OnObjectChanged_AudioAsset(const FAssetData& InAssetData)
{
	FYapTransactions::BeginModify(LOCTEXT("NodeAudioAssetChanged", "Audio Asset Changed"), GetFlowYapDialogueNode());

	GetFragment().Bit.SetDialogueAudioAsset(InAssetData.GetAsset());

	FYapTransactions::EndModify();
}

EVisibility SFlowGraphNode_YapFragmentWidget::Visibility_AudioAssetErrorState() const
{
	if (AudioAssetErrorLevel() != EYapErrorLevel::OK)
	{
		return EVisibility::HitTestInvisible;
	}
	
	return EVisibility::Hidden;
}

FSlateColor SFlowGraphNode_YapFragmentWidget::ColorAndOpacity_AudioAssetErrorState() const
{
	switch (AudioAssetErrorLevel())
	{
	case EYapErrorLevel::OK:
		{
			return YapColor::Green;
		}
	case EYapErrorLevel::Warning:
		{
			return YapColor::Orange;
		}
	case EYapErrorLevel::Error:
		{
			return YapColor::Red;
		}
	}

	return YapColor::Black;
}

EYapErrorLevel SFlowGraphNode_YapFragmentWidget::AudioAssetErrorLevel() const
{
	UClass* AssetClass = UYapProjectSettings::Get()->GetDialogueAssetClass();

	static EYapErrorLevel CachedErrorLevel = EYapErrorLevel::OK;
	static double LastUpdateTime = 0;
	
	if (GWorld)
	{
		if (GWorld->GetRealTimeSeconds() - LastUpdateTime < 1.0)
		{
			return CachedErrorLevel;
		}		
	}
	
	const TSoftObjectPtr<UObject> Asset = GetFragment().Bit.GetDialogueAudioAsset_SoftPtr<UObject>();

	if (Asset)
	{
		if (!Asset->IsA(AssetClass))
		{
			CachedErrorLevel = EYapErrorLevel::Error;
			return CachedErrorLevel;
		}
	}

	if (GetFragment().Bit.GetTimeMode() == EYapTimeMode::AudioTime && !GetFragment().Bit.HasDialogueAudioAsset())
	{
		CachedErrorLevel = UYapProjectSettings::Get()->GetMissingAudioErrorLevel();
		return CachedErrorLevel;
	}

	CachedErrorLevel = EYapErrorLevel::OK;
	return CachedErrorLevel;
}

EVisibility SFlowGraphNode_YapFragmentWidget::Visibility_AudioButton() const
{
	if (bShowSettings)
	{
		return EVisibility::Collapsed;
	}

	return GetFragment().GetBit().HasDialogueAudioAsset() ? EVisibility::Visible : EVisibility::Collapsed;
}

// ================================================================================================
// HELPER API
// ================================================================================================

UFlowNode_YapDialogue* SFlowGraphNode_YapFragmentWidget::GetFlowYapDialogueNode() const
{
	return Owner->GetFlowYapDialogueNodeMutable();
}

FYapFragment& SFlowGraphNode_YapFragmentWidget::GetFragment() const
{
	return GetFlowYapDialogueNode()->GetFragmentByIndexMutable(FragmentIndex);
}

bool SFlowGraphNode_YapFragmentWidget::FragmentFocused() const
{
	uint8 FocusedFragmentIndex;
	return (Owner->GetFocusedFragmentIndex(FocusedFragmentIndex) && FocusedFragmentIndex == FragmentIndex);
}

EVisibility SFlowGraphNode_YapFragmentWidget::Visibility_RowHighlight() const
{
	if (GetFlowYapDialogueNode()->GetRunningFragmentIndex() == FragmentIndex)
	{
		return EVisibility::HitTestInvisible;
	}

	if (GetFragment().IsActivationLimitMet())
	{
		return EVisibility::HitTestInvisible;
	}
	
	if (GetFlowYapDialogueNode()->GetActivationState() != EFlowNodeState::Active && GetFlowYapDialogueNode()->ActivationLimitsMet())
	{
		return EVisibility::HitTestInvisible;
	}

	return EVisibility::Collapsed;
}

FSlateColor SFlowGraphNode_YapFragmentWidget::BorderBackgroundColor_RowHighlight() const
{
	if (GetFlowYapDialogueNode()->GetRunningFragmentIndex() == FragmentIndex)
	{
		return YapColor::White_Glass;
	}
	
	if (GetFragment().IsActivationLimitMet())
	{
		return YapColor::Red_Glass;
	}
	
	if (GetFlowYapDialogueNode()->GetActivationState() != EFlowNodeState::Active && GetFlowYapDialogueNode()->ActivationLimitsMet())
	{
		return YapColor::Red_Glass;
	}

	return YapColor::White_Glass;
}

void SFlowGraphNode_YapFragmentWidget::SetNodeSelected()
{
	UEdGraphNode* GraphNode = GetFlowYapDialogueNode()->GetGraphNode();
	
	TSharedPtr<SFlowGraphEditor> GraphEditor = FFlowGraphUtils::GetFlowGraphEditor(GraphNode->GetGraph());

	if (GraphEditor)
	{
		GraphEditor->SelectSingleNode(GraphNode);
	}
}

// ================================================================================================
// OVERRIDES
// ================================================================================================

FSlateColor SFlowGraphNode_YapFragmentWidget::GetNodeTitleColor() const
{
	FLinearColor Color;

	if (GetFlowYapDialogueNode()->GetDynamicTitleColor(Color))
	{
		return Color;
	}

	return FLinearColor::Black;
}

void SFlowGraphNode_YapFragmentWidget::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	if (bShowSettings && !Owner->GetIsSelected())
	{
		bShowSettings = false;
				
		CentreBox->SetContent(CentreDialogueWidget.ToSharedRef());
	}

	if (bDialogueExpanded && !Owner->GetIsSelected())
	{
		bDialogueExpanded = false;

		FragmentOverlay->RemoveSlot(ExpandedDialogueWidget.ToSharedRef());
	}
	
	if (bTitleTextExpanded && !Owner->GetIsSelected())
	{
		bTitleTextExpanded = false;

		FragmentOverlay->RemoveSlot(ExpandedTitleTextWidget.ToSharedRef());
	}
	
	if (Owner->GetIsSelected() && !IsBeingEdited() && !MoveFragmentControls.IsValid())
	{
		UE_LOG(LogYap, Verbose, TEXT("Making)))"));
		MoveFragmentControls = CreateFragmentControlsWidget();
		FragmentWidgetOverlay->AddSlot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Center)
		.Padding(-24, 0, 0, 0)
		[
			MoveFragmentControls.ToSharedRef()
		];
	}
	else if (MoveFragmentControls.IsValid() && (!Owner->GetIsSelected() || IsBeingEdited()))
	{
		UE_LOG(LogYap, Verbose, TEXT("Removinging)))"));

		FragmentWidgetOverlay->RemoveSlot(MoveFragmentControls.ToSharedRef());
		MoveFragmentControls = nullptr;
	}
}

TSharedRef<SWidget> SFlowGraphNode_YapFragmentWidget::CreateRightFragmentPane()
{
	return SNew(SVerticalBox)
	+ SVerticalBox::Slot()
	.HAlign(HAlign_Center)
	.VAlign(VAlign_Top)
	.Padding(4, 0, 2, 0)
	.AutoHeight()
	[
		SAssignNew(PromptOutPinBox, SBox)
	]
	+ SVerticalBox::Slot()
	[
		SNew(SSpacer)
	]
	+ SVerticalBox::Slot()
	.HAlign(HAlign_Center)
	.VAlign(VAlign_Bottom)
	.Padding(4, 0, 2, 0)
	.AutoHeight()
	[
		SNew(SOverlay)
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(SBox)
			.WidthOverride(16)
			.HeightOverride(8)
			.Visibility(this, &SFlowGraphNode_YapFragmentWidget::Visibility_EnableOnStartPinButton)
			[
				SNew(SButton)
				.OnClicked(this, &SFlowGraphNode_YapFragmentWidget::OnClicked_EnableOnStartPinButton)
				.ButtonColorAndOpacity(YapColor::LightGray_Trans)
				.ToolTipText(INVTEXT("Click to enable 'On Start' Pin"))
			]
		]
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SAssignNew(StartPinBox, SBox)
			.WidthOverride(16)
			.HeightOverride(16)
		]
	]
	+ SVerticalBox::Slot()
	.HAlign(HAlign_Center)
	.VAlign(VAlign_Bottom)
	.Padding(4, 0, 2, 6)
	.AutoHeight()
	[
		SNew(SOverlay)
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(SBox)
			.WidthOverride(16)
			.HeightOverride(8)
			.Visibility(this, &SFlowGraphNode_YapFragmentWidget::Visibility_EnableOnEndPinButton)
			[
				SNew(SButton)
				.OnClicked(this, &SFlowGraphNode_YapFragmentWidget::OnClicked_EnableOnEndPinButton)
				.ButtonColorAndOpacity(YapColor::LightGray_Trans)
				.ToolTipText(INVTEXT("Click to enable 'On End' Pin"))
			]
		]
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SAssignNew(EndPinBox, SBox)
			.WidthOverride(16)
			.HeightOverride(16)
		]
	];
}

TSharedPtr<SBox> SFlowGraphNode_YapFragmentWidget::GetPinContainer(const FFlowPin& Pin)
{
	if (Pin == GetFragment().GetStartPin())
	{
		return StartPinBox;
	}

	if (Pin == GetFragment().GetEndPin())
	{
		return EndPinBox;
	}

	if (Pin == GetFragment().GetPromptPin())
	{
		return PromptOutPinBox;
	}

	return nullptr;
}

EVisibility SFlowGraphNode_YapFragmentWidget::Visibility_EnableOnStartPinButton() const
{
	if (GEditor->IsPlayingSessionInEditor())
	{
		return EVisibility::Collapsed;
	}
	
	return GetFragment().UsesStartPin() ? EVisibility::Collapsed : EVisibility::Visible;
}

EVisibility SFlowGraphNode_YapFragmentWidget::Visibility_EnableOnEndPinButton() const
{
	if (GEditor->IsPlayingSessionInEditor())
	{
		return EVisibility::Collapsed;
	}
	
	return GetFragment().UsesEndPin() ? EVisibility::Collapsed : EVisibility::Visible;
}

FReply SFlowGraphNode_YapFragmentWidget::OnClicked_EnableOnStartPinButton()
{
	FYapTransactions::BeginModify(LOCTEXT("YapDialogue", "Enable OnStart Pin"), GetFlowYapDialogueNode());

	GetFragment().bShowOnStartPin = true;
	
	GetFlowYapDialogueNode()->ForceReconstruction();
	
	FYapTransactions::EndModify();

	return FReply::Handled();
}

FReply SFlowGraphNode_YapFragmentWidget::OnClicked_EnableOnEndPinButton()
{
	FYapTransactions::BeginModify(LOCTEXT("YapDialogue", "Enable OnEnd Pin"), GetFlowYapDialogueNode());

	GetFragment().bShowOnEndPin = true;

	GetFlowYapDialogueNode()->ForceReconstruction();
	
	FYapTransactions::EndModify();
	
	return FReply::Handled();
}

bool SFlowGraphNode_YapFragmentWidget::GetNodeSelected() const
{
	UFlowGraphNode* FlowGraphNode = Cast<UFlowGraphNode>(GetFlowYapDialogueNode()->GetGraphNode());

	if (IsValid(FlowGraphNode))
	{
		TSharedPtr<SFlowGraphEditor> GraphEditor = FFlowGraphUtils::GetFlowGraphEditor(FlowGraphNode->GetGraph());

		if (GraphEditor)
		{
			return GraphEditor->GetSelectedFlowNodes().Contains(FlowGraphNode);
		}
	}

	return false;
}


// ================================================================================================
// MOOD KEY SELECTOR WIDGET
// ------------------------------------------------------------------------------------------------

TSharedRef<SBox> SFlowGraphNode_YapFragmentWidget::CreateMoodKeySelectorWidget()
{
	TSharedPtr<SBox> Box;
	FMenuBuilder MenuBuilder(true, nullptr);
	FGameplayTag SelectedMoodKey = GetCurrentMoodKey();

	for (const FGameplayTag& MoodKey : UYapProjectSettings::Get()->GetMoodTags())
	{
		if (!MoodKey.IsValid())
		{
			UE_LOG(LogYap, Warning, TEXT("Warning: Portrait keys contains an invalid entry. Clean this up!"));
			continue;
		}
		
		bool bSelected = MoodKey == SelectedMoodKey;
		MenuBuilder.AddWidget(CreateMoodKeyMenuEntryWidget(MoodKey, bSelected), FText::GetEmpty());
	}

	TSharedPtr<SImage> PortraitIconImage;
	
	FString IconPath = UYapProjectSettings::Get()->GetPortraitIconPath(GetCurrentMoodKey());

	// TODO ensure that system works and displays labels if user does not supply icons but only FNames. Use Generic mood icon?
	SAssignNew(Box, SBox)
	.WidthOverride(24)
	[
		SNew(SComboButton)
		.HasDownArrow(false)
		.ContentPadding(FMargin(0.f, 0.f))
		.MenuPlacement(MenuPlacement_CenteredBelowAnchor)
		.ButtonColorAndOpacity(FSlateColor(FLinearColor(0.f, 0.f, 0.f, 0.75f)))
		.HAlign(HAlign_Center)
		.ButtonStyle(FAppStyle::Get(), "SimpleButton")
		.OnMenuOpenChanged(this, &SFlowGraphNode_YapFragmentWidget::OnMenuOpenChanged_MoodKeySelector)
		.ToolTipText(LOCTEXT("Fragment", "Choose a mood key"))
		.ButtonContent()
		[
			SNew(SBox)
			.Padding(2, 2)
			[
				SAssignNew(PortraitIconImage, SImage)
				.ColorAndOpacity(FSlateColor::UseForeground())
				.Image(this, &SFlowGraphNode_YapFragmentWidget::Image_MoodKeySelector)
			]
		]
		.MenuContent()
		[
			MenuBuilder.MakeWidget()
		]
	];
	
	return Box.ToSharedRef();
}

EVisibility SFlowGraphNode_YapFragmentWidget::Visibility_MoodKeySelector() const
{
	return IsHovered() || MoodKeySelectorMenuOpen ? EVisibility::Visible : EVisibility::Collapsed;
}

void SFlowGraphNode_YapFragmentWidget::OnMenuOpenChanged_MoodKeySelector(bool bMenuOpen)
{
	MoodKeySelectorMenuOpen = bMenuOpen;
}

const FSlateBrush* SFlowGraphNode_YapFragmentWidget::Image_MoodKeySelector() const
{
	return GEditor->GetEditorSubsystem<UYapEditorSubsystem>()->GetMoodKeyBrush(GetCurrentMoodKey());
}

FGameplayTag SFlowGraphNode_YapFragmentWidget::GetCurrentMoodKey() const
{
	return GetFragment().Bit.GetMoodKey();
}

// ================================================================================================
// MOOD KEY MENU ENTRY WIDGET
// ------------------------------------------------------------------------------------------------

TSharedRef<SWidget> SFlowGraphNode_YapFragmentWidget::CreateMoodKeyMenuEntryWidget(FGameplayTag MoodKey, bool bSelected, const FText& InLabel, FName InTextStyle)
{
	const UYapProjectSettings* ProjectSettings = UYapProjectSettings::Get();
		
	TSharedPtr<SHorizontalBox> HBox = SNew(SHorizontalBox);

	TSharedPtr<SImage> PortraitIconImage;
		
	FString IconPath = ProjectSettings->GetPortraitIconPath(MoodKey);

	// TODO this is dumb, cache FSlateIcons or FSlateBrushes in the subsystem instead?
	UTexture2D* MoodKeyIcon = GEditor->GetEditorSubsystem<UYapEditorSubsystem>()->GetMoodKeyIcon(MoodKey);
	
	FSlateBrush Brush;
	Brush.ImageSize = FVector2D(16, 16);
	Brush.SetResourceObject(MoodKeyIcon);
	
	TSharedRef<FDeferredCleanupSlateBrush> MoodKeyBrush = FDeferredCleanupSlateBrush::CreateBrush(Brush);
	
	if (MoodKey.IsValid())
	{
		HBox->AddSlot()
		.AutoWidth()
		.Padding(0, 0, 0, 0)
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Center)
		[
			SAssignNew(PortraitIconImage, SImage)
			.ColorAndOpacity(FSlateColor::UseForeground())
			.Image(TAttribute<const FSlateBrush*>::Create(TAttribute<const FSlateBrush*>::FGetter::CreateLambda([MoodKeyBrush](){return MoodKeyBrush->GetSlateBrush();})))
		];
	}

	// TODO ensure that system works and displays labels if user does not supply icons but only FNames
	if (!InLabel.IsEmpty())
	{
		HBox->AddSlot()	
		 .VAlign(VAlign_Center)
		.Padding(0.f, 0.f, 0.f, 0.f)
		.AutoWidth()
		[
			SNew(STextBlock)
			.TextStyle( &FAppStyle::Get().GetWidgetStyle< FTextBlockStyle >( InTextStyle ))
			.Justification(ETextJustify::Center)
			.Text(InLabel)
		];
	}
	
	return SNew(SButton)
	.ContentPadding(FMargin(4, 4))
	.ButtonStyle(FAppStyle::Get(), "SimpleButton")
	.ButtonColorAndOpacity(FLinearColor(1,1,1,0.25))
	.ClickMethod(EButtonClickMethod::MouseDown)
	.OnClicked(this, &SFlowGraphNode_YapFragmentWidget::OnClicked_MoodKeyMenuEntry, MoodKey)
	[
		SAssignNew(PortraitIconImage, SImage)
		.ColorAndOpacity(FSlateColor::UseForeground())
		.Image(TAttribute<const FSlateBrush*>::Create(TAttribute<const FSlateBrush*>::FGetter::CreateLambda([MoodKeyBrush](){return MoodKeyBrush->GetSlateBrush();})))
	];
}

FReply SFlowGraphNode_YapFragmentWidget::OnClicked_MoodKeyMenuEntry(FGameplayTag NewValue)
{
	/*
	 *
	 *TODO
	if (InErrorState())
	{
		return FReply::Unhandled();
	}*/
	
	FYapTransactions::BeginModify(LOCTEXT("NodeMoodKeyChanged", "Portrait Key Changed"), GetFlowYapDialogueNode());

	GetFragment().Bit.SetMoodKey(NewValue);

	FYapTransactions::EndModify();

	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE