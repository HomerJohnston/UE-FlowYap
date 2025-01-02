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
#include "Yap/Testing/SYapPropertyMenuAssetPicker.h"

bool SFlowGraphNode_YapFragmentWidget::UseChildSafeSettings() const
{
	return GetFragment().bHasChildSafeData;
}

FReply SFlowGraphNode_YapFragmentWidget::OnClicked_DialogueSettingsButton()
{
	bShowSettings = !bShowSettings;

	if (bShowSettings)
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
			CentreDialogueWidget = CreateCentreDialogueWidget();
		}
		CentreBox->SetContent(CentreDialogueWidget.ToSharedRef());
	}

	return FReply::Handled();
}

TSharedPtr<SWidget> SFlowGraphNode_YapFragmentWidget::CreateCentreDialogueWidget()
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
			CreateDialogueWidget()
		]
		+ SVerticalBox::Slot()
		.Padding(0, 4, 0, 0)
		.AutoHeight()
		[
			CreateTitleTextWidget()
		]
	]
	+ SOverlay::Slot()
	.HAlign(HAlign_Right)
	.VAlign(VAlign_Top)
	.Padding(0, 0, 0, 0)
	[
		SNew(SButton)
		.Cursor(EMouseCursor::Default)
		.ButtonStyle(FYapEditorStyle::Get(), YapStyles.ButtonStyle_DialogueExpander)
		.OnClicked(this, &SFlowGraphNode_YapFragmentWidget::OnClicked_DialogueSettingsButton)
		.ContentPadding(0)
		[
			SNew(SImage)
			.Image(FYapEditorStyle::GetImageBrush(YapBrushes.Icon_SettingsExpander))
			.ColorAndOpacity(FSlateColor::UseForeground())
		]
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
					GetFragment().GetBitMutable().SetDialogueAudioAsset(InAssetData.GetAsset());
				})
			)
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			CreateAudioPreviewWidget()
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
					GetFragment().GetBitMutable().SetDialogueAudioAssetSafe(InAssetData.GetAsset());
				}
				)
			)
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			CreateAudioPreviewWidget()
		]
	]
	+ SVerticalBox::Slot()
	.VAlign(VAlign_Bottom)
	.HAlign(HAlign_Fill)
	.Padding(0, 1, 0, 0)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.HAlign(HAlign_Left)
		.AutoWidth()
		[
			CreateMoodKeySelectorWidget()
		]
		+ SHorizontalBox::Slot()
		.HAlign(HAlign_Fill)
		[
			SNew(SSpacer)
		]
		+ SHorizontalBox::Slot()
		.HAlign(HAlign_Right)
		.AutoWidth()
		[
			CreateBottomRowWidget()
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
	return bTextEditorExpanded || bShowSettings;
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
	.Cursor(EMouseCursor::Default)
	.ContentPadding(0)
	.ButtonStyle(FYapEditorStyle::Get(), YapStyles.ButtonStyle_DialogueExpander)
	.Visibility(Attribute)
	.IsEnabled(this, &SFlowGraphNode_YapFragmentWidget::Enabled_AudioPreviewButton)
	.ToolTipText(INVTEXT("Play audio"))
	[
		SNew(SImage)
		.DesiredSizeOverride(FVector2D(14, 14))
		.Image(FYapEditorStyle::GetImageBrush(YapBrushes.Icon_Speaker))
		.ColorAndOpacity(FSlateColor::UseForeground())
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
			YapEditor::CreateMaturityCheckBox
			(
				TAttribute<ECheckBoxState>::CreateSP(this, &SFlowGraphNode_YapFragmentWidget::IsChecked_MaturitySettings),
				FOnCheckStateChanged::CreateSP(this, &SFlowGraphNode_YapFragmentWidget::OnCheckStateChanged_MaturitySettings),
				TAttribute<FSlateColor>::CreateSP(this, &SFlowGraphNode_YapFragmentWidget::ColorAndOpacity_ChildSafeSettingsCheckBox),
				TAttribute<EVisibility>::CreateLambda( [this] () { return UseChildSafeSettings() ? EVisibility::Visible : EVisibility::Hidden; } )
			)
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
			//.OnConditionDetailsViewBuilt(Owner, &SFlowGraphNode_YapDialogueWidget::OnConditionDetailsViewBuilt)
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
	const FYapBit& Bit = GetFragment().GetBit();

	if (!GetFragment().bHasChildSafeData)
	{
		return ECheckBoxState::Unchecked;
	}
	
	bool bHasDialogueTextSafe = !Bit.GetSafeDialogueText().IsEmpty();
	bool bHasTitleTextSafe = Bit.GetMatureTitleText().IsEmpty() ? true : !Bit.GetSafeTitleText().IsEmpty();
	bool bHasAudio = !Bit.HasDialogueAudioAsset() ? true : Bit.HasDialogueAudioAssetSafe();

	if (bHasDialogueTextSafe && bHasTitleTextSafe && bHasAudio)
	{
		return ECheckBoxState::Checked;
	}

	if (bHasDialogueTextSafe || bHasTitleTextSafe || bHasAudio)
	{
		return ECheckBoxState::Undetermined;
	}

	return ECheckBoxState::Unchecked;
}

void SFlowGraphNode_YapFragmentWidget::OnCheckStateChanged_MaturitySettings(ECheckBoxState CheckBoxState)
{
	GetFragment().bHasChildSafeData = !GetFragment().bHasChildSafeData;
}

FSlateColor SFlowGraphNode_YapFragmentWidget::ColorAndOpacity_ChildSafeSettingsCheckBox() const
{
	const FYapBit& Bit = GetFragment().GetBit();

	if (!GetFragment().GetHasChildSafeData())
	{
		return YapColor::DarkGray;
	}
	
	bool bHasDialogueText = !Bit.GetMatureDialogueText().IsEmpty();
	bool bHasTitleText = !Bit.GetMatureTitleText().IsEmpty();
	bool bHasAudio = Bit.HasDialogueAudioAsset();
	
	bool bHasDialogueTextSafe = !Bit.GetSafeDialogueText().IsEmpty();
	bool bHasTitleTextSafe = !Bit.GetSafeTitleText().IsEmpty();
	bool bHasAudioSafe = Bit.HasDialogueAudioAssetSafe();

	FLinearColor ActiveModifier = UseChildSafeSettings() ? YapColor::White : YapColor::White_Trans;
	
	if (bHasDialogueTextSafe || bHasTitleTextSafe || bHasAudioSafe)
	{
		if ((bHasDialogueText ? bHasDialogueTextSafe : true) && (bHasTitleText ? bHasTitleTextSafe : true) && (bHasAudio ? bHasAudioSafe : true))
		{
			return YapColor::LightBlue * ActiveModifier;
		}
		
		return YapColor::OrangeRed * ActiveModifier;
	}
	
	return YapColor::DarkGray * ActiveModifier;
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

		GetFragment().Bit.SetDirectedAt(Character);

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
	}
	
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

TSharedRef<SOverlay> SFlowGraphNode_YapFragmentWidget::CreateDirectedAtWidget()
{
	int32 PortraitSize = UYapEditorSettings::Get()->GetPortraitSize() / 3;
	
	return SNew(SOverlay)
	+ SOverlay::Slot()
	.Padding(0, 0, 0, 0)
	[
		SNew(SBox)
		.WidthOverride(PortraitSize)
		.HeightOverride(PortraitSize)
		.Visibility(EVisibility::HitTestInvisible)
		[
			SNew(SBorder)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			.BorderImage(FYapEditorStyle::GetImageBrush(YapBrushes.Panel_Rounded))
			.BorderBackgroundColor(this, &SFlowGraphNode_YapFragmentWidget::BorderBackgroundColor_DirectedAtImage)
		]
	]
	+ SOverlay::Slot()
	.Padding(0)
	[
		SNew(SLevelOfDetailBranchNode)
		.UseLowDetailSlot(Owner, &SFlowGraphNode_YapDialogueWidget::UseLowDetail)
		.HighDetail()
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			.Padding(0)
			[
				SNew(SAssetDropTarget)
				.bSupportsMultiDrop(false)
				.OnAreAssetsAcceptableForDrop(this, &SFlowGraphNode_YapFragmentWidget::OnAreAssetsAcceptableForDrop_DirectedAtWidget)
				.OnAssetsDropped(this, &SFlowGraphNode_YapFragmentWidget::OnAssetsDropped_DirectedAtWidget)
				[
					SNew(SButton)
					.Cursor(EMouseCursor::Default)
					.ButtonStyle(FAppStyle::Get(), "SimpleButton")
					//.ToolTipText(this, &SFlowGraphNode_YapFragmentWidget::ToolTipText_PortraitWidget)
					.ContentPadding(0)
					.OnClicked(this, &SFlowGraphNode_YapFragmentWidget::OnClicked_DirectedAtWidget)
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
								.Image(this, &SFlowGraphNode_YapFragmentWidget::Image_DirectedAtWidget)	
							]
						]
					]
				]
			]
		]
		.LowDetail()
		[
			SNew(SImage)
			.DesiredSizeOverride(FVector2D(PortraitSize, PortraitSize))
			.Image(this, &SFlowGraphNode_YapFragmentWidget::Image_DirectedAtWidget)
		]
	];
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
			.VAlign(VAlign_Fill)
			[
				SNew(SBox)
				.WidthOverride(32)
				.HeightOverride(32)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
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
							.VAlign(VAlign_Top)
							.HAlign(HAlign_Right)
							.Padding(0, -2, -2, 0)
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

// ================================================================================================
// DIALOGUE WIDGET
// ------------------------------------------------------------------------------------------------

TSharedRef<SWidget> SFlowGraphNode_YapFragmentWidget::CreateDialogueWidget()
{
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
			.OnClicked(this, &SFlowGraphNode_YapFragmentWidget::OnClicked_ExpandTextEditor, &GetBit().MatureDialogueText, &GetBit().SafeDialogueText, YapStyles.EditableTextBoxStyle_Dialogue, YapColor::White)
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
						.Text(this, &SFlowGraphNode_YapFragmentWidget::Text_PreviewText, &GetBit().GetMatureDialogueText(), &GetBit().GetMatureDialogueText())
						.ToolTipText(this, &SFlowGraphNode_YapFragmentWidget::ToolTipText_PreviewText, INVTEXT("Dialogue Text"), &GetBit().GetMatureDialogueText(), &GetBit().GetSafeDialogueText())
						.ColorAndOpacity(this, &SFlowGraphNode_YapFragmentWidget::ColorAndOpacity_PreviewText, YapColor::White, &GetBit().GetMatureDialogueText(), &GetBit().GetMatureDialogueText())
					]
				]
				+ SOverlay::Slot()
				.VAlign(VAlign_Center)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Visibility(this, &SFlowGraphNode_YapFragmentWidget::Visibility_EmptyTextIndicator, &GetBit().GetMatureDialogueText())
					.Justification(ETextJustify::Center)
					.TextStyle(FYapEditorStyle::Get(), YapStyles.TextBlockStyle_DialogueText)
					.Text(INVTEXT("Dialogue Text (None)"))
					.ColorAndOpacity(YapColor::White_Glass)
				]
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
				.FillColorAndOpacity(this, &SFlowGraphNode_YapFragmentWidget::FillColorAndOpacity_FragmentTimePadding)
				.BarFillType(EProgressBarFillType::FillFromCenterHorizontal)
			]
		]
		+ SOverlay::Slot()
		.VAlign(VAlign_Bottom)
		.HAlign(HAlign_Right)
		.Padding(4)
		[
			CreateAudioPreviewWidget(TAttribute<EVisibility>::CreateSP(this, &SFlowGraphNode_YapFragmentWidget::Visibility_AudioButton))
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

FText SFlowGraphNode_YapFragmentWidget::Text_PreviewText(const FText* MatureText, const FText* SafeText) const
{
	if (GEditor->PlayWorld)
	{
		return GetFragment().GetHasChildSafeData() ? *MatureText : *SafeText;
	}
	
	return *MatureText;
}

/*
FSlateColor SFlowGraphNode_YapFragmentWidget::ColorAndOpacity_DialogueText() const
{
	FLinearColor Color = GetDialogueNode()->GetIsPlayerPrompt() ? YapColor::White : YapColor::LightGray;
	
	if (GEditor->PlayWorld)
	{		
		if (GetDialogueNode()->GetRunningFragmentIndex() == GetFragment().IndexInDialogue)
		{
			Color = YapColor::White;
		}
		else
		{
			Color = (GetFragment().IsActivationLimitMet()) ? YapColor::DarkRed : YapColor::LightGray;
		}
	}

	if (GetFragment().GetBitReplaced())
	{
		Color *= YapColor::LightGreen;
	}

	if (UseChildSafeSettings())
	{
		Color *= YapColor::LightBlue;
	}
	
	return Color;
}
*/

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
	const float MaxPaddedSetting = UYapProjectSettings::Get()->GetFragmentPaddingSliderMax();
	const float FragmentPadding = GetFragment().GetPaddingToNextFragment();

	if (GEditor->PlayWorld)
	{
		const TOptional<uint8>& RunningIndex = GetDialogueNode()->RunningFragmentIndex;
		
		if (RunningIndex == GetFragment().IndexInDialogue) // TODO IndexInDialogue isn't being updated maybe when I add fragments from the details pane?
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
		else if (!RunningIndex.IsSet() || RunningIndex.GetValue() < GetFragment().IndexInDialogue)
		{
			return FragmentPadding / MaxPaddedSetting;
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
		const TOptional<uint8>& RunningIndex = GetDialogueNode()->RunningFragmentIndex;

		if (RunningIndex == GetFragment().IndexInDialogue)
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

FSlateColor SFlowGraphNode_YapFragmentWidget::BorderBackgroundColor_PortraitImage() const
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

	if (!GetDialogueNode()->GetIsPlayerPrompt())
	{
		Color.A *= 0.5f;
	}

	return Color;
}

FText SFlowGraphNode_YapFragmentWidget::ToolTipText_PortraitWidget() const
{
	const FYapBit& Bit = GetFragment().GetBit();

	if (Bit.GetSpeakerAsset().IsPending())
	{
		return INVTEXT("Unloaded");
	}
	else if (Bit.GetSpeakerAsset().IsNull())
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

void SFlowGraphNode_YapFragmentWidget::OnSetNewSpeaker(const FAssetData& AssetData)
{
	FYapTransactions::BeginModify(INVTEXT("Change character asset"), GetDialogueNode());
	
	GetBit().SetSpeaker(AssetData.GetAsset());

	FYapTransactions::EndModify();
}

FReply SFlowGraphNode_YapFragmentWidget::OnClicked_PortraitWidget()
{
	TSharedPtr<SWidget> Widget = SNew(SBorder)
		.Padding(1, 1, 1, 1) // No idea why but the details view already has a 4 pixel transparent area on top
		.BorderImage(FYapEditorStyle::GetImageBrush(YapBrushes.Box_SolidLightGray_Rounded))
		.BorderBackgroundColor(YapColor::DimGray)
		[
			SNew(SYapPropertyMenuAssetPicker)
			.AllowedClasses({UYapCharacter::StaticClass()})
			.AllowClear(true)
			.InitialObject(GetBit().GetSpeaker())
			.OnSet(this, &SFlowGraphNode_YapFragmentWidget::OnSetNewSpeaker)
		];

	Owner->AddOverlayWidget(PortraitWidget, Widget);
	
	return FReply::Handled();

#if 0
	bShowSettings = !bShowSettings;

	if (bShowSettings)
	{
		if (CentreSettingsWidget == nullptr)
		{
			CentreSettingsWidget = CreateCentreSettingsWidget();
		}
		
		CentreBox->SetContent(CentreSettingsWidget.ToSharedRef());

		Owner->SetNodeSelected();
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
#endif
}

FText SFlowGraphNode_YapFragmentWidget::Text_PortraitWidget() const
{
	const FYapBit& Bit = GetFragment().GetBit();
	
	if (Bit.GetSpeakerAsset().IsNull())
	{
		return INVTEXT("Character\nUnset");
	}
	
	if (Image_PortraitImage() == nullptr)
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

bool SFlowGraphNode_YapFragmentWidget::OnAreAssetsAcceptableForDrop_PortraitWidget(TArrayView<FAssetData> AssetDatas) const
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

void SFlowGraphNode_YapFragmentWidget::OnAssetsDropped_PortraitWidget(const FDragDropEvent& DragDropEvent, TArrayView<FAssetData> AssetDatas)
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

TSharedRef<SOverlay> SFlowGraphNode_YapFragmentWidget::CreatePortraitWidget()
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
			SNew(SOverlay)
			+ SOverlay::Slot()
			[
				SNew(SAssetDropTarget)
				.bSupportsMultiDrop(false)
				.OnAreAssetsAcceptableForDrop(this, &SFlowGraphNode_YapFragmentWidget::OnAreAssetsAcceptableForDrop_PortraitWidget)
				.OnAssetsDropped(this, &SFlowGraphNode_YapFragmentWidget::OnAssetsDropped_PortraitWidget)
				[
					SNew(SButton)
					.Cursor(EMouseCursor::Default)
					.ButtonStyle(FAppStyle::Get(), "SimpleButton")
					.ToolTipText(this, &SFlowGraphNode_YapFragmentWidget::ToolTipText_PortraitWidget)
					.ContentPadding(0)
					.OnClicked(this, &SFlowGraphNode_YapFragmentWidget::OnClicked_PortraitWidget)
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
								.Image(this, &SFlowGraphNode_YapFragmentWidget::Image_PortraitImage)	
							]
						]
						+ SOverlay::Slot()
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Center)
						[
							SNew(STextBlock)
							.Text(this, &SFlowGraphNode_YapFragmentWidget::Text_PortraitWidget)
							.Font(FCoreStyle::GetDefaultFontStyle("Normal", 8))
							.ColorAndOpacity(YapColor::Red)
							.Justification(ETextJustify::Center)
						]
					]
				]
			]
		]
		.LowDetail()
		[
			SNew(SImage)
			.DesiredSizeOverride(FVector2D(PortraitSize, PortraitSize))
			.Image(this, &SFlowGraphNode_YapFragmentWidget::Image_PortraitImage)
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
			.BorderBackgroundColor(this, &SFlowGraphNode_YapFragmentWidget::BorderBackgroundColor_PortraitImage)
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

const FSlateBrush* SFlowGraphNode_YapFragmentWidget::Image_PortraitImage() const
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
	const TSoftObjectPtr<UYapCharacter> Asset = GetFragment().Bit.GetSpeakerAsset();

	if (Asset.IsPending())
	{
		Asset.LoadSynchronous();
	}
	
	if (!Asset) { return ""; }

	return Asset.ToString();
}

void SFlowGraphNode_YapFragmentWidget::OnObjectChanged_CharacterSelect(const FAssetData& InAssetData)
{
	FYapTransactions::BeginModify(LOCTEXT("NodeCharacterChanged", "Character Changed"), GetDialogueNode());

	UObject* Asset = InAssetData.GetAsset();

	UYapCharacter* Character = Cast<UYapCharacter>(Asset);
	
	GetFragment().Bit.SetSpeaker(Character);

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

// ================================================================================================
// TITLE TEXT WIDGET
// ------------------------------------------------------------------------------------------------

TSharedRef<SWidget> SFlowGraphNode_YapFragmentWidget::CreateTitleTextWidget()
{
	TitleTextEditButtonWidget = SNew(SButton)
	.Cursor(EMouseCursor::Default)
	.Visibility(this, &SFlowGraphNode_YapFragmentWidget::Visibility_TitleText)
	.ToolTipText(INVTEXT("Title text"))
	.ButtonStyle(FYapEditorStyle::Get(),YapStyles.ButtonStyle_ActivationLimit)
	.OnClicked(this, &SFlowGraphNode_YapFragmentWidget::OnClicked_ExpandTextEditor, &GetBit().MatureTitleText, &GetBit().SafeTitleText, YapStyles.EditableTextBoxStyle_TitleText, YapColor::YellowGray)
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
				.Text(this, &SFlowGraphNode_YapFragmentWidget::Text_PreviewText, &GetBit().GetMatureTitleText(), &GetBit().GetSafeTitleText())
				.ToolTipText(this, &SFlowGraphNode_YapFragmentWidget::ToolTipText_PreviewText, INVTEXT("Title Text"), &GetBit().GetMatureTitleText(), &GetBit().GetSafeTitleText())
				.ColorAndOpacity(this, &SFlowGraphNode_YapFragmentWidget::ColorAndOpacity_PreviewText, YapColor::YellowGray, &GetBit().GetMatureTitleText(), &GetBit().GetSafeTitleText())// [this] () { return UseChildSafeSettings() ? YapColor::YellowGray * YapColor::LightBlue : YapColor::YellowGray; } )
			]
		]
		+ SOverlay::Slot()
		.VAlign(VAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Visibility(this, &SFlowGraphNode_YapFragmentWidget::Visibility_EmptyTextIndicator, &GetBit().GetMatureTitleText())
			.Justification(ETextJustify::Center)
			.TextStyle(FYapEditorStyle::Get(), YapStyles.TextBlockStyle_TitleText)
			.Text(INVTEXT("Title Text (None)"))
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

void SFlowGraphNode_YapFragmentWidget::OnTextCommitted_EditedText(const FText& NewValue, ETextCommit::Type CommitType, FText* EditedText)
{
	FYapTransactions::BeginModify(LOCTEXT("Yap Dialogue Node", "Text Changed"), GetDialogueNode());

	if (CommitType != ETextCommit::OnCleared)
	{
		*EditedText = NewValue;
	}

	FYapTransactions::EndModify();
}

FReply SFlowGraphNode_YapFragmentWidget::OnClicked_ExpandTextEditor(FText* MatureText, FText* SafeText, FName TextStyle, FLinearColor BaseColor)
{
	if (bTextEditorExpanded)
	{
		return FReply::Handled();
	}
		
	TSharedRef<IEditableTextProperty> EditableTextProperty = MakeShareable(new FYapEditableTextPropertyHandle(*MatureText));
	TSharedRef<IEditableTextProperty> EditableTextPropertySafe = MakeShareable(new FYapEditableTextPropertyHandle(*SafeText));

	float TotalPortraitWidgetSize = UYapEditorSettings::Get()->GetPortraitSize() + 8;
	float FullHeight = TotalPortraitWidgetSize - 4;
	float HalfHeight = FullHeight / 2;

	ExpandedTextEditorWidget_StartOffset = TotalPortraitWidgetSize + 5;
	ExpandedTextEditorWidget_Offset = ExpandedTextEditorWidget_StartOffset;
	ExpandedTextEditorWidget_OffsetAlpha = 0.0;
	
	ExpandedTextEditorWidget = SNew(SBox)
	.Padding(ExpandedTextEditorWidget_Offset, 0, 0, 0)
	[
		SNew(SOverlay)
		+ SOverlay::Slot()
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.Padding(0, 0, 0, 0)
			.AutoHeight()
			[
				SNew(SBox)
				.MinDesiredHeight_Lambda( [this, HalfHeight, FullHeight] () { return UseChildSafeSettings() ? HalfHeight : FullHeight; } )
				[
					SNew(SYapTextPropertyEditableTextBox, EditableTextProperty)
					.Style(FYapEditorStyle::Get(), TextStyle)
					.Text(this, &SFlowGraphNode_YapFragmentWidget::Text_EditedText, MatureText)
					.OnTextCommitted(this, &SFlowGraphNode_YapFragmentWidget::OnTextCommitted_EditedText, MatureText)
					//.Font(FontInfo)
					.ForegroundColor(BaseColor)
					.Cursor(EMouseCursor::Default)
				]
			]
			+ SVerticalBox::Slot()
			.Padding(0, 4, 0, 0)
			.AutoHeight()
			[
				SNew(SBox)
				.MinDesiredHeight(HalfHeight)
				[
					SNew(SYapTextPropertyEditableTextBox, EditableTextPropertySafe)
					.Style(FYapEditorStyle::Get(), TextStyle)
					.Text(this, &SFlowGraphNode_YapFragmentWidget::Text_EditedText, SafeText)
					.OnTextCommitted(this, &SFlowGraphNode_YapFragmentWidget::OnTextCommitted_EditedText, SafeText)
					.ForegroundColor(BaseColor * YapColor::LightBlue)
					.Cursor(EMouseCursor::Default)
					.Visibility_Lambda( [this] () { return UseChildSafeSettings() ? EVisibility::Visible : EVisibility::Collapsed; })
				]
			]
		]
	];
	/*
	+ SOverlay::Slot()
	.HAlign(HAlign_Fill)
	.VAlign(VAlign_Bottom)
	.Padding(0, 0, 28, -10)
	[
		SNew(SOverlay)
		+ SOverlay::Slot()
		.Padding(8, 0)
		[
			SNew(SBox)
			.HeightOverride(2)
			[
				SNew(SProgressBar)
				.Cursor(EMouseCursor::Default)
				.BorderPadding(0)
				.Percent(this, &SFlowGraphNode_YapFragmentWidget::FragmentTimePadding_Percent)
				.Style(FYapEditorStyle::Get(), YapStyles.ProgressBarStyle_FragmentTimePadding)
				.FillColorAndOpacity(this, &SFlowGraphNode_YapFragmentWidget::FillColorAndOpacity_FragmentTimePadding)
			]
		]
		+ SOverlay::Slot()
		.Padding(0, -2)
		[
			SNew(SSlider)
			.Cursor(EMouseCursor::Default)
			.Value(this, &SFlowGraphNode_YapFragmentWidget::Value_FragmentTimePadding)
			.OnValueChanged(this, &SFlowGraphNode_YapFragmentWidget::OnValueChanged_FragmentTimePadding)
			.Style(FYapEditorStyle::Get(), YapStyles.SliderStyle_FragmentTimePadding)
			.SliderHandleColor(YapColor::Gray)
			.ToolTipText(this, &SFlowGraphNode_YapFragmentWidget::ToolTipText_FragmentTimePadding)
		]
	];
	*/
	
	FragmentOverlay->AddSlot()
	.Padding(-1, 0, -1, 0)
	[
		ExpandedTextEditorWidget.ToSharedRef()
	];

	bTextEditorExpanded = true;

	Owner->SetNodeSelected();
	
	return FReply::Handled();

}

FText SFlowGraphNode_YapFragmentWidget::ToolTipText_PreviewText(FText Label, const FText* MatureText, const FText* SafeText) const
{
	if (bCtrlPressed)
	{
		FText Unset = INVTEXT("\u26A0 No text \u26A0");//" !! \u203C \u270D \u2756 \u26A0");
		
		if (GetFragment().GetHasChildSafeData())
		{
			return FText::Format(INVTEXT("{0}\n\n\u2668\u2502{1}\n\n\u26F9\u2502{2}"), Label, MatureText->IsEmpty() ? Unset : *MatureText, SafeText->IsEmpty() ? Unset : *SafeText);
		}
		else
		{
			return FText::Format(INVTEXT("{0}\n\n\u2756\u2502{1}"), Label, MatureText->IsEmpty() ? Unset : *MatureText);
		}
	}
	else
	{
		return INVTEXT("Hold CTRL to show");
	}
}

FSlateColor SFlowGraphNode_YapFragmentWidget::ColorAndOpacity_PreviewText(FLinearColor BaseColor, const FText* MatureText, const FText* SafeText) const
{
	FLinearColor Color = BaseColor;
	
	if (GEditor->PlayWorld)
	{
		if (GetFragment().GetHasChildSafeData() && !SafeText->IsEmpty())
		{
			Color *= YapColor::LightBlue;
		}
	}
	
	return Color;
}

EVisibility SFlowGraphNode_YapFragmentWidget::Visibility_TitleText() const
{
	if (GetDialogueNode()->GetIsPlayerPrompt())
	{
		return EVisibility::Visible;
	}
	
	return UYapProjectSettings::Get()->GetHideTitleTextOnNPCDialogueNodes() ? EVisibility::Collapsed : EVisibility::Visible;
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
					.Cursor(EMouseCursor::Default)
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
					.Cursor(EMouseCursor::Default)
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
					.Cursor(EMouseCursor::Default)
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
					.Cursor(EMouseCursor::Default)
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
	FYapTransactions::BeginModify(LOCTEXT("NodeUseProjectDefaultTimeSettings", "Use Project Default Time Settings Changed"), GetDialogueNode());

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
	FYapTransactions::BeginModify(LOCTEXT("Fragment", "Fragment Time Mode Changed"), GetDialogueNode());

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
	FYapTransactions::BeginModify(LOCTEXT("Fragment", "Fragment Time Mode Changed"), GetDialogueNode());

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
	FYapTransactions::BeginModify(LOCTEXT("NodeTimedModeChanged", "Timed Mode Changed"), GetDialogueNode());

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
	FYapTransactions::BeginModify(LOCTEXT("NodeEnteredTimeChanged", "Entered Time Changed"), GetDialogueNode());

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
	const UObject* Asset = GetFragment().Bit.GetDialogueAudioAsset<UObject>();

	if (!Asset) { return ""; }

	return Asset->GetPathName();
}

void SFlowGraphNode_YapFragmentWidget::OnObjectChanged_AudioAsset(const FAssetData& InAssetData)
{
	FYapTransactions::BeginModify(LOCTEXT("NodeAudioAssetChanged", "Audio Asset Changed"), GetDialogueNode());

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
	if (GetDialogueNode()->GetRunningFragmentIndex() == FragmentIndex)
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
	if (GetDialogueNode()->GetRunningFragmentIndex() == FragmentIndex)
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
	
	if (bShowSettings && !bOwnerSelected)
	{
		bShowSettings = false;
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

TSharedRef<SBox> SFlowGraphNode_YapFragmentWidget::CreateMoodKeySelectorWidget()
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
	return SNew(SBox)
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
		.ToolTipText(this, &SFlowGraphNode_YapFragmentWidget::ToolTipText_MoodKeySelector)
		.ButtonContent()
		[
			SNew(SBox)
			.Padding(2, 2)
			[
				SNew(SImage)
				.ColorAndOpacity(FSlateColor::UseForeground())
				.Image(this, &SFlowGraphNode_YapFragmentWidget::Image_MoodKeySelector)
			]
		]
		.MenuContent()
		[
			MoodTagSelectorPanel
		]
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

	GetFragment().Bit.SetMoodKey(NewValue);

	FYapTransactions::EndModify();

	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE