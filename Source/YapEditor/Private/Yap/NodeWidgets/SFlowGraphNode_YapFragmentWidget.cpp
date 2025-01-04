// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
#define LOCTEXT_NAMESPACE "FlowYap"
#include "Yap/NodeWidgets/SFlowGraphNode_YapFragmentWidget.h"

#include "Engine/World.h"
#include "PropertyCustomizationHelpers.h"
#include "SAssetDropTarget.h"
#include "SLevelOfDetailBranchNode.h"
#include "Graph/FlowGraphEditor.h"
#include "Graph/FlowGraphUtils.h"
#include "Slate/DeferredCleanupSlateBrush.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "Widgets/Input/SSlider.h"
#include "Widgets/Layout/SPopup.h"
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
#include "Yap/YapEditorSettings.h"
#include "Yap/YapInputTracker.h"
#include "Yap/YapSubsystem.h"
#include "Yap/Helpers/SYapTextPropertyEditableTextBox.h"
#include "Yap/Helpers/YapEditableTextPropertyHandle.h"
#include "Yap/NodeWidgets/SActivationCounterWidget.h"
#include "Yap/NodeWidgets/SMaturityCheckBox.h"
#include "Yap/NodeWidgets/SYapConditionsScrollBox.h"
#include "Yap/NodeWidgets/SSkippableCheckBox.h"
#include "Yap/NodeWidgets/SYapTest.h"
#include "Yap/Testing/SYapPropertyMenuAssetPicker.h"

bool SFlowGraphNode_YapFragmentWidget::UseChildSafeSettings() const
{
	return GetFragment().bHasChildSafeData;
}

bool SFlowGraphNode_YapFragmentWidget::HasAnyChildSafeData() const
{
	const FYapBit& Bit = GetFragment().GetBit();

	bool bHasSafeDialogueText = !Bit.GetSafeDialogueText().IsEmpty();
	bool bHasSafeTitleText = !Bit.GetSafeTitleText().IsEmpty();
	bool bHasSafeAudio = Bit.HasDialogueAudioAssetSafe();

	return (bHasSafeDialogueText || bHasSafeTitleText || bHasSafeAudio);
}

bool SFlowGraphNode_YapFragmentWidget::HasCompleteChildSafeData() const
{
	const FYapBit& Bit = GetFragment().GetBit();
	
	bool bHasSafeDialogueText = !Bit.GetSafeDialogueText().IsEmpty();
	bool bHasSafeTitleText = Bit.GetMatureTitleText().IsEmpty() ? true : !Bit.GetSafeTitleText().IsEmpty();
	bool bHasSafeAudio = !Bit.HasDialogueAudioAsset() ? true : Bit.HasDialogueAudioAssetSafe();

	return bHasSafeDialogueText && bHasSafeTitleText && bHasSafeAudio;
}

FReply SFlowGraphNode_YapFragmentWidget::OnClicked_DialogueCornerButton()
{
	bShowAudioSettings = !bShowAudioSettings;

	if (bShowAudioSettings)
	{
		if (CenterSettingsWidget == nullptr)
		{
			CenterSettingsWidget = CreateCenterSettingsWidget();
		}
		
		CentreBox->SetContent(CenterSettingsWidget.ToSharedRef());

		Owner->SetNodeSelected();
	}
	else
	{
		if (CentreDialogueWidget == nullptr)
		{
			CentreDialogueWidget = CreateCentreTextDisplayWidget();
		}
		CentreBox->SetContent(CentreDialogueWidget.ToSharedRef());
	}

	return FReply::Handled();
}

TSharedPtr<SWidget> SFlowGraphNode_YapFragmentWidget::CreateCentreTextDisplayWidget()
{	
	return SNew(SOverlay)
	+ SOverlay::Slot()
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.Padding(0, 0, 0, 0)
		.VAlign(VAlign_Fill)
		.HAlign(HAlign_Fill)
		[
			CreateDialogueDisplayWidget()
		]
		+ SVerticalBox::Slot()
		.Padding(0, 4, 0, 0)
		.AutoHeight()
		[
			CreateTitleTextDisplayWidget()
		]
	]
	+ SOverlay::Slot()
	.HAlign(HAlign_Right)
	.VAlign(VAlign_Top)
	.Padding(0, 0, 0, 0)
	[
		SNew(SButton)
		.Cursor(EMouseCursor::Default)
		.ButtonStyle(FYapEditorStyle::Get(), YapStyles.ButtonStyle_DialogueCornerFoldout)
		.OnClicked(this, &SFlowGraphNode_YapFragmentWidget::OnClicked_DialogueCornerButton)
		.ContentPadding(0)
		[
			SNew(SImage)
			.Image(FYapEditorStyle::GetImageBrush(YapBrushes.Icon_SettingsExpander))
			.ColorAndOpacity(FSlateColor::UseForeground())
		]
	]
	+ SOverlay::Slot()
	.VAlign(VAlign_Bottom)
	.HAlign(HAlign_Right)
	.Padding(-1)
	[
		CreateAudioPreviewWidget(&GetBit().MatureDialogueAudioAsset, TAttribute<EVisibility>::CreateSP(this, &SFlowGraphNode_YapFragmentWidget::Visibility_AudioButton))
	];
}

TSharedPtr<SWidget> SFlowGraphNode_YapFragmentWidget::CreateCenterSettingsWidget()
{
	//CreateMoodKeySelectorWidget()

	return SNew(SVerticalBox)
	.Cursor(EMouseCursor::Default)
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
			CreateAudioAssetWidget
			(
				TAttribute<EVisibility>::CreateLambda
				([this] ()
				{
					return EVisibility::Visible;
				}),
				TAttribute<FString>::CreateLambda
				([this] ()
				{
					return GetFragment().GetBit().GetDialogueAudioAsset<UObject>()->GetPathName();
				}),
				TDelegate<void(const FAssetData&)>::CreateLambda
				([this] (const FAssetData& InAssetData)
				{
					FYapTransactions::BeginModify(INVTEXT("Setting audio asset"), GetDialogueNode());

					GetFragment().GetBitMutable().SetDialogueAudioAsset(InAssetData.GetAsset());

					FYapTransactions::EndModify();
				})
			)
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			CreateAudioPreviewWidget(&GetBit().SafeDialogueAudioAsset, EVisibility::Visible)
		]
	]
	+ SVerticalBox::Slot()
	.VAlign(VAlign_Top)
	.HAlign(HAlign_Fill)
	.AutoHeight()
		.Padding(0, 1, 0, 1)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.HAlign(HAlign_Fill)
		[
			CreateAudioAssetWidget
			(
				TAttribute<EVisibility>::CreateLambda
				([this] ()
				{
					return UseChildSafeSettings() ? EVisibility::Visible : EVisibility::Collapsed;
				}),
				TAttribute<FString>::CreateLambda
				([this] ()
				{
					return GetFragment().GetBit().GetDialogueAudioAssetSafe<UObject>()->GetPathName();
				}),
				TDelegate<void(const FAssetData&)>::CreateLambda
				([this] (const FAssetData& InAssetData)
				{
					FYapTransactions::BeginModify(INVTEXT("Setting audio asset"), GetDialogueNode());
					
					GetFragment().GetBitMutable().SetDialogueAudioAssetSafe(InAssetData.GetAsset());

					FYapTransactions::EndModify();
				}
				)
			)
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			CreateAudioPreviewWidget(&GetBit().SafeDialogueAudioAsset, TAttribute<EVisibility>::CreateLambda
				([this] ()
				{
					return UseChildSafeSettings() ? EVisibility::Visible : EVisibility::Collapsed;
				}))
		]
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
	return bTextEditorExpanded || bShowAudioSettings;
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
	if (GEditor->PlayWorld)
	{
		return EVisibility::Collapsed;
	}

	if (Owner->HasActiveOverlay())
	{
		return EVisibility::Collapsed;
	}
	
	return GetDialogueNode()->GetNumFragments() > 1 ? EVisibility::Visible : EVisibility::Collapsed;
}

EVisibility SFlowGraphNode_YapFragmentWidget::Visibility_FragmentShiftWidget(EYapFragmentControlsDirection YapFragmentControlsDirection) const
{
	if (FragmentIndex == 0 && YapFragmentControlsDirection == EYapFragmentControlsDirection::Up)
	{
		return EVisibility::Hidden;
	}

	if (FragmentIndex == GetDialogueNode()->GetNumFragments() - 1 && YapFragmentControlsDirection == EYapFragmentControlsDirection::Down)
	{
		return EVisibility::Hidden;
	}

	return EVisibility::Visible;
}

FReply SFlowGraphNode_YapFragmentWidget::OnClicked_FragmentShift(EYapFragmentControlsDirection YapFragmentControlsDirection)
{
	int32 OtherIndex = YapFragmentControlsDirection == EYapFragmentControlsDirection::Up ? FragmentIndex - 1 : FragmentIndex + 1;
	
	GetDialogueNode()->SwapFragments(FragmentIndex, OtherIndex);

	return FReply::Handled();
}

FReply SFlowGraphNode_YapFragmentWidget::OnClicked_FragmentDelete()
{
	GetDialogueNode()->DeleteFragmentByIndex(FragmentIndex);

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
			.Cursor(EMouseCursor::Default)
			.ButtonStyle(FYapEditorStyle::Get(), YapStyles.ButtonStyle_FragmentControls)
			.ContentPadding(FMargin(3, 4))
			.ToolTipText(LOCTEXT("DialogueMoveFragmentUp_Tooltip", "Move Fragment Up"))
			.Visibility(this, &SFlowGraphNode_YapFragmentWidget::Visibility_FragmentShiftWidget, EYapFragmentControlsDirection::Up)
			.OnClicked(this, &SFlowGraphNode_YapFragmentWidget::OnClicked_FragmentShift, EYapFragmentControlsDirection::Up)
			[
				SNew(SImage)
				//.Image(FYapEditorStyle::GetImageBrush(YapBrushes.Icon_UpArrow))
				.Image(FAppStyle::Get().GetBrush("Icons.ChevronUp"))
				.DesiredSizeOverride(FVector2D(16, 16))
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
			.Cursor(EMouseCursor::Default)
			.ButtonStyle(FYapEditorStyle::Get(), YapStyles.ButtonStyle_FragmentControls)
			.ContentPadding(FMargin(3, 4))
			.ToolTipText(LOCTEXT("DialogueDeleteFragment_Tooltip", "Delete Fragment"))
			.OnClicked(this, &SFlowGraphNode_YapFragmentWidget::OnClicked_FragmentDelete)
			//.ButtonColorAndOpacity(YapColor::Red)
			[
				SNew(SImage)
				//.Image(FYapEditorStyle::GetImageBrush(YapBrushes.Icon_Delete))
				.Image(FAppStyle::Get().GetBrush("Icons.Delete"))
				.DesiredSizeOverride(FVector2D(16, 16))
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
			.Cursor(EMouseCursor::Default)
			.ButtonStyle(FYapEditorStyle::Get(), YapStyles.ButtonStyle_FragmentControls)
			.ContentPadding(FMargin(3, 4))
			.ToolTipText(LOCTEXT("DialogueMoveFragmentDown_Tooltip", "Move Fragment Down"))
			.Visibility(this, &SFlowGraphNode_YapFragmentWidget::Visibility_FragmentShiftWidget, EYapFragmentControlsDirection::Down)
			.OnClicked(this, &SFlowGraphNode_YapFragmentWidget::OnClicked_FragmentShift, EYapFragmentControlsDirection::Down)
			[
				SNew(SImage)
				//.Image(FYapEditorStyle::GetImageBrush(YapBrushes.Icon_DownArrow))
				.Image(FAppStyle::Get().GetBrush("Icons.ChevronDown"))
				.DesiredSizeOverride(FVector2D(16, 16))
				.ColorAndOpacity(FSlateColor::UseForeground())
			]
		]
	];
}

bool SFlowGraphNode_YapFragmentWidget::Enabled_AudioPreviewButton() const
{
	return GetFragment().GetBit().HasDialogueAudioAsset();
}

TSharedRef<SWidget> SFlowGraphNode_YapFragmentWidget::CreateAudioPreviewWidget(const TSoftObjectPtr<UObject>* AudioAsset, TAttribute<EVisibility> Attribute)
{
	return SNew(SButton)
	.Cursor(EMouseCursor::Default)
	.ContentPadding(2)
	.ButtonStyle(FYapEditorStyle::Get(), YapStyles.ButtonStyle_AudioPreview)
	.Visibility(Attribute)
	.IsEnabled(this, &SFlowGraphNode_YapFragmentWidget::Enabled_AudioPreviewButton)
	.ToolTipText(INVTEXT("Play audio"))
	[
		SNew(SOverlay)
		+ SOverlay::Slot()
		[
			SNew(SImage)
			.DesiredSizeOverride(FVector2D(18, 18))
			.Image(FAppStyle::GetBrush("Icons.FilledCircle"))
			.ColorAndOpacity(YapColor::Black_SemiTrans)
		]
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(SImage)
			.DesiredSizeOverride(FVector2D(12, 12))
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
	FYapTransactions::BeginModify(LOCTEXT("Dialogue", "Change Dialogue Activation Limit"), GetDialogueNode());

	GetFragment().ActivationLimit = FCString::Atoi(*Text.ToString());

	GetDialogueNode()->OnReconstructionRequested.Execute();
	
	FYapTransactions::EndModify();
}

EVisibility SFlowGraphNode_YapFragmentWidget::Visibility_FragmentRowNormalControls() const
{
	if (bTextEditorExpanded)
	{
		return EVisibility::Hidden;
	}
	
	return EVisibility::Visible;
}

TSharedRef<SWidget> SFlowGraphNode_YapFragmentWidget::CreateUpperFragmentBar()
{
	UFlowNode_YapDialogue* DialogueNode = GetDialogueNode();
	
	TSharedRef<SWidget> Box = SNew(SBox)
	.Visibility(this, &SFlowGraphNode_YapFragmentWidget::Visibility_UpperFragmentBar)
	.Padding(0, 0, 32, 4)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Center)
		.Padding(6, 0, 0, 0)
		.AutoWidth()
		[
			SNew(SBox)
			.WidthOverride(20)
			.HeightOverride(20)
		]
		+ SHorizontalBox::Slot()
		.HAlign(HAlign_Fill)
		.Padding(5, 0, 0, 0)
		[
			SAssignNew(ConditionsScrollBox, SYapConditionsScrollBox)
			.DialogueNode(DialogueNode)
			.FragmentIndex(FragmentIndex)
			.ConditionsArrayProperty(FindFProperty<FArrayProperty>(FYapFragment::StaticStruct(), GET_MEMBER_NAME_CHECKED(FYapFragment, Conditions)))
			.ConditionsContainer(&GetFragment())
			.OnConditionsArrayChanged(Owner, &SFlowGraphNode_YapDialogueWidget::OnConditionsArrayChanged)
			.OnConditionDetailsViewBuilt(Owner, &SFlowGraphNode_YapDialogueWidget::OnConditionDetailsViewBuilt)
		]
		+ SHorizontalBox::Slot()
		.HAlign(HAlign_Right)
		.AutoWidth()
		.VAlign(VAlign_Fill)
		.Padding(4, 0, 5, 0)
		[
			SNew(SLevelOfDetailBranchNode)
			.UseLowDetailSlot(Owner, &SFlowGraphNode_YapDialogueWidget::UseLowDetail)
			.HighDetail()
			[
				SNew(SBox)
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				.Visibility(this, &SFlowGraphNode_YapFragmentWidget::Visibility_FragmentTagWidget)
				[
					CreateFragmentTagWidget()
				]
			]
		]
		+ SHorizontalBox::Slot()
		.HAlign(HAlign_Right)
		.AutoWidth()
		.Padding(2, -2, -27, -2)
		[
			SNew(SBox)
			.WidthOverride(20)
			.HAlign(HAlign_Center)
			[
				YapEditor::CreateSkippableCheckBox
				(
					TAttribute<ECheckBoxState>::CreateSP(this, &SFlowGraphNode_YapFragmentWidget::IsChecked_SkippableToggle),
					FOnCheckStateChanged::CreateSP(this, &SFlowGraphNode_YapFragmentWidget::OnCheckStateChanged_SkippableToggle),
					TAttribute<FSlateColor>::CreateSP(this, &SFlowGraphNode_YapFragmentWidget::ColorAndOpacity_SkippableToggleIcon),
					TAttribute<EVisibility>::CreateSP(this, &SFlowGraphNode_YapFragmentWidget::Visibility_SkippableToggleIconOff)
				)
			]
		]
	];
	
	OnConditionsUpdated();

	return Box;
}

EVisibility SFlowGraphNode_YapFragmentWidget::Visibility_FragmentTagWidget() const
{
	return GetDialogueNode()->GetDialogueTag().IsValid() ? EVisibility::Visible : EVisibility::Collapsed;
}

ECheckBoxState SFlowGraphNode_YapFragmentWidget::IsChecked_SkippableToggle() const
{
	switch (GetFragment().GetBit().GetSkippable())
	{
		case EYapDialogueSkippable::Default:
		{
			return ECheckBoxState::Undetermined;
		}
		case EYapDialogueSkippable::NotSkippable:
		{
			return ECheckBoxState::Unchecked;
		}
		case EYapDialogueSkippable::Skippable:
		{
			return ECheckBoxState::Checked;
		}
		default:
		{
			check(false);
		}
	}
	return ECheckBoxState::Undetermined;
}

FSlateColor SFlowGraphNode_YapFragmentWidget::ColorAndOpacity_SkippableToggleIcon() const
{
	EYapDialogueSkippable SkippableSetting = GetFragment().GetBit().GetSkippable();
	
	if (SkippableSetting == EYapDialogueSkippable::NotSkippable)
	{
		return YapColor::Red;
	}
	else if (SkippableSetting == EYapDialogueSkippable::Skippable)
	{
		return YapColor::Green;
	}
	else
	{
		return YapColor::DarkGray;
	}
}

EVisibility SFlowGraphNode_YapFragmentWidget::Visibility_SkippableToggleIconOff() const
{
	switch (GetFragment().GetBit().GetSkippable())
	{
		case EYapDialogueSkippable::Default:
		{
			EYapDialogueSkippable DefaultSkippable = GetDialogueNode()->GetSkippableSetting();

			switch (DefaultSkippable)
			{
				case EYapDialogueSkippable::Default:
				{
					return UYapProjectSettings::Get()->GetDialogueSkippableByDefault() ? EVisibility::Collapsed : EVisibility::Visible;
				}
				case EYapDialogueSkippable::Skippable:
				{
					return EVisibility::Collapsed;
				}
				case EYapDialogueSkippable::NotSkippable:
				{
					return EVisibility::Visible;
				}
				default:
				{
					check(false);
				}
			}
		}
		case EYapDialogueSkippable::Skippable:
		{
			return EVisibility::Collapsed;
		}
		case EYapDialogueSkippable::NotSkippable:
		{
			return EVisibility::Visible;
		}
		default:
		{
			check(false);
		}
	}

	return EVisibility::Collapsed;
}

void SFlowGraphNode_YapFragmentWidget::OnCheckStateChanged_SkippableToggle(ECheckBoxState CheckBoxState)
{
	FYapTransactions::BeginModify(LOCTEXT("YapDialogue", "Toggle Skippable"), GetDialogueNode());

	if (GEditor->GetEditorSubsystem<UYapEditorSubsystem>()->GetInputTracker()->GetControlPressed())
	{
		GetFragment().GetBitMutable().Skippable = EYapDialogueSkippable::Default;
	}
	else if (CheckBoxState == ECheckBoxState::Checked)
	{
		GetFragment().GetBitMutable().Skippable = EYapDialogueSkippable::Skippable;
	}
	else
	{
		GetFragment().GetBitMutable().Skippable = EYapDialogueSkippable::NotSkippable;
	}	
	
	FYapTransactions::EndModify();
}

ECheckBoxState SFlowGraphNode_YapFragmentWidget::IsChecked_MaturitySettings() const
{
	if (!UseChildSafeSettings())
	{
		return ECheckBoxState::Unchecked;
	}

	if (HasCompleteChildSafeData())
	{
		return ECheckBoxState::Checked;
	}

	if (HasAnyChildSafeData())
	{
		return ECheckBoxState::Undetermined;
	}

	return ECheckBoxState::Unchecked;
}

void SFlowGraphNode_YapFragmentWidget::OnCheckStateChanged_MaturitySettings(ECheckBoxState CheckBoxState)
{
	FYapTransactions::BeginModify(INVTEXT("Change child-safety settings"), GetDialogueNode());
	
	if (UseChildSafeSettings() && bCtrlPressed)
	{
		if (IsChildSafeInErrorState())
		{
			GetFragment().bIgnoreChildSafeErrors = !GetFragment().bIgnoreChildSafeErrors;
		}
		else
		{
			// Ignore ctrl-clicking on a widget that is not in an error state 
		}
	}
	else
	{
		if (!UseChildSafeSettings())
		{
			// Turn on child safety settings
			GetFragment().bHasChildSafeData = true;
			GetFragment().bIgnoreChildSafeErrors = false;
			bEditingChildSafeSettings = true;
		}
		else
		{
			// Only turn off child safety settings if we don't have any data assigned, otherwise just toggle whether we're editing it
			if (!HasAnyChildSafeData())
			{
				GetFragment().bHasChildSafeData = false;
				bEditingChildSafeSettings = false;
			}
			else
			{
				bEditingChildSafeSettings = !bEditingChildSafeSettings;
			}
		}
	}

	if (bEditingChildSafeSettings)
	{
		Owner->SetNodeSelected();
	}
	
	FYapTransactions::EndModify();
}

FSlateColor SFlowGraphNode_YapFragmentWidget::ColorAndOpacity_ChildSafeSettingsCheckBox() const
{
	const FYapBit& Bit = GetFragment().GetBit();

	if (!UseChildSafeSettings())
	{
		return YapColor::DarkGray;
	}

	if (IsChildSafeInErrorState())
	{
		return GetFragment().bIgnoreChildSafeErrors ? YapColor::LightGreen_SemiGlass : YapColor::OrangeRed;
	}
	
	return YapColor::LightBlue_SemiGlass;
}

bool SFlowGraphNode_YapFragmentWidget::IsChildSafeInErrorState() const
{
	if (!UseChildSafeSettings())
	{
		return false;
	}
	
	const FYapBit& Bit = GetFragment().GetBit();
	
	bool bHasDialogueTextSafe = !Bit.GetSafeDialogueText().IsEmpty();
	bool bHasTitleTextSafe = !Bit.GetSafeTitleText().IsEmpty();
	bool bHasAudioSafe = Bit.HasDialogueAudioAssetSafe();
	
	if (!bHasDialogueTextSafe && !bHasTitleTextSafe && !bHasAudioSafe)
	{
		return true;
	}
	
	bool bHasDialogueText = !Bit.GetMatureDialogueText().IsEmpty();
	bool bHasTitleText = !Bit.GetMatureTitleText().IsEmpty();
	bool bHasAudio = Bit.HasDialogueAudioAsset();
	
	if ((bHasDialogueText ? bHasDialogueTextSafe : true) && (bHasTitleText ? bHasTitleTextSafe : true) && (bHasAudio ? bHasAudioSafe : true))
	{
		return false;
	}

	return true;
}

FSlateColor SFlowGraphNode_YapFragmentWidget::BorderBackgroundColor_DirectedAtImage() const
{
	const FYapBit& Bit = GetFragment().GetBit();

	FLinearColor Color;
	
	if (Bit.GetDirectedAtAsset().IsValid())
	{
		Color = Bit.GetDirectedAtAsset().Get()->GetEntityColor();
	}
	else
	{
		Color = YapColor::Transparent;		
	}

	float A = UYapEditorSettings::Get()->GetPortraitBorderAlpha();
	
	Color.R *= A;
	Color.G *= A;
	Color.B *= A;

	return Color;
}

void SFlowGraphNode_YapFragmentWidget::OnAssetsDropped_DirectedAtWidget(const FDragDropEvent& DragDropEvent, TArrayView<FAssetData> AssetDatas)
{
	if (AssetDatas.Num() != 1)
	{
		return;
	}

	UObject* Object = AssetDatas[0].GetAsset();
	
	if (UYapCharacter* Character = Cast<UYapCharacter>(Object))
	{
		FYapTransactions::BeginModify(INVTEXT("Setting character"), GetDialogueNode());

		GetBit().SetDirectedAt(Character);

		FYapTransactions::EndModify();
	}
}

bool SFlowGraphNode_YapFragmentWidget::OnAreAssetsAcceptableForDrop_DirectedAtWidget(TArrayView<FAssetData> AssetDatas) const
{
	if (AssetDatas.Num() != 1)
	{
		return false;
	}

	UClass* Class = AssetDatas[0].GetClass();

	if (Class == UYapCharacter::StaticClass())
	{
		return true;
	}

	return false;
}

FReply SFlowGraphNode_YapFragmentWidget::OnClicked_DirectedAtWidget()
{
	if (bCtrlPressed)
	{
		FYapTransactions::BeginModify(INVTEXT("Clearing Directed At Character"), GetDialogueNode());

		GetBit().SetDirectedAt(nullptr);

		FYapTransactions::EndModify();

		return FReply::Handled();
	}
	
	TSharedPtr<SWidget> Widget = SNew(SBorder)
		.Padding(1, 1, 1, 1) // No idea why but the details view already has a 4 pixel transparent area on top
		.BorderImage(FYapEditorStyle::GetImageBrush(YapBrushes.Box_SolidLightGray_Rounded))
		.BorderBackgroundColor(YapColor::DimGray)
		[
			SNew(SYapPropertyMenuAssetPicker)
			.AllowedClasses({UYapCharacter::StaticClass()})
			.AllowClear(true)
			.InitialObject(GetBit().GetDirectedAt())
			.OnSet(this, &SFlowGraphNode_YapFragmentWidget::OnSetNewDirectedAtAsset)
		];

	Owner->AddOverlayWidget(DirectedAtWidget, Widget);
	
	return FReply::Handled();
}

const FSlateBrush* SFlowGraphNode_YapFragmentWidget::Image_DirectedAtWidget() const
{
	const FSlateBrush& PortraitBrush = GetFragment().GetBit().GetDirectedAtPortraitBrush();

	if (PortraitBrush.GetResourceObject())
	{
		return &PortraitBrush;
	}
	else
	{
		return nullptr;
	}
}

TSharedRef<SWidget> SFlowGraphNode_YapFragmentWidget::CreateDirectedAtWidget()
{
	int32 PortraitSize = UYapEditorSettings::Get()->GetPortraitSize() / 3;

	return SNew(SBorder)
	.HAlign(HAlign_Center)
	.VAlign(VAlign_Center)
	.BorderImage(FYapEditorStyle::GetImageBrush(YapBrushes.Panel_Rounded))
	.BorderBackgroundColor(this, &SFlowGraphNode_YapFragmentWidget::BorderBackgroundColor_DirectedAtImage)
	.Padding(0, 0, 0, 0)
	[
		SNew(SBox)
		.WidthOverride(PortraitSize + 2)
		.HeightOverride(PortraitSize + 2)
		[
			SNew(SLevelOfDetailBranchNode)
			.UseLowDetailSlot(Owner, &SFlowGraphNode_YapDialogueWidget::UseLowDetail)
			.HighDetail()
			[
				SNew(SAssetDropTarget)
				.bSupportsMultiDrop(false)
				.OnAreAssetsAcceptableForDrop(this, &SFlowGraphNode_YapFragmentWidget::OnAreAssetsAcceptableForDrop_DirectedAtWidget)
				.OnAssetsDropped(this, &SFlowGraphNode_YapFragmentWidget::OnAssetsDropped_DirectedAtWidget)
				[
					SAssignNew(DirectedAtWidget, SButton)
					.Cursor(EMouseCursor::Default)
					.ButtonStyle(FAppStyle::Get(), "SimpleButton")
					//.ToolTipText(this, &SFlowGraphNode_YapFragmentWidget::ToolTipText_PortraitWidget)
					.ContentPadding(0)
					.OnClicked(this, &SFlowGraphNode_YapFragmentWidget::OnClicked_DirectedAtWidget)
					[
						SNew(SImage)
						.DesiredSizeOverride(FVector2D(PortraitSize, PortraitSize))
						.Image(this, &SFlowGraphNode_YapFragmentWidget::Image_DirectedAtWidget)	
					]
				]
			]
			.LowDetail()
			[
				SNew(SImage)
				.DesiredSizeOverride(FVector2D(PortraitSize, PortraitSize))
				.Image(this, &SFlowGraphNode_YapFragmentWidget::Image_DirectedAtWidget)
			]
		]
	];
}

// ================================================================================================
// FRAGMENT WIDGET
// ------------------------------------------------------------------------------------------------

TSharedRef<SWidget> SFlowGraphNode_YapFragmentWidget::CreateFragmentWidget()
{
	SpeakerWidget = CreateSpeakerWidget();

	CentreBox = SNew(SBox);

	CentreDialogueWidget = CreateCentreTextDisplayWidget();
	
	CentreBox->SetContent(CentreDialogueWidget.ToSharedRef());

	return SAssignNew(FragmentWidgetOverlay, SOverlay)
	.ToolTip(nullptr)
	+ SOverlay::Slot()
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 0, 0, 3)
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
			.VAlign(VAlign_Center)
			[
				SNew(SBox)
				.WidthOverride(32)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					.Padding(0, 0, 0, 2)
					[
						SNew(SBox)
						.WidthOverride(22)
						.HeightOverride(22)
						[
							SNew(SOverlay)
							+ SOverlay::Slot()
							.HAlign(HAlign_Fill)
							.VAlign(VAlign_Fill)
							.Padding(-1)
							[
								SNew(SBorder)
								.BorderImage(FYapEditorStyle::GetImageBrush(YapBrushes.Border_Thick_RoundedSquare))
								.BorderBackgroundColor(YapColor::Yellow)
								.Visibility_Lambda( [this] () { return bEditingChildSafeSettings ? EVisibility::Visible : EVisibility::Hidden; } )
							]
							+ SOverlay::Slot()
							.HAlign(HAlign_Center)
							.VAlign(VAlign_Center)
							[
								SNew(SCheckBox)
								.Cursor(EMouseCursor::Default)
								.Style(FYapEditorStyle::Get(), YapStyles.CheckBoxStyle_Skippable)
								.Type(ESlateCheckBoxType::ToggleButton)
								.Padding(FMargin(0, 0))
								.CheckBoxContentUsesAutoWidth(true)
								.ToolTip(nullptr)
								//.ToolTipText(INVTEXT("Child-safe settings."))
								.IsChecked(this, &SFlowGraphNode_YapFragmentWidget::IsChecked_MaturitySettings)
								.OnCheckStateChanged(this, &SFlowGraphNode_YapFragmentWidget::OnCheckStateChanged_MaturitySettings)
								.Content()
								[
									SNew(SBox)
									.WidthOverride(20)
									.HeightOverride(20)
									.HAlign(HAlign_Center)
									.VAlign(VAlign_Center)
									[
										SNew(SImage)
										.ColorAndOpacity(this, &SFlowGraphNode_YapFragmentWidget::ColorAndOpacity_ChildSafeSettingsCheckBox)
										.DesiredSizeOverride(FVector2D(16, 16))
										.Image(FYapEditorStyle::GetImageBrush(YapBrushes.Icon_Baby))
									]
								]
							]
						]
					]
					+ SVerticalBox::Slot()
					[
						SNew(SBox)
						.WidthOverride(22)
						.HeightOverride(24)
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Center)
						.Padding(-2, -2, -2, -2)
						[
							SNew(SLevelOfDetailBranchNode)
							.UseLowDetailSlot(Owner, &SFlowGraphNode_YapDialogueWidget::UseLowDetail)
							.HighDetail()
							[
								SNew(SActivationCounterWidget, FOnTextCommitted::CreateSP(this, &SFlowGraphNode_YapFragmentWidget::OnTextCommitted_FragmentActivationLimit))
								.ActivationCount(this, &SFlowGraphNode_YapFragmentWidget::GetFragmentActivationCount)
								.ActivationLimit(this, &SFlowGraphNode_YapFragmentWidget::GetFragmentActivationLimit)
								.FontHeight(10)	
							]
						]
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					.Padding(0, 2, 0, 0)
					[
						SNew(SBox)
						.WidthOverride(22)
						.HeightOverride(22)
						[
							CreateMoodKeySelectorWidget()
						]
					]
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
								SpeakerWidget.ToSharedRef()
							]
							+ SOverlay::Slot()
							.VAlign(VAlign_Top)
							.HAlign(HAlign_Right)
							.Padding(-2)
							[
								CreateDirectedAtWidget()
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

EVisibility SFlowGraphNode_YapFragmentWidget::Visibility_EmptyTextIndicator(const FText* Text) const
{
	return Text->IsEmpty() ? EVisibility::HitTestInvisible : EVisibility::Hidden;
}

TSharedRef<SWidget> SFlowGraphNode_YapFragmentWidget::BuildTimeSettingsWidget()
{
	return SNew(SBorder)
	.Padding(1, 1, 1, 1)
	.BorderImage(FYapEditorStyle::GetImageBrush(YapBrushes.Box_SolidLightGray_Rounded))
	.BorderBackgroundColor(YapColor::DimGray)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.Padding(8, 8, 8, 8)
		[
			SNew(SBox)
			.WidthOverride(80)
			.VAlign(VAlign_Fill)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.Padding(0, 0, 0, 2)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					.Padding(0, 0, 8, 0)
					[
						// =============================
						// USE PROJECT DEFAULTS BUTTON
						// =============================
						SNew(SButton)
						.Cursor(EMouseCursor::Default)
						.ButtonStyle(FYapEditorStyle::Get(), YapStyles.ButtonStyle_TimeSetting)
						.ContentPadding(FMargin(4, 3))
						.ToolTipText(LOCTEXT("UseProjectDefaultTimeSettings_Tooltip", "Use time settings from project settings"))
						.OnClicked(this, &SFlowGraphNode_YapFragmentWidget::OnClicked_UseProjectDefaultTimeSettingsButton)
						.ButtonColorAndOpacity(this, &SFlowGraphNode_YapFragmentWidget::ButtonColorAndOpacity_UseProjectDefaultTimeSettingsButton)
						.ForegroundColor(this, &SFlowGraphNode_YapFragmentWidget::ButtonColorAndOpacity_UseTimeMode, EYapTimeMode::AudioTime, YapColor::White)
						.HAlign(HAlign_Center)
						[
							SNew(SImage)
							.DesiredSizeOverride(FVector2D(16, 16))
							.ColorAndOpacity(FSlateColor::UseForeground())
							.Image(FAppStyle::GetBrush("ProjectSettings.TabIcon"))
						]
					]
					+ SHorizontalBox::Slot()
					.FillWidth(1.0)
					.VAlign(VAlign_Center)
					[
						// -----------------------------
						// TIME DISPLAY
						// -----------------------------
						SNew(SNumericEntryBox<double>)
						.IsEnabled(false)
						.ToolTipText(LOCTEXT("FragmentTimeEntry_Tooltip", "Time this dialogue fragment will play for"))
						.Justification(ETextJustify::Center)
						.Value(this, &SFlowGraphNode_YapFragmentWidget::Value_ManualTimeEntryBox)
						.OnValueCommitted(this, &SFlowGraphNode_YapFragmentWidget::OnValueCommitted_ManualTimeEntryBox)
					]
				]
				+ SVerticalBox::Slot()
				.Padding(0, 2, 0, 0)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					.Padding(0, 0, 8, 0)
					[
						// =============================
						// USE AUDIO TIME BUTTON
						// =============================
						SNew(SButton)
						.Cursor(EMouseCursor::Default)
						.ButtonStyle(FYapEditorStyle::Get(), YapStyles.ButtonStyle_TimeSetting)
						.ContentPadding(FMargin(4, 3))
						.ToolTipText(LOCTEXT("UseTimeFromAudio_Tooltip", "Use a time read from the audio asset"))
						.OnClicked(this, &SFlowGraphNode_YapFragmentWidget::OnClicked_UseAudioTimeButton)
						.ButtonColorAndOpacity(this, &SFlowGraphNode_YapFragmentWidget::ButtonColorAndOpacity_UseTimeMode, EYapTimeMode::AudioTime, YapColor::Yellow)
						.ForegroundColor(this, &SFlowGraphNode_YapFragmentWidget::ButtonColorAndOpacity_UseTimeMode, EYapTimeMode::AudioTime, YapColor::White)
						.HAlign(HAlign_Center)
						[
							SNew(SImage)
							.DesiredSizeOverride(FVector2D(16, 16))
							.Image(FYapEditorStyle::GetImageBrush(YapBrushes.Icon_AudioTime))
							.ColorAndOpacity(FSlateColor::UseForeground())
						]
					]
					+ SHorizontalBox::Slot()
					.FillWidth(1.0)
					.VAlign(VAlign_Center)
					[
						// -----------------------------
						// TIME DISPLAY
						// -----------------------------
						SNew(SNumericEntryBox<double>)
						.IsEnabled(false)
						.ToolTipText(LOCTEXT("FragmentTimeEntry_Tooltip", "Time this dialogue fragment will play for"))
						.Justification(ETextJustify::Center)
						.Value_Lambda( [this] () { return GetBit().GetAudioTime(); } )
						.OnValueCommitted(this, &SFlowGraphNode_YapFragmentWidget::OnValueCommitted_ManualTimeEntryBox)
					]
				]
				+ SVerticalBox::Slot()
				.Padding(0, 2, 0, 2)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					.Padding(0, 0, 8, 0)
					[
						// =============================
						// USE TEXT TIME BUTTON
						// =============================
						SNew(SButton)
						.Cursor(EMouseCursor::Default)
						.ButtonStyle(FYapEditorStyle::Get(), YapStyles.ButtonStyle_TimeSetting)
						.ContentPadding(FMargin(4, 3))
						.ToolTipText(LOCTEXT("UseTimeFromText_Tooltip", "Use a time calculated from text length"))
						.OnClicked(this, &SFlowGraphNode_YapFragmentWidget::OnClicked_UseTextTimeButton)
						.ButtonColorAndOpacity(this, &SFlowGraphNode_YapFragmentWidget::ButtonColorAndOpacity_UseTimeMode, EYapTimeMode::TextTime, YapColor::Orange)
						.ForegroundColor(this, &SFlowGraphNode_YapFragmentWidget::ButtonColorAndOpacity_UseTimeMode, EYapTimeMode::TextTime, YapColor::White)
						.HAlign(HAlign_Center)
						[
							SNew(SImage)
							.DesiredSizeOverride(FVector2D(16, 16))
							.Image(FYapEditorStyle::GetImageBrush(YapBrushes.Icon_TextTime))
							.ColorAndOpacity(FSlateColor::UseForeground())
						]
					]
					+ SHorizontalBox::Slot()
					.FillWidth(1.0)
					.VAlign(VAlign_Center)
					[
						// -----------------------------
						// TIME DISPLAY
						// -----------------------------
						SNew(SNumericEntryBox<double>)
						.IsEnabled(false)
						.ToolTipText(LOCTEXT("FragmentTimeEntry_Tooltip", "Time this dialogue fragment will play for"))
						.Justification(ETextJustify::Center)
						.Value_Lambda( [this] () { return GetBit().GetTextTime(); } )
						.OnValueCommitted(this, &SFlowGraphNode_YapFragmentWidget::OnValueCommitted_ManualTimeEntryBox)
					]
				]
				+ SVerticalBox::Slot()
				.Padding(0, 2, 0, 2)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					.Padding(0, 0, 8, 0)
					[
						// =============================
						// USE MANUAL TIME ENTRY BUTTON
						// =============================
						SNew(SButton)
						.Cursor(EMouseCursor::Default)
						.ButtonStyle(FYapEditorStyle::Get(), YapStyles.ButtonStyle_TimeSetting)
						.ContentPadding(FMargin(4, 3))
						.ToolTipText(LOCTEXT("UseEnteredTime_Tooltip", "Use a manually entered time"))
						.OnClicked(this, &SFlowGraphNode_YapFragmentWidget::OnClicked_UseManuallyEnteredTimeButton)
						.ButtonColorAndOpacity(this, &SFlowGraphNode_YapFragmentWidget::ButtonColorAndOpacity_UseTimeMode, EYapTimeMode::ManualTime, YapColor::OrangeRed)
						.ForegroundColor(this, &SFlowGraphNode_YapFragmentWidget::ButtonColorAndOpacity_UseTimeMode, EYapTimeMode::ManualTime, YapColor::White)
						.HAlign(HAlign_Center)
						[
							SNew(SImage)
							.DesiredSizeOverride(FVector2D(16, 16))
							.ColorAndOpacity(FSlateColor::UseForeground())
							.Image(FYapEditorStyle::GetImageBrush(YapBrushes.Icon_Timer))
						]
					]
					+ SHorizontalBox::Slot()
					.FillWidth(1.0)
					.VAlign(VAlign_Center)
					[
						// -----------------------------
						// TIME DISPLAY
						// -----------------------------
						SNew(SNumericEntryBox<double>)
						.IsEnabled(true)
						.Delta(0.1)
						.MinValue(0.0)
						.ToolTipText(LOCTEXT("FragmentTimeEntry_Tooltip", "Time this dialogue fragment will play for"))
						.Justification(ETextJustify::Center)
						.Value_Lambda( [this] () { return GetBit().GetManualTime(); } )
						.OnValueCommitted(this, &SFlowGraphNode_YapFragmentWidget::OnValueCommitted_ManualTimeEntryBox)
					]
				]
			]
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(0)
		[
			SNew(SSeparator)
			.Orientation(Orient_Vertical)
			.Thickness(1)
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(8, 8, 8, 8)
		[
			SNew(SBox)
			.WidthOverride(80)
			.VAlign(VAlign_Fill)
			[
				SNew(STextBlock)
				.Text(INVTEXT("TEST"))
			]
		]
	];
}

// ================================================================================================
// DIALOGUE WIDGET
// ------------------------------------------------------------------------------------------------

TSharedRef<SWidget> SFlowGraphNode_YapFragmentWidget::CreateDialogueDisplayWidget()
{
	FMargin TimerSliderPadding = GetDialogueNode()->UsesTitleText() ? FMargin(0, 0, 0, -6) : FMargin(0, 0, 0, -2);
	
	TSharedRef<SWidget> Widget = SNew(SLevelOfDetailBranchNode)
	.UseLowDetailSlot(Owner, &SFlowGraphNode_YapDialogueWidget::UseLowDetail)
	.HighDetail()
	[
		SNew(SOverlay)
		+ SOverlay::Slot()
		[
			SNew(SButton)
			.Cursor(EMouseCursor::Default)
			.ButtonStyle(FYapEditorStyle::Get(), YapStyles.ButtonStyle_ActivationLimit)
			.OnClicked(this, &SFlowGraphNode_YapFragmentWidget::OnClicked_TextDisplayWidget)
			.ToolTipText(this, &SFlowGraphNode_YapFragmentWidget::ToolTipText_TextDisplayWidget, INVTEXT("Dialogue Text"), &GetBit().GetMatureDialogueText(), &GetBit().GetSafeDialogueText())
			.ContentPadding(0)
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
					.Padding(6)
					.FillSize(1.0)
					.VAlign(VAlign_Top)
					[
						SNew(STextBlock)
						.TextStyle(FYapEditorStyle::Get(), YapStyles.TextBlockStyle_DialogueText)
						.Text(this, &SFlowGraphNode_YapFragmentWidget::Text_TextDisplayWidget, &GetBit().GetMatureDialogueText(), &GetBit().GetSafeDialogueText())
						.ColorAndOpacity(this, &SFlowGraphNode_YapFragmentWidget::ColorAndOpacity_TextDisplayWidget, YapColor::White, &GetBit().GetMatureDialogueText(), &GetBit().GetSafeDialogueText())
					]
				]
				+ SOverlay::Slot()
				.VAlign(VAlign_Center)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Visibility_Lambda( [this] ()
					{
						const FText& Text = bEditingChildSafeSettings ? GetBit().GetSafeDialogueText() : GetBit().GetMatureDialogueText();
						return Text.IsEmpty() ? EVisibility::HitTestInvisible : EVisibility::Hidden;;
					})
					.Justification(ETextJustify::Center)
					.TextStyle(FYapEditorStyle::Get(), YapStyles.TextBlockStyle_DialogueText)
					.Text_Lambda( [this] () { return bEditingChildSafeSettings ? INVTEXT("Child-Safe Dialogue Text (None)") : INVTEXT("Dialogue Text (None)"); } )
					.ColorAndOpacity(YapColor::White_Glass)
				]
			]
		]
		+ SOverlay::Slot()
		.VAlign(VAlign_Bottom)
		.HAlign(HAlign_Fill)
		.Padding(TimerSliderPadding)
		[
			SNew(SHorizontalBox)
			.RenderOpacity(1.00)
			+ SHorizontalBox::Slot()
			.FillWidth(0.50)
			.VAlign(VAlign_Center)
			[
				SNew(SBox)
				.HeightOverride(2)
				.Visibility(EVisibility::HitTestInvisible)
				.ToolTipText(this, &SFlowGraphNode_YapFragmentWidget::ToolTipText_TextDisplayWidget, INVTEXT("Dialogue Text"), &GetBit().GetMatureDialogueText(), &GetBit().GetSafeDialogueText())
				[
					SNew(SProgressBar)
					.BorderPadding(0)
					.Percent(this, &SFlowGraphNode_YapFragmentWidget::FragmentTime_Percent)
					.Style(FYapEditorStyle::Get(), YapStyles.ProgressBarStyle_FragmentTimePadding)
					.FillColorAndOpacity(this, &SFlowGraphNode_YapFragmentWidget::FillColorAndOpacity_FragmentTimePadding)
					.BarFillType(EProgressBarFillType::RightToLeft)
				]
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SBox)
				.HeightOverride(8)
				.WidthOverride(8)
				.ToolTipText(this, &SFlowGraphNode_YapFragmentWidget::ToolTipText_FragmentTimePadding)
				[
					SNew(SYapTimeSettingsPopup)
					.ButtonContent()
					[
						SNew(SImage)
						.Image(FAppStyle::GetBrush("Icons.FilledCircle"))
						//.Image(FAppStyle::GetBrush("Icons.Toolbar.Settings"))
						.ColorAndOpacity(YapColor::Gray)
					]
					.MenuContent()
					[
						BuildTimeSettingsWidget()
					]
				]
			]
			+ SHorizontalBox::Slot()
			.FillWidth(0.50)
			.VAlign(VAlign_Center)
			[
				SNew(SBox)
				.HeightOverride(2)
				.Visibility(EVisibility::HitTestInvisible)
				[
					SNew(SProgressBar)
					.BorderPadding(0)
					.Percent(this, &SFlowGraphNode_YapFragmentWidget::FragmentTimePadding_Percent)
					.Style(FYapEditorStyle::Get(), YapStyles.ProgressBarStyle_FragmentTimePadding)
					.FillColorAndOpacity(this, &SFlowGraphNode_YapFragmentWidget::FillColorAndOpacity_FragmentTimePadding)
					.BarFillType(EProgressBarFillType::LeftToRight)
				]
			]
		]
	]
	.LowDetail()
	[
		SNew(SBorder)
		.BorderImage(FYapEditorStyle::GetImageBrush(YapBrushes.Box_SolidWhite_Rounded))
		.BorderBackgroundColor(YapColor::DarkGray_Glass)
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.TextStyle(FYapEditorStyle::Get(), YapStyles.TextBlockStyle_DialogueText)
			.HighlightText(INVTEXT("TODO"))
			.Text(INVTEXT("..."))
			.ColorAndOpacity(YapColor::Red)
			.HighlightColor(YapColor::Orange)
		]
	];
	
	return Widget;
}

FVector2D SFlowGraphNode_YapFragmentWidget::DialogueScrollBar_Thickness() const
{
	if (IsFragmentFocused())
	{
		return FVector2D(8, 8);
	}
	
	return FVector2D(8, 8);
}

FOptionalSize SFlowGraphNode_YapFragmentWidget::Dialogue_MaxDesiredHeight() const
{
	if (IsFragmentFocused())
	{
		int16 DeadSpace = 15;
		int16 LineHeight = 15;
		int16 FocusedLines = 9;
	
		return DeadSpace + FocusedLines * LineHeight;
	}

	return 58; // TODO fluctuate with portrait widget height
}

FText SFlowGraphNode_YapFragmentWidget::Text_TextDisplayWidget(const FText* MatureText, const FText* SafeText) const
{
	if (GEditor->PlayWorld)
	{
		return GetFragment().GetHasChildSafeData() ? *MatureText : *SafeText;
	}

	if (bEditingChildSafeSettings)
	{
		return *SafeText;
	}
	
	return *MatureText;
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

	FString Filter = GetDialogueNode()->GetDialogueTag().ToString();

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

EVisibility SFlowGraphNode_YapFragmentWidget::Visibility_ConditionWidgets() const
{
	return EVisibility::Visible;
	//return (GetFragment().GetConditions().Num() > 0) ? EVisibility::Visible : EVisibility::Hidden;
}

void SFlowGraphNode_YapFragmentWidget::OnConditionsUpdated()
{
	//Owner->EditedConditionDetailsWidget = nullptr;
}

// ================================================================================================
// FRAGMENT TIME PADDING WIDGET
// ------------------------------------------------------------------------------------------------

TOptional<float> SFlowGraphNode_YapFragmentWidget::FragmentTimePadding_Percent() const
{
	const float MaxPaddedSetting = UYapEditorSettings::Get()->GetPaddingTimeSliderMax();
	const float FragmentPadding = GetFragment().GetPaddingToNextFragment();

	if (GEditor->PlayWorld)
	{
		const FYapFragment* RunningFragment = GetDialogueNode()->GetRunningFragment();
		
		if (RunningFragment == &GetFragment())
		{
			if (GetDialogueNode()->FragmentStartedTime < GetDialogueNode()->FragmentEndedTime)
			{
				double ElapsedPaddingTime = GEditor->PlayWorld->GetTimeSeconds() - GetDialogueNode()->FragmentEndedTime;
				return (1 - (ElapsedPaddingTime / FragmentPadding)) * (FragmentPadding / MaxPaddedSetting);
			}
			else
			{
				return FragmentPadding / MaxPaddedSetting;
			}
		}
		else if (RunningFragment == nullptr || !GetDialogueNode()->GetFinishedFragments().Contains(&GetFragment()))
		{
			return FragmentPadding / MaxPaddedSetting;
		}

		return 0.0;
	}

	return FragmentPadding / MaxPaddedSetting;		
}

TOptional<float> SFlowGraphNode_YapFragmentWidget::FragmentTime_Percent() const
{
	const float MaxTimeSetting = UYapEditorSettings::Get()->GetDialogueTimeSliderMax();

	const float FragmentTime = GetFragment().GetBit().GetTime();

	if (GEditor->PlayWorld)
	{
		const FYapFragment* RunningFragment = GetDialogueNode()->GetRunningFragment();
		
		if (RunningFragment == &GetFragment())
		{
			if (GetDialogueNode()->FragmentStartedTime < GetDialogueNode()->FragmentEndedTime)
			{
				double ElapsedPaddingTime = GEditor->PlayWorld->GetTimeSeconds() - GetDialogueNode()->FragmentEndedTime;
				return (1 - (ElapsedPaddingTime / FragmentTime)) * (FragmentTime / MaxTimeSetting);
			}
			else
			{
				return FragmentTime / MaxTimeSetting;
			}
		}
		else if (RunningFragment == nullptr || !GetDialogueNode()->GetFinishedFragments().Contains(&GetFragment()))
		{
			return FragmentTime / MaxTimeSetting;
		}

		return 0.0;
	}

	return FragmentTime / MaxTimeSetting;		
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

	// We will attempt to snap to the default time unless you hold ctrl
	if (!bCtrlPressed && UYapProjectSettings::Get()->IsDefaultFragmentPaddingTimeEnabled())
	{
		float DefaultFragmentPaddingTime = UYapProjectSettings::Get()->GetDefaultFragmentPaddingTime();
		
		// If we're within 5% of the default, set it to default
		if (FMath::Abs(NewValue - DefaultFragmentPaddingTime) / MaxPaddedSetting <= 0.05)
		{
			GetFragment().SetPaddingToNextFragment(-1);
			return;
		}
	}
	
	GetFragment().SetPaddingToNextFragment(NewValue);
}

FSlateColor SFlowGraphNode_YapFragmentWidget::FillColorAndOpacity_FragmentTimePadding() const
{
	const float MaxPaddedSetting =  UYapProjectSettings::Get()->GetFragmentPaddingSliderMax();
	
	if (GEditor->PlayWorld)
	{
		const FYapFragment* RunningFragment = GetDialogueNode()->GetRunningFragment();

		if (RunningFragment == &GetFragment())
		{
			return YapColor::White_Trans;
		}

		return YapColor::DarkGray_Trans;
	}
	
	if (GetFragment().GetPaddingToNextFragment() > MaxPaddedSetting)
	{
		return YapColor::Blue_Trans;
	}
	
	return GetFragment().PaddingToNextFragment < 0 ? YapColor::DimGray_Trans : YapColor::LightBlue_Trans;
}

FText SFlowGraphNode_YapFragmentWidget::ToolTipText_FragmentTimePadding() const
{
	FNumberFormattingOptions Formatting;
	Formatting.MaximumFractionalDigits = 3;
	
	return FText::Format(LOCTEXT("Fragment", "Post-delay: {0}"), FText::AsNumber(GetFragment().GetPaddingToNextFragment(), &Formatting));
}

FSlateColor SFlowGraphNode_YapFragmentWidget::BorderBackgroundColor_CharacterImage() const
{
	const FYapBit& Bit = GetFragment().GetBit();

	FLinearColor Color;
	
	if (Bit.GetSpeakerAsset().IsValid())
	{
		Color = Bit.GetSpeakerAsset().Get()->GetEntityColor();
	}
	else
	{
		Color = YapColor::Gray_Glass;		
	}

	Color.A *= UYapEditorSettings::Get()->GetPortraitBorderAlpha();

	if (!GetDialogueNode()->IsPlayerPrompt())
	{
		Color.A *= 0.5f;
	}

	return Color;
}

void SFlowGraphNode_YapFragmentWidget::OnSetNewSpeakerAsset(const FAssetData& AssetData)
{
	FYapTransactions::BeginModify(INVTEXT("Change character asset"), GetDialogueNode());

	GetBit().SetSpeaker(AssetData.GetAsset());

	FYapTransactions::EndModify();
}

void SFlowGraphNode_YapFragmentWidget::OnSetNewDirectedAtAsset(const FAssetData& AssetData)
{
	FYapTransactions::BeginModify(INVTEXT("Change character asset"), GetDialogueNode());

	GetBit().SetDirectedAt(AssetData.GetAsset());

	FYapTransactions::EndModify();
}

FReply SFlowGraphNode_YapFragmentWidget::OnClicked_SpeakerWidget(TSoftObjectPtr<UYapCharacter>* CharacterAsset, const UYapCharacter* Character)
{
	TSharedPtr<SWidget> Widget = SNew(SBorder)
		.Padding(1, 1, 1, 1) // No idea why but the details view already has a 4 pixel transparent area on top
		.BorderImage(FYapEditorStyle::GetImageBrush(YapBrushes.Box_SolidLightGray_Rounded))
		.BorderBackgroundColor(YapColor::DimGray)
		[
			SNew(SYapPropertyMenuAssetPicker)
			.AllowedClasses({UYapCharacter::StaticClass()})
			.AllowClear(true)
			.InitialObject(Character)
			.OnSet(this, &SFlowGraphNode_YapFragmentWidget::OnSetNewSpeakerAsset)
		];

	Owner->AddOverlayWidget(SpeakerWidget, Widget);
	
	return FReply::Handled();
}

FText SFlowGraphNode_YapFragmentWidget::Text_SpeakerWidget() const
{
	const FYapBit& Bit = GetFragment().GetBit();
	
	if (Bit.GetSpeakerAsset().IsNull())
	{
		return INVTEXT("Character\nUnset");
	}
	
	if (Image_SpeakerImage() == nullptr)
	{
		TSharedPtr<FGameplayTagNode> GTN = UGameplayTagsManager::Get().FindTagNode(Bit.GetMoodKey());
		
		FText CharacterName = Bit.GetSpeakerAsset().IsValid() ? Bit.GetSpeakerAsset().Get()->GetEntityName() : INVTEXT("Unloaded");
		
		if (CharacterName.IsEmpty())
		{
			CharacterName = INVTEXT("Unnamed");
		}

		FText MoodTagName;
		
		if (GTN.IsValid())
		{
			MoodTagName = FText::FromName(GTN->GetSimpleTagName());
		}
		else
		{
			TSharedPtr<FGameplayTagNode> DefaultGTN = UGameplayTagsManager::Get().FindTagNode(UYapProjectSettings::Get()->GetDefaultMoodTag());

			MoodTagName = DefaultGTN.IsValid() ? FText::Format(INVTEXT("{0}{1}"), FText::FromName(DefaultGTN->GetSimpleTagName()), INVTEXT("(D)")) : INVTEXT("None(D)");
		}
		
		return FText::Format(INVTEXT("{0}\n\n{1}\n<missing>"), CharacterName, MoodTagName);
	}
	
	return INVTEXT("");
}

bool SFlowGraphNode_YapFragmentWidget::OnAreAssetsAcceptableForDrop_SpeakerWidget(TArrayView<FAssetData> AssetDatas) const
{
	if (AssetDatas.Num() != 1)
	{
		return false;
	}

	UClass* Class = AssetDatas[0].GetClass();
	
	if (Class == UYapCharacter::StaticClass())
	{
		return true;
	}

	if (Class == UYapProjectSettings::Get()->GetDialogueAssetClass())
	{
		return true;
	}

	return false;
}

void SFlowGraphNode_YapFragmentWidget::OnAssetsDropped_SpeakerWidget(const FDragDropEvent& DragDropEvent, TArrayView<FAssetData> AssetDatas)
{
	if (AssetDatas.Num() != 1)
	{
		return;
	}

	UObject* Object = AssetDatas[0].GetAsset();
	
	if (UYapCharacter* Character = Cast<UYapCharacter>(Object))
	{
		FYapTransactions::BeginModify(INVTEXT("Setting character"), GetDialogueNode());

		GetBit().SetSpeaker(Character);

		FYapTransactions::EndModify();
	}

	else if (Object->GetClass() == UYapProjectSettings::Get()->GetDialogueAssetClass())
	{
		FYapTransactions::BeginModify(INVTEXT("Setting dialogue asset"), GetDialogueNode());

		GetBit().SetDialogueAudioAsset(Object);

		FYapTransactions::EndModify();
	}
}

// ================================================================================================
// PORTRAIT WIDGET
// ------------------------------------------------------------------------------------------------

TSharedRef<SOverlay> SFlowGraphNode_YapFragmentWidget::CreateSpeakerWidget()
{
	int32 PortraitSize = UYapEditorSettings::Get()->GetPortraitSize();
	
	return SNew(SOverlay)
	+ SOverlay::Slot()
	.Padding(0)
	[
		SNew(SLevelOfDetailBranchNode)
		.UseLowDetailSlot(Owner, &SFlowGraphNode_YapDialogueWidget::UseLowDetail)
		.HighDetail()
		[
			SNew(SAssetDropTarget)
			.bSupportsMultiDrop(false)
			.OnAreAssetsAcceptableForDrop(this, &SFlowGraphNode_YapFragmentWidget::OnAreAssetsAcceptableForDrop_SpeakerWidget)
			.OnAssetsDropped(this, &SFlowGraphNode_YapFragmentWidget::OnAssetsDropped_SpeakerWidget)
			[
				SNew(SButton)
				.Cursor(EMouseCursor::Default)
				.ButtonStyle(FAppStyle::Get(), "SimpleButton")
				.ToolTip(nullptr)
				.ContentPadding(0)
				.OnClicked(this, &SFlowGraphNode_YapFragmentWidget::OnClicked_SpeakerWidget, &GetBit().SpeakerAsset, GetBit().GetSpeaker())
				[
					SNew(SOverlay)
					+ SOverlay::Slot()
					[
						SNew(SBox)
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Center)
						[
							SNew(SImage)
							.DesiredSizeOverride(FVector2D(PortraitSize, PortraitSize))
							.Image(this, &SFlowGraphNode_YapFragmentWidget::Image_SpeakerImage)	
						]
					]
					+ SOverlay::Slot()
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(this, &SFlowGraphNode_YapFragmentWidget::Text_SpeakerWidget)
						.Font(FCoreStyle::GetDefaultFontStyle("Normal", 8))
						.ColorAndOpacity(YapColor::Red)
						.Justification(ETextJustify::Center)
					]
				]
			]
		]
		.LowDetail()
		[
			SNew(SImage)
			.DesiredSizeOverride(FVector2D(PortraitSize, PortraitSize))
			.Image(this, &SFlowGraphNode_YapFragmentWidget::Image_SpeakerImage)
		]
	]
	+ SOverlay::Slot()
	.Padding(0, 0, 0, 0)
	[
		SNew(SBox)
		.WidthOverride(PortraitSize + 8)
		.HeightOverride(PortraitSize + 8)
		.Visibility(EVisibility::HitTestInvisible)
		[
			SNew(SBorder)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			.BorderImage(FYapEditorStyle::GetImageBrush(YapBrushes.Border_Thick_RoundedSquare))
			.BorderBackgroundColor(this, &SFlowGraphNode_YapFragmentWidget::BorderBackgroundColor_CharacterImage)
		]
	];
}

EVisibility SFlowGraphNode_YapFragmentWidget::Visibility_PortraitImage() const
{	
	if (IsFragmentFocused())
	{
		return EVisibility::Collapsed;
	}
	
	if (!Owner->GetIsSelected())
	{
		return EVisibility::Visible;
	}

	return EVisibility::Visible;
}

const FSlateBrush* SFlowGraphNode_YapFragmentWidget::Image_SpeakerImage() const
{
	const FSlateBrush& PortraitBrush = GetFragment().GetBit().GetSpeakerPortraitBrush();

	if (PortraitBrush.GetResourceObject())
	{
		return &PortraitBrush;
	}
	else
	{
		return nullptr;
	}
}

EVisibility SFlowGraphNode_YapFragmentWidget::Visibility_MissingPortraitWarning() const
{
	const FSlateBrush& Brush = GetFragment().GetBit().GetSpeakerPortraitBrush();
	
	return (Brush.GetResourceObject()) ? EVisibility::Hidden : EVisibility::Visible;
}

EVisibility SFlowGraphNode_YapFragmentWidget::Visibility_CharacterSelect() const
{
	return IsHovered() ? EVisibility::Visible : EVisibility::Collapsed;
}

FString SFlowGraphNode_YapFragmentWidget::ObjectPath_CharacterSelect() const
{	
	const TSoftObjectPtr<UYapCharacter> Asset = GetBit().GetSpeakerAsset();

	if (Asset.IsPending())
	{
		(void)Asset.LoadSynchronous();
	}
	
	if (!Asset) { return ""; }

	return Asset.ToString();
}

void SFlowGraphNode_YapFragmentWidget::OnObjectChanged_CharacterSelect(const FAssetData& InAssetData)
{
	FYapTransactions::BeginModify(LOCTEXT("NodeCharacterChanged", "Character Changed"), GetDialogueNode());

	UObject* Asset = InAssetData.GetAsset();

	UYapCharacter* Character = Cast<UYapCharacter>(Asset);
	
	GetBit().SetSpeaker(Character);

	FYapTransactions::EndModify();
}

FText SFlowGraphNode_YapFragmentWidget::ToolTipText_MoodKeySelector() const
{
	TSharedPtr<FGameplayTagNode> TagNode = UGameplayTagsManager::Get().FindTagNode(GetFragment().GetBit().GetMoodKey());

	if (TagNode.IsValid())
	{
		return FText::FromName(TagNode->GetSimpleTagName());
	}

	return INVTEXT("Default");
}

FSlateColor SFlowGraphNode_YapFragmentWidget::ForegroundColor_MoodKeySelectorWidget() const
{
	if (GetBit().GetMoodKey() == FGameplayTag::EmptyTag)
	{
		return YapColor::DarkGray;
	}
	
	return YapColor::White_Trans;
}

// ================================================================================================
// TITLE TEXT WIDGET
// ------------------------------------------------------------------------------------------------

TSharedRef<SWidget> SFlowGraphNode_YapFragmentWidget::CreateTitleTextDisplayWidget()
{
	TitleTextEditButtonWidget = SNew(SButton)
	.Cursor(EMouseCursor::Default)
	.Visibility(this, &SFlowGraphNode_YapFragmentWidget::Visibility_TitleText)
	.ToolTipText(INVTEXT("Title text"))
	.ButtonStyle(FYapEditorStyle::Get(),YapStyles.ButtonStyle_ActivationLimit)
	.OnClicked(this, &SFlowGraphNode_YapFragmentWidget::OnClicked_TextDisplayWidget)
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
			.Padding(2, 0, 2, 0)
			.FillSize(1.0)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.TextStyle(FYapEditorStyle::Get(), YapStyles.TextBlockStyle_TitleText)
				.Text(this, &SFlowGraphNode_YapFragmentWidget::Text_TextDisplayWidget, &GetBit().GetMatureTitleText(), &GetBit().GetSafeTitleText())
				.ToolTipText(this, &SFlowGraphNode_YapFragmentWidget::ToolTipText_TextDisplayWidget, INVTEXT("Title Text"), &GetBit().GetMatureTitleText(), &GetBit().GetSafeTitleText())
				.ColorAndOpacity(this, &SFlowGraphNode_YapFragmentWidget::ColorAndOpacity_TextDisplayWidget, YapColor::YellowGray, &GetBit().GetMatureTitleText(), &GetBit().GetSafeTitleText())
			]
		]
		+ SOverlay::Slot()
		.VAlign(VAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Visibility_Lambda( [this] ()
			{
				const FText& Text = bEditingChildSafeSettings ? GetBit().GetSafeTitleText() : GetBit().GetMatureTitleText();
				return Text.IsEmpty() ? EVisibility::HitTestInvisible : EVisibility::Hidden;;
			})
			.Justification(ETextJustify::Center)
			.TextStyle(FYapEditorStyle::Get(), YapStyles.TextBlockStyle_TitleText)
			.Text_Lambda( [this] () { return bEditingChildSafeSettings ? INVTEXT("Child-Safe Title Text (None)") : INVTEXT("Title Text (None)"); } )
			.ColorAndOpacity(YapColor::White_Glass)
		]
	];

	return TitleTextEditButtonWidget.ToSharedRef();
}

EVisibility SFlowGraphNode_YapFragmentWidget::Visibility_TitleTextEdit() const
{
	return TitleTextEditButtonWidget->IsHovered() ? EVisibility::HitTestInvisible : EVisibility::Collapsed;
}

FText SFlowGraphNode_YapFragmentWidget::Text_EditedText(FText* Text) const
{
	return *Text;
}

void SFlowGraphNode_YapFragmentWidget::OnTextCommitted_EditedText(const FText& NewValue, ETextCommit::Type CommitType, void (FYapBit::*Func)(FText* TextToSet, const FText& NewValue), FText* TextToSet)
{
	FYapTransactions::BeginModify(LOCTEXT("Yap Dialogue Node", "Text Changed"), GetDialogueNode());

	if (CommitType != ETextCommit::OnCleared)
	{
		(GetBit().*Func)(TextToSet, NewValue);
	}

	FYapTransactions::EndModify();
}

// TODO templates to reduce code fat
FReply SFlowGraphNode_YapFragmentWidget::OnClicked_TextDisplayWidget()
{
	FReply ButtonReply = FReply::Handled();
	
	if (bTextEditorExpanded)
	{
		// This should never happen
		return ButtonReply;
	}
	
	TSharedRef<IEditableTextProperty> MatureDialogueTextProperty = MakeShareable(new FYapEditableTextPropertyHandle(GetBit().MatureDialogueText));
	TSharedRef<IEditableTextProperty> MatureTitleTextProperty = MakeShareable(new FYapEditableTextPropertyHandle(GetBit().MatureTitleText));

	TSharedRef<IEditableTextProperty> SafeDialogueTextProperty = MakeShareable(new FYapEditableTextPropertyHandle(GetBit().SafeDialogueText));
	TSharedRef<IEditableTextProperty> SafeTitleTextProperty = MakeShareable(new FYapEditableTextPropertyHandle(GetBit().SafeTitleText));

	float TotalPortraitWidgetSize = UYapEditorSettings::Get()->GetPortraitSize() + 8;

	ExpandedTextEditorWidget_StartOffset = TotalPortraitWidgetSize + 5;
	ExpandedTextEditorWidget_Offset = ExpandedTextEditorWidget_StartOffset;
	ExpandedTextEditorWidget_OffsetAlpha = 0.0;

	
	ExpandedTextEditorWidget = SNew(SBox)
	.Padding(ExpandedTextEditorWidget_Offset, 0, 0, 0)
	[
		SNew(SOverlay)
		+ SOverlay::Slot()
		.Padding(4, 0, 0, 0)
		[
			SNew(SVerticalBox)
			.Visibility_Lambda( [this] () { return bEditingChildSafeSettings ? EVisibility::Hidden : EVisibility::Visible; } )
			+ SVerticalBox::Slot()
			.Padding(0, 0, 0, 0)
			.VAlign(VAlign_Fill)
			.HAlign(HAlign_Fill)
			[
				SNew(SBox)
				[
					SNew(SYapTextPropertyEditableTextBox, MatureDialogueTextProperty)
					.Style(FYapEditorStyle::Get(), YapStyles.EditableTextBoxStyle_Dialogue)
					.Text(this, &SFlowGraphNode_YapFragmentWidget::Text_EditedText, &GetBit().MatureDialogueText)
					.OnTextCommitted(this, &SFlowGraphNode_YapFragmentWidget::OnTextCommitted_EditedText, &FYapBit::SetDialogueText, &GetBit().MatureDialogueText)
					.ForegroundColor(YapColor::White)
					.Cursor(EMouseCursor::Default)
				]
			]
			+ SVerticalBox::Slot()
			.Padding(0, 4, 0, 0)
			.AutoHeight()
			[
				SNew(SBox)
				.Visibility(GetDialogueNode()->UsesTitleText() ? EVisibility::Visible : EVisibility::Collapsed)
				[
					SNew(SYapTextPropertyEditableTextBox, MatureTitleTextProperty)
					.Style(FYapEditorStyle::Get(), YapStyles.EditableTextBoxStyle_TitleText)
					.Text(this, &SFlowGraphNode_YapFragmentWidget::Text_EditedText, &GetBit().MatureTitleText)
					.OnTextCommitted(this, &SFlowGraphNode_YapFragmentWidget::OnTextCommitted_EditedText, &FYapBit::SetDialogueText, &GetBit().MatureTitleText)
					.ForegroundColor(YapColor::YellowGray)
					.Cursor(EMouseCursor::Default)
				]
			]
		]
		+ SOverlay::Slot()
		.Padding(4, 0, 0, 0)
		[
			SNew(SVerticalBox)
			.Visibility_Lambda( [this] () { return bEditingChildSafeSettings ? EVisibility::Visible : EVisibility::Hidden; } )
			+ SVerticalBox::Slot()
			.Padding(0, 0, 0, 0)
			.VAlign(VAlign_Fill)
			.HAlign(HAlign_Fill)
			[
				SNew(SBox)
				[
					SNew(SYapTextPropertyEditableTextBox, SafeDialogueTextProperty)
					.Style(FYapEditorStyle::Get(), YapStyles.EditableTextBoxStyle_Dialogue)
					.Text(this, &SFlowGraphNode_YapFragmentWidget::Text_EditedText, &GetBit().SafeDialogueText)
					.OnTextCommitted(this, &SFlowGraphNode_YapFragmentWidget::OnTextCommitted_EditedText, &FYapBit::SetDialogueText, &GetBit().SafeDialogueText)
					.ForegroundColor(YapColor::White * YapColor::LightBlue)
					.Cursor(EMouseCursor::Default)
				]
			]
			+ SVerticalBox::Slot()
			.Padding(0, 5, 0, 0)
			.AutoHeight()
			[
				SNew(SBox)
				.Visibility(GetDialogueNode()->UsesTitleText() ? EVisibility::Visible : EVisibility::Collapsed)
				[
					SNew(SYapTextPropertyEditableTextBox, SafeTitleTextProperty)
					.Style(FYapEditorStyle::Get(), YapStyles.EditableTextBoxStyle_TitleText)
					.Text(this, &SFlowGraphNode_YapFragmentWidget::Text_EditedText, &GetBit().SafeTitleText)
					.OnTextCommitted(this, &SFlowGraphNode_YapFragmentWidget::OnTextCommitted_EditedText, &FYapBit::SetDialogueText, &GetBit().SafeTitleText)
					.ForegroundColor(YapColor::YellowGray * YapColor::LightBlue)
					.Cursor(EMouseCursor::Default)
				]
			]
		]
		+ SOverlay::Slot()
		.VAlign(VAlign_Fill)
		.HAlign(HAlign_Left)
		.Padding(-2, 0, 0, 0)
		[
			SNew(SBox)
			.WidthOverride(5)
			[
				SNew(SButton)
				.Cursor(EMouseCursor::Default)
				.ButtonStyle(FCoreStyle::Get(), "HoverHintOnly")
				.ContentPadding(0)
				[
					SNew(SSpacer)
				]
			]
		]
	];
	
	FragmentOverlay->AddSlot()
	.Padding(-1, 0, -1, 0)
	[
		ExpandedTextEditorWidget.ToSharedRef()
	];

	bTextEditorExpanded = true;

	Owner->SetNodeSelected();
	
	return FReply::Handled();

}

FText SFlowGraphNode_YapFragmentWidget::ToolTipText_TextDisplayWidget(FText Label, const FText* MatureText, const FText* SafeText) const
{
	//if (bCtrlPressed)
	//{
		FText Unset = INVTEXT("\u26A0 No text \u26A0");//" !! \u203C \u270D \u2756 \u26A0");
		
		if (GetFragment().GetHasChildSafeData())
		{
			return FText::Format(INVTEXT("{0}\n\n\u2668\u2502{1}\n\n\u26F9\u2502{2}"), Label, MatureText->IsEmpty() ? Unset : *MatureText, SafeText->IsEmpty() ? Unset : *SafeText);
		}
		else
		{
			return FText::Format(INVTEXT("{0}\n\n\u2756\u2502{1}"), Label, MatureText->IsEmpty() ? Unset : *MatureText);
		}
	//}
	//else
	//{
	//	return INVTEXT("Hold CTRL to show");
	//}
}

FSlateColor SFlowGraphNode_YapFragmentWidget::ColorAndOpacity_TextDisplayWidget(FLinearColor BaseColor, const FText* MatureText, const FText* SafeText) const
{
	FLinearColor Color = BaseColor;
	
	if (GEditor->PlayWorld)
	{
		if (GetFragment().GetHasChildSafeData() && !SafeText->IsEmpty())
		{
			Color *= YapColor::LightBlue;
		}
	}

	if (bEditingChildSafeSettings)
	{
		Color *= YapColor::LightBlue;
	}
	
	return Color;
}

EVisibility SFlowGraphNode_YapFragmentWidget::Visibility_TitleText() const
{
	return GetDialogueNode()->UsesTitleText() ? EVisibility::Visible : EVisibility::Collapsed;
}

// ================================================================================================
// FRAGMENT TAG WIDGET
// ------------------------------------------------------------------------------------------------

TSharedRef<SWidget> SFlowGraphNode_YapFragmentWidget::CreateFragmentTagWidget()
{
	auto TagAttribute = TAttribute<FGameplayTag>::CreateSP(this, &SFlowGraphNode_YapFragmentWidget::Value_FragmentTag);
	FString FilterString = GetDialogueNode()->GetDialogueTag().ToString();
	auto OnTagChanged = TDelegate<void(const FGameplayTag)>::CreateSP(this, &SFlowGraphNode_YapFragmentWidget::OnTagChanged_FragmentTag);

	return SNew(SGameplayTagComboFiltered)
	.Tag(TagAttribute)
	.Filter(FilterString)
	.OnTagChanged(OnTagChanged)
	.ToolTipText(INVTEXT("Fragment tag"));
}

FGameplayTag SFlowGraphNode_YapFragmentWidget::Value_FragmentTag() const
{
	return GetFragment().FragmentTag;
}

void SFlowGraphNode_YapFragmentWidget::OnTagChanged_FragmentTag(FGameplayTag GameplayTag)
{
	FYapTransactions::BeginModify(LOCTEXT("Fragment", "Change Fragment Tag"), GetDialogueNode());

	GetFragment().FragmentTag = GameplayTag;

	FYapTransactions::EndModify();

	Owner->RequestUpdateGraphNode();
}

FReply SFlowGraphNode_YapFragmentWidget::OnClicked_UseProjectDefaultTimeSettingsButton()
{
	FYapTransactions::BeginModify(LOCTEXT("NodeUseProjectDefaultTimeSettings", "Use Project Default Time Settings Changed"), GetDialogueNode());
	GetBit().SetUseProjectDefaultSettings();
	FYapTransactions::EndModify();

	return FReply::Handled();
}

FReply SFlowGraphNode_YapFragmentWidget::OnClicked_UseAudioTimeButton()
{
	FYapTransactions::BeginModify(LOCTEXT("Fragment", "Fragment Time Mode Changed"), GetDialogueNode());
	GetBit().SetBitTimeMode(EYapTimeMode::AudioTime);
	FYapTransactions::EndModify();

	return FReply::Handled();
}

FReply SFlowGraphNode_YapFragmentWidget::OnClicked_UseTextTimeButton()
{
	FYapTransactions::BeginModify(LOCTEXT("Fragment", "Fragment Time Mode Changed"), GetDialogueNode());
	GetBit().SetBitTimeMode(EYapTimeMode::TextTime);
	FYapTransactions::EndModify();

	return FReply::Handled();
}

FReply SFlowGraphNode_YapFragmentWidget::OnClicked_UseManuallyEnteredTimeButton()
{
	FYapTransactions::BeginModify(LOCTEXT("Fragment", "Fragment Time Mode Changed"), GetDialogueNode());
	GetBit().SetBitTimeMode(EYapTimeMode::ManualTime);
	FYapTransactions::EndModify();

	return FReply::Handled();
}

// ---------------------

TOptional<double> SFlowGraphNode_YapFragmentWidget::Value_ManualTimeEntryBox() const
{
	double Time = GetBit().GetTime();
	
	return (Time > 0) ? Time : TOptional<double>();
}

void SFlowGraphNode_YapFragmentWidget::OnValueCommitted_ManualTimeEntryBox(double NewValue, ETextCommit::Type CommitType)
{
	FYapTransactions::BeginModify(LOCTEXT("NodeEnteredTimeChanged", "Entered Time Changed"), GetDialogueNode());

	if (CommitType == ETextCommit::OnEnter || CommitType == ETextCommit::OnUserMovedFocus)
	{
		GetBit().SetManualTime(NewValue);
	}
	else if (CommitType == ETextCommit::OnCleared)
	{
		GetBit().SetManualTime(0.0);
	}

	FYapTransactions::EndModify();
}

FSlateColor SFlowGraphNode_YapFragmentWidget::ButtonColorAndOpacity_UseProjectDefaultTimeSettingsButton() const
{
	return GetBit().GetUseProjectDefaultTimeSettings() ? YapColor::BrightBlue : YapColor::DarkGray;
}

FSlateColor SFlowGraphNode_YapFragmentWidget::ButtonColorAndOpacity_UseTimeMode(EYapTimeMode TimeMode, FLinearColor ColorTint) const
{
	if (GetBit().GetTimeMode() == TimeMode)
	{
		bool bFromDefaults = GetBit().GetUseProjectDefaultTimeSettings();

		return bFromDefaults ? ColorTint.Desaturate(0.50) : ColorTint;
	}
	
	return YapColor::DimGray;
}

// ================================================================================================
// AUDIO ASSET WIDGET
// ------------------------------------------------------------------------------------------------

TSharedRef<SWidget> SFlowGraphNode_YapFragmentWidget::CreateAudioAssetWidget(TAttribute<EVisibility> VisibilityAtt, TAttribute<FString> ObjectPathAtt, TDelegate<void(const FAssetData&)> OnObjectChangedAtt)
{
	UClass* DialogueAssetClass = UObject::StaticClass(); // if I use nullptr then SObjectPropertyEntryBox throws a shitfit
	
	TSoftClassPtr<UObject> DialogueAssetClassPtr = UYapProjectSettings::Get()->GetDialogueAssetClass();
	
	if (!DialogueAssetClassPtr.IsNull())
	{
		DialogueAssetClass = DialogueAssetClassPtr.LoadSynchronous();
	}
	
	TSharedRef<SObjectPropertyEntryBox> AudioAssetProperty = SNew(SObjectPropertyEntryBox)
		.IsEnabled(!DialogueAssetClassPtr.IsNull())
		.DisplayBrowse(true)
		.DisplayUseSelected(false)
		.DisplayThumbnail(false)
		.AllowedClass(DialogueAssetClass)
		.EnableContentPicker(true)
		.ObjectPath(ObjectPathAtt)
		.OnObjectChanged(OnObjectChangedAtt)
		.ToolTipText(LOCTEXT("DialogueAudioAsset_Tooltip", "Select an audio asset."));
	
	TSharedRef<SWidget> Widget = SNew(SOverlay)
	.Visibility(VisibilityAtt)
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
	const UObject* Asset = GetBit().GetDialogueAudioAsset<UObject>();

	if (!Asset) { return ""; }

	return Asset->GetPathName();
}

void SFlowGraphNode_YapFragmentWidget::OnObjectChanged_AudioAsset(const FAssetData& InAssetData)
{
	FYapTransactions::BeginModify(LOCTEXT("NodeAudioAssetChanged", "Audio Asset Changed"), GetDialogueNode());

	GetBit().SetDialogueAudioAsset(InAssetData.GetAsset());

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
	UClass* AssetClass = UYapProjectSettings::Get()->GetDialogueAssetClass().LoadSynchronous();

	static EYapErrorLevel CachedErrorLevel = EYapErrorLevel::OK;
	static double LastUpdateTime = 0;
	
	if (GWorld)
	{
		if (GWorld->GetRealTimeSeconds() - LastUpdateTime < 1.0)
		{
			return CachedErrorLevel;
		}		
	}
	
	const TSoftObjectPtr<UObject> Asset = GetBit().GetDialogueAudioAsset_SoftPtr<UObject>();

	if (Asset)
	{
		if (!Asset->IsA(AssetClass))
		{
			CachedErrorLevel = EYapErrorLevel::Error;
			return CachedErrorLevel;
		}
	}

	if (GetBit().GetTimeMode() == EYapTimeMode::AudioTime && !GetBit().HasDialogueAudioAsset())
	{
		CachedErrorLevel = UYapProjectSettings::Get()->GetMissingAudioErrorLevel();
		return CachedErrorLevel;
	}

	CachedErrorLevel = EYapErrorLevel::OK;
	return CachedErrorLevel;
}

EVisibility SFlowGraphNode_YapFragmentWidget::Visibility_AudioButton() const
{
	if (bShowAudioSettings)
	{
		return EVisibility::Collapsed;
	}

	return GetFragment().GetBit().HasDialogueAudioAsset() ? EVisibility::Visible : EVisibility::Collapsed;
}

// ================================================================================================
// HELPER API
// ================================================================================================

const UFlowNode_YapDialogue* SFlowGraphNode_YapFragmentWidget::GetDialogueNode() const
{
	return Owner->GetFlowYapDialogueNodeMutable();
}

UFlowNode_YapDialogue* SFlowGraphNode_YapFragmentWidget::GetDialogueNode()
{
	return const_cast<UFlowNode_YapDialogue*>(const_cast<const SFlowGraphNode_YapFragmentWidget*>(this)->GetDialogueNode());
}

const FYapFragment& SFlowGraphNode_YapFragmentWidget::GetFragment() const
{
	return GetDialogueNode()->GetFragmentByIndex(FragmentIndex);
}

FYapFragment& SFlowGraphNode_YapFragmentWidget::GetFragment()
{
	return const_cast<FYapFragment&>(const_cast<const SFlowGraphNode_YapFragmentWidget*>(this)->GetFragment());
}

const FYapBit& SFlowGraphNode_YapFragmentWidget::GetBit() const
{
	return GetFragment().GetBit();
}

FYapBit& SFlowGraphNode_YapFragmentWidget::GetBit()
{
	return const_cast<FYapBit&>(const_cast<const SFlowGraphNode_YapFragmentWidget*>(this)->GetBit());
}

bool SFlowGraphNode_YapFragmentWidget::IsFragmentFocused() const
{
	uint8 FocusedFragmentIndex;
	return (Owner->GetFocusedFragmentIndex(FocusedFragmentIndex) && FocusedFragmentIndex == FragmentIndex);
}

EVisibility SFlowGraphNode_YapFragmentWidget::Visibility_RowHighlight() const
{
	if (GetDialogueNode()->GetRunningFragment() == &GetFragment())
	{
		return EVisibility::HitTestInvisible;
	}

	if (GetFragment().IsActivationLimitMet())
	{
		return EVisibility::HitTestInvisible;
	}
	
	if (GetDialogueNode()->GetActivationState() != EFlowNodeState::Active && GetDialogueNode()->ActivationLimitsMet())
	{
		return EVisibility::HitTestInvisible;
	}

	return EVisibility::Collapsed;
}

FSlateColor SFlowGraphNode_YapFragmentWidget::BorderBackgroundColor_RowHighlight() const
{
	if (GetDialogueNode()->GetRunningFragment() == &GetFragment())
	{
		return YapColor::White_Glass;
	}
	
	if (GetFragment().IsActivationLimitMet())
	{
		return YapColor::Red_Glass;
	}
	
	if (GetDialogueNode()->GetActivationState() != EFlowNodeState::Active && GetDialogueNode()->ActivationLimitsMet())
	{
		return YapColor::Red_Glass;
	}

	return YapColor::White_Glass;
}

// ================================================================================================
// OVERRIDES
// ================================================================================================

FSlateColor SFlowGraphNode_YapFragmentWidget::GetNodeTitleColor() const
{
	FLinearColor Color;

	if (GetDialogueNode()->GetDynamicTitleColor(Color))
	{
		return Color;
	}

	return FLinearColor::Black;
}

void SFlowGraphNode_YapFragmentWidget::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	bCtrlPressed = GEditor->GetEditorSubsystem<UYapEditorSubsystem>()->GetInputTracker()->GetControlPressed();

	bool bOwnerSelected = Owner->GetIsSelected();
	
	if (bShowAudioSettings && !bOwnerSelected)
	{
		bShowAudioSettings = false;
		CentreBox->SetContent(CentreDialogueWidget.ToSharedRef());
	}

	if (bTextEditorExpanded && !bOwnerSelected)
	{
		FragmentOverlay->RemoveSlot(ExpandedTextEditorWidget.ToSharedRef());
		bTextEditorExpanded = false;
		ExpandedTextEditorWidget = nullptr;
	}

	if (ExpandedTextEditorWidget && ExpandedTextEditorWidget_OffsetAlpha < 1.0)
	{
		ExpandedTextEditorWidget_OffsetAlpha = ExpandedTextEditorWidget_OffsetAlpha + 6.0 * InDeltaTime;
		
		ExpandedTextEditorWidget_Offset = FMath::InterpEaseOut(ExpandedTextEditorWidget_StartOffset, 0.f, ExpandedTextEditorWidget_OffsetAlpha, 3.0);
		
		ExpandedTextEditorWidget->SetPadding(FMargin(ExpandedTextEditorWidget_Offset, 0, 0, 0));
	}

	if (bOwnerSelected && !MoveFragmentControls.IsValid())
	{
		UE_LOG(LogYap, Verbose, TEXT("Making)))"));
		MoveFragmentControls = CreateFragmentControlsWidget();
		MoveFragmentControls->SetCursor(EMouseCursor::Default);
		FragmentWidgetOverlay->AddSlot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Center)
		.Padding(-28, 0, 0, 0)
		[
			MoveFragmentControls.ToSharedRef()
		];
	}
	else if (MoveFragmentControls.IsValid() && (!bOwnerSelected))
	{
		UE_LOG(LogYap, Verbose, TEXT("Removinging)))"));

		FragmentWidgetOverlay->RemoveSlot(MoveFragmentControls.ToSharedRef());
		MoveFragmentControls = nullptr;
	}

	if (bEditingChildSafeSettings && !bOwnerSelected)
	{
		bEditingChildSafeSettings = false;

		if (!HasAnyChildSafeData())
		{
			GetFragment().bHasChildSafeData = false;
		}
	}
}

TSharedRef<SWidget> SFlowGraphNode_YapFragmentWidget::CreateRightFragmentPane()
{
	SAssignNew(StartPinBox, SBox)
	.WidthOverride(16)
	.HeightOverride(16);
	
	SAssignNew(EndPinBox, SBox)
	.WidthOverride(16)
	.HeightOverride(16);
	
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
		SNew(SLevelOfDetailBranchNode)
		.UseLowDetailSlot(Owner, &SFlowGraphNode_YapDialogueWidget::UseLowDetail)
		.HighDetail()
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
					.Cursor(EMouseCursor::Default)
					.OnClicked(this, &SFlowGraphNode_YapFragmentWidget::OnClicked_EnableOnStartPinButton)
					.ButtonColorAndOpacity(YapColor::LightGray_Trans)
					.ToolTipText(INVTEXT("Click to enable 'On Start' Pin"))
				]
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				StartPinBox.ToSharedRef()
			]
		]
		.LowDetail()
		[
			StartPinBox.ToSharedRef()
		]
	]
	+ SVerticalBox::Slot()
	.HAlign(HAlign_Center)
	.VAlign(VAlign_Bottom)
	.Padding(4, 0, 2, 6)
	.AutoHeight()
	[
		SNew(SLevelOfDetailBranchNode)
		.UseLowDetailSlot(Owner, &SFlowGraphNode_YapDialogueWidget::UseLowDetail)
		.HighDetail()
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
					.Cursor(EMouseCursor::Default)
					.OnClicked(this, &SFlowGraphNode_YapFragmentWidget::OnClicked_EnableOnEndPinButton)
					.ButtonColorAndOpacity(YapColor::LightGray_Trans)
					.ToolTipText(INVTEXT("Click to enable 'On End' Pin"))
				]
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				EndPinBox.ToSharedRef()
			]
		]
		.LowDetail()
		[
			EndPinBox.ToSharedRef()
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
	FYapTransactions::BeginModify(LOCTEXT("YapDialogue", "Enable OnStart Pin"), GetDialogueNode());

	GetFragment().bShowOnStartPin = true;
	
	GetDialogueNode()->ForceReconstruction();
	
	FYapTransactions::EndModify();

	return FReply::Handled();
}

FReply SFlowGraphNode_YapFragmentWidget::OnClicked_EnableOnEndPinButton()
{
	FYapTransactions::BeginModify(LOCTEXT("YapDialogue", "Enable OnEnd Pin"), GetDialogueNode());

	GetFragment().bShowOnEndPin = true;

	GetDialogueNode()->ForceReconstruction();
	
	FYapTransactions::EndModify();
	
	return FReply::Handled();
}

// ================================================================================================
// MOOD KEY SELECTOR WIDGET
// ------------------------------------------------------------------------------------------------

TSharedRef<SWidget> SFlowGraphNode_YapFragmentWidget::CreateMoodKeySelectorWidget()
{
	FGameplayTag SelectedMoodKey = GetCurrentMoodKey();

	TSharedRef<SUniformWrapPanel> MoodTagSelectorPanel = SNew(SUniformWrapPanel)
		.NumColumnsOverride(4); // TODO use maff

	MoodTagSelectorPanel->AddSlot()
	[
		CreateMoodKeyMenuEntryWidget(FGameplayTag::EmptyTag, SelectedMoodKey == FGameplayTag::EmptyTag)
	];
	
	for (const FGameplayTag& MoodKey : UYapProjectSettings::Get()->GetMoodTags())
	{
		if (!MoodKey.IsValid())
		{
			UE_LOG(LogYap, Warning, TEXT("Warning: Portrait keys contains an invalid entry. Clean this up!"));
			continue;
		}
		
		bool bSelected = MoodKey == SelectedMoodKey;
		
		MoodTagSelectorPanel->AddSlot()
		[
			CreateMoodKeyMenuEntryWidget(MoodKey, bSelected)
		];
	}

	// TODO ensure that system works and displays labels if user does not supply icons but only FNames. Use Generic mood icon?
	return SNew(SComboButton)
		.Cursor(EMouseCursor::Default)
		.HasDownArrow(false)
		.ButtonStyle(FYapEditorStyle::Get(), YapStyles.ButtonStyle_DialogueCornerFoldout)
		.ContentPadding(FMargin(0.f, 0.f))
		.MenuPlacement(MenuPlacement_CenteredBelowAnchor)
		//.ButtonColorAndOpacity(FSlateColor(FLinearColor(0.f, 0.f, 0.f, 0.5f)))
		.HAlign(HAlign_Center)
		.ButtonStyle(FAppStyle::Get(), "SimpleButton")
		.OnMenuOpenChanged(this, &SFlowGraphNode_YapFragmentWidget::OnMenuOpenChanged_MoodKeySelector)
		.ToolTipText(this, &SFlowGraphNode_YapFragmentWidget::ToolTipText_MoodKeySelector)
		.ForegroundColor(this, &SFlowGraphNode_YapFragmentWidget::ForegroundColor_MoodKeySelectorWidget)
		.ButtonContent()
		[
			SNew(SImage)
			.ColorAndOpacity(FSlateColor::UseSubduedForeground())
			.Image(this, &SFlowGraphNode_YapFragmentWidget::Image_MoodKeySelector)
		]
		.MenuContent()
		[
			MoodTagSelectorPanel
		];
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
	return GetBit().GetMoodKey();
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
	Brush.ImageSize = FVector2D(24, 24);
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

	FText ToolTipText;
	
	if (MoodKey.IsValid())
	{
		TSharedPtr<FGameplayTagNode> TagNode = UGameplayTagsManager::Get().FindTagNode(MoodKey);
		ToolTipText = FText::FromName(TagNode->GetSimpleTagName());
	}
	else
	{
		ToolTipText = INVTEXT("Default");
	}
	
	return SNew(SButton)
	.Cursor(EMouseCursor::Default)
	.ContentPadding(FMargin(4, 4))
	.ButtonStyle(FAppStyle::Get(), "SimpleButton")
	.ClickMethod(EButtonClickMethod::MouseDown)
	.OnClicked(this, &SFlowGraphNode_YapFragmentWidget::OnClicked_MoodKeyMenuEntry, MoodKey)
	.ToolTipText(ToolTipText)
	[
		SNew(SOverlay)
		+ SOverlay::Slot()
		.Padding(-3)
		[
			SNew(SBorder)
			.Visibility_Lambda([this, MoodKey]()
			{
				if (GetFragment().GetBit().GetMoodKey() == MoodKey)
				{
					return EVisibility::Visible;
				}
				
				return EVisibility::Collapsed;
			})
			.BorderImage(FYapEditorStyle::GetImageBrush(YapBrushes.Border_RoundedSquare))
			.BorderBackgroundColor(YapColor::White_Trans)
		]
		+ SOverlay::Slot()
		[
			SAssignNew(PortraitIconImage, SImage)
			.ColorAndOpacity(FSlateColor::UseForeground())
			.Image(TAttribute<const FSlateBrush*>::Create(TAttribute<const FSlateBrush*>::FGetter::CreateLambda([MoodKeyBrush](){return MoodKeyBrush->GetSlateBrush();})))	
		]
	];
}

FReply SFlowGraphNode_YapFragmentWidget::OnClicked_MoodKeyMenuEntry(FGameplayTag NewValue)
{	
	FYapTransactions::BeginModify(LOCTEXT("NodeMoodKeyChanged", "Portrait Key Changed"), GetDialogueNode());

	GetBit().SetMoodKey(NewValue);

	FYapTransactions::EndModify();

	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE