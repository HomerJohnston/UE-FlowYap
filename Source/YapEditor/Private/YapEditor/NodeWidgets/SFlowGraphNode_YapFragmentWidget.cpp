// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license.

#include "YapEditor/NodeWidgets/SFlowGraphNode_YapFragmentWidget.h"

#include "Engine/World.h"
#include "PropertyCustomizationHelpers.h"
#include "SAssetDropTarget.h"
#include "SLevelOfDetailBranchNode.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "Widgets/Notifications/SProgressBar.h"
#include "Yap/YapCharacter.h"
#include "YapEditor/YapColors.h"
#include "YapEditor/YapEditorSubsystem.h"
#include "Yap/YapFragment.h"
#include "Yap/YapProjectSettings.h"
#include "YapEditor/YapTransactions.h"
#include "Yap/YapUtil.h"
#include "YapEditor/YapEditorStyle.h"
#include "Yap/Enums/YapMissingAudioErrorLevel.h"
#include "Yap/Nodes/FlowNode_YapDialogue.h"
#include "YapEditor/NodeWidgets/SFlowGraphNode_YapDialogueWidget.h"
#include "YapEditor/SlateWidgets/SGameplayTagComboFiltered.h"
#include "Yap/YapBitReplacement.h"
#include "YapEditor/YapInputTracker.h"
#include "YapEditor/Helpers/SYapTextPropertyEditableTextBox.h"
#include "YapEditor/Helpers/YapEditableTextPropertyHandle.h"
#include "YapEditor/NodeWidgets/SActivationCounterWidget.h"
#include "YapEditor/NodeWidgets/SYapConditionsScrollBox.h"
#include "YapEditor/NodeWidgets/SYapButtonPopup.h"
#include "YapEditor/Testing/SYapPropertyMenuAssetPicker.h"
#include "Templates/FunctionFwd.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Yap/YapSubsystem.h"
#include "Yap/Enums/YapErrorLevel.h"
#include "YapEditor/YapDeveloperSettings.h"
#include "Framework/MultiBox/SToolBarButtonBlock.h"
#include "YapEditor/Helpers/ProgressionSettingWidget.h"

#define LOCTEXT_NAMESPACE "YapEditor"



bool SFlowGraphNode_YapFragmentWidget::NeedsChildSafeData() const
{
	return GetBit().NeedsChildSafeData();
}

bool SFlowGraphNode_YapFragmentWidget::HasAnyChildSafeData() const
{
	const FYapBit& Bit = GetBit();

	bool bHasSafeDialogueText = !Bit.SafeDialogueText.IsEmpty();
	bool bHasSafeTitleText = !Bit.SafeTitleText.IsEmpty();
	bool bHasSafeAudio = !Bit.SafeAudioAsset.IsNull();

	return (bHasSafeDialogueText || bHasSafeTitleText || bHasSafeAudio);
}

bool SFlowGraphNode_YapFragmentWidget::HasCompleteChildSafeData() const
{
	const FYapBit& Bit = GetBit();

	if (!HasAnyChildSafeData())
	{
		return false;
	}

	// For all three elements, if the mature text is set, then the safe text must also be set
	bool bDialogueTextOK = Bit.MatureDialogueText.IsEmpty() ? true : !Bit.SafeDialogueText.IsEmpty();
	bool bTitleTextOK = Bit.MatureTitleText.IsEmpty() ? true : !Bit.SafeTitleText.IsEmpty(); 
	bool bAudioAssetOK = Bit.MatureAudioAsset.IsNull() ? true : !Bit.SafeAudioAsset.IsNull();

	return bDialogueTextOK && bTitleTextOK && bAudioAssetOK;
}

TSharedRef<SWidget> SFlowGraphNode_YapFragmentWidget::CreateCentreTextDisplayWidget()
{
	return SNew(SYapButtonPopup)
	.ButtonStyle(FYapEditorStyle::Get(), YapStyles.ButtonStyle_NoBorder)
	.PopupContentGetter(FPopupContentGetter::CreateSP(this, &SFlowGraphNode_YapFragmentWidget::PopupContentGetter_ExpandedEditor))
	.PopupPlacement(MenuPlacement_Center)
	.ButtonForegroundColor(YapColor::DarkGray_SemiGlass)
	.ButtonContent()
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.Padding(0, 0, 0, 0)
		.VAlign(VAlign_Fill)
		.HAlign(HAlign_Fill)
		[
			SNew(SBox)
			.MaxDesiredHeight(49)
			[
				CreateDialogueDisplayWidget()
			]
		]
		+ SVerticalBox::Slot()
		.Padding(0, 4, 0, 0)
		.AutoHeight()
		[
			SNew(SBox)
			.MaxDesiredHeight(20)
			.Visibility(this, &SFlowGraphNode_YapFragmentWidget::Visibility_TitleText)
			[
				CreateTitleTextDisplayWidget()
			]
		]
	];
}

void SFlowGraphNode_YapFragmentWidget::Construct(const FArguments& InArgs, SFlowGraphNode_YapDialogueWidget* InOwner, uint8 InFragmentIndex)
{	
	Owner = InOwner;
	
	FragmentIndex = InFragmentIndex;

	TimeModeButtonColors =
	{
		{ EYapTimeMode::None, YapColor::Noir },
		{ EYapTimeMode::Default, YapColor::Green },
		{ EYapTimeMode::AudioTime, YapColor::LightBlue },
		{ EYapTimeMode::TextTime, YapColor::LightYellow },
		{ EYapTimeMode::ManualTime, YapColor::LightRed },
	};
	
	ChildSlot
	[
		CreateFragmentWidget()
	];
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
				.Image(FAppStyle::Get().GetBrush("Icons.ChevronDown"))
				.DesiredSizeOverride(FVector2D(16, 16))
				.ColorAndOpacity(FSlateColor::UseForeground())
			]
		]
	];
}

bool SFlowGraphNode_YapFragmentWidget::Enabled_AudioPreviewButton() const
{
	const UObject* AudioAsset = GetBit().GetAudioAsset<UObject>(EYapMaturitySetting::Mature);
	
	return IsValid(AudioAsset);
}

FReply SFlowGraphNode_YapFragmentWidget::OnClicked_AudioPreviewWidget(const TSoftObjectPtr<UObject>* Object)
{
	if (!Object)
	{
		return FReply::Handled();
	}

	if (Object->IsNull())
	{
		return FReply::Handled();
	}

	const UYapBroker* BrokerCDO = UYapProjectSettings::GetEditorBrokerDefault();

	if (IsValid(BrokerCDO))
	{
		if (BrokerCDO->ImplementsPreviewAudioAsset_Internal())
		{
			bool bResult = BrokerCDO->PreviewAudioAsset_Internal(Object->LoadSynchronous());

			if (!bResult)
			{
				FNotificationInfo NotificationInfo(LOCTEXT("AudioPreview_UnknownWarning_Title", "Cannot Play Audio Preview"));
				NotificationInfo.ExpireDuration = 5.0f;
				NotificationInfo.Image = FAppStyle::GetBrush("Icons.WarningWithColor");
				NotificationInfo.SubText = LOCTEXT("AudioPreview_UnknownWarning_Description", "Unknown error!");
				FSlateNotificationManager::Get().AddNotification(NotificationInfo);
			}
		}
		else
		{
			FNotificationInfo NotificationInfo(LOCTEXT("AudioPreview_BrokerPlayFunctionMissingWarning_Title", "Cannot Play Audio Preview"));
			NotificationInfo.ExpireDuration = 5.0f;
			NotificationInfo.Image = FAppStyle::GetBrush("Icons.WarningWithColor");
			NotificationInfo.SubText = LOCTEXT("AudioPreview_BrokerPlayFunctionMissingWarning_Description", "Your Broker Class must implement the \"PlayDialogueAudioAssetInEditor\" function.");
			FSlateNotificationManager::Get().AddNotification(NotificationInfo);
		}
	}
	else
	{
		FNotificationInfo NotificationInfo(LOCTEXT("AudioPreview_BrokerPlayFunctionMissingWarning_Title", "Cannot Play Audio Preview"));
		NotificationInfo.ExpireDuration = 5.0f;
		NotificationInfo.Image = FAppStyle::GetBrush("Icons.WarningWithColor");
		NotificationInfo.SubText = LOCTEXT("AudioPreview_BrokerMissingWarning_Description", "Yap Broker class missing - you must set a Yap Broker class in project settings.");
		FSlateNotificationManager::Get().AddNotification(NotificationInfo);
	}
	
	return FReply::Handled();
}

TSharedRef<SWidget> SFlowGraphNode_YapFragmentWidget::CreateAudioPreviewWidget(const TSoftObjectPtr<UObject>* AudioAsset, TAttribute<EVisibility> VisibilityAtt)
{
	return SNew(SBox)
	.WidthOverride(28)
	.HeightOverride(20)
	[
		SNew(SButton)
		.Cursor(EMouseCursor::Default)
		.ContentPadding(1)
		.ButtonStyle(FYapEditorStyle::Get(), YapStyles.ButtonStyle_SimpleButton)
		.Visibility(VisibilityAtt)
		.IsEnabled(this, &SFlowGraphNode_YapFragmentWidget::Enabled_AudioPreviewButton)
		.ToolTipText(LOCTEXT("PlayAudio", "Play audio"))
		.OnClicked(this, &SFlowGraphNode_YapFragmentWidget::OnClicked_AudioPreviewWidget, AudioAsset)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
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
	FYapTransactions::BeginModify(LOCTEXT("ChangeActivationLimit", "Change activation limit"), GetDialogueNode());

	GetFragment().ActivationLimit = FCString::Atoi(*Text.ToString());

	GetDialogueNode()->OnReconstructionRequested.Execute();
	
	FYapTransactions::EndModify();
}

TSharedRef<SWidget> SFlowGraphNode_YapFragmentWidget::CreateUpperFragmentBar()
{
	UFlowNode_YapDialogue* DialogueNode = GetDialogueNode();

	const FYapBit& Bit = GetBit();
	
	TSharedRef<SWidget> Box = SNew(SBox)
	.Padding(0, 0, 32, 4)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Center)
		.Padding(6, 0, 0, 0)
		.AutoWidth()
		[
			// Unused spot. Could have a button here if I really need one but it's nice to have some dead grab space here and there.
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
		]
		+ SHorizontalBox::Slot()
		.HAlign(HAlign_Right)
		.AutoWidth()
		.VAlign(VAlign_Fill)
		.Padding(4, 0, 1, 0)
		[
			SNew(SLevelOfDetailBranchNode)
			.Visibility(this, &SFlowGraphNode_YapFragmentWidget::Visibility_FragmentTagWidget)
			.UseLowDetailSlot(Owner, &SFlowGraphNode_YapDialogueWidget::UseLowDetail)
			.HighDetail()
			[
				SNew(SBox)
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				[
					CreateFragmentTagWidget()
				]
			]
		]
		+ SHorizontalBox::Slot()
		.HAlign(HAlign_Right)
		.AutoWidth()
		.Padding(6, -2, -27, -2)
		[
			SNew(SBox)
			.WidthOverride(20)
			[
				MakeProgressionPopupButton<FYapBit, &FYapBit::GetSkippable, &FYapBit::GetAutoAdvance>(&GetBit().Skippable, &GetBit().AutoAdvance, &GetBit(), GetDialogueNode())
			]
		]
	];
	
	//OnConditionsUpdated();

	return Box;
}

EVisibility SFlowGraphNode_YapFragmentWidget::Visibility_FragmentTagWidget() const
{
	return GetDialogueNode()->GetDialogueTag().IsValid() ? EVisibility::Visible : EVisibility::Collapsed;
}

void SFlowGraphNode_YapFragmentWidget::OnCheckStateChanged_SkippableToggle(ECheckBoxState CheckBoxState)
{
	/*
	FYapTransactions::BeginModify(LOCTEXT("ToggleSkippable", "Toggle skippable"), GetDialogueNode());

	if (GEditor->GetEditorSubsystem<UYapEditorSubsystem>()->GetInputTracker()->GetControlPressed())
	{
		GetFragment().GetBitMutable().Skippable = EYapDialogueProgression::Default;
	}
	else if (CheckBoxState == ECheckBoxState::Checked)
	{
		GetFragment().GetBitMutable().Skippable = EYapDialogueProgression::Skippable;
	}
	else
	{
		GetFragment().GetBitMutable().Skippable = EYapDialogueProgression::NotSkippable;
	}	
	
	FYapTransactions::EndModify();
	*/
}

ECheckBoxState SFlowGraphNode_YapFragmentWidget::IsChecked_ChildSafeSettings() const
{
	if (!NeedsChildSafeData())
	{
		return ECheckBoxState::Unchecked;
	}

	if (HasCompleteChildSafeData())
	{
		return ECheckBoxState::Checked;
	}

	return ECheckBoxState::Undetermined;
}

void SFlowGraphNode_YapFragmentWidget::OnCheckStateChanged_MaturitySettings(ECheckBoxState CheckBoxState)
{
	FYapTransactions::BeginModify(LOCTEXT("ChangeChildSafetySettings", "Change child-safety settings"), GetDialogueNode());
	
	if (NeedsChildSafeData() && bCtrlPressed)
	{
		if (!HasCompleteChildSafeData())
		{
			GetBit().bIgnoreChildSafeErrors = !GetBit().bIgnoreChildSafeErrors;
		}
		else
		{
			// Ignore ctrl-clicking on a widget that is not in an error state 
		}
	}
	else
	{
		if (!NeedsChildSafeData())
		{
			// Turn on child safety settings
			GetBit().bNeedsChildSafeData = true;
			GetBit().bIgnoreChildSafeErrors = false;
		}
		else
		{
			// Turn off child safety settings if we don't have any data assigned; otherwise flash a warning
			if (!HasAnyChildSafeData())
			{
				FYapScopedTransaction T("ChangeChildSafeSettings", LOCTEXT("TurnOffChildSafe", "Disable child-safe settings"), GetDialogueNode());

				GetBit().bNeedsChildSafeData = false;
			}
			else
			{
				EAppReturnType::Type ReturnType = FMessageDialog::Open(EAppMsgType::YesNoCancel, LOCTEXT("TurnOffChildSafeSettingsDialog_DataWarning", "Node contains child-safe data: do you want to reset it? Press 'Yes' to remove child-safe data, or 'No' to leave it hidden."), LOCTEXT("TurnOffChildSafeSettingsDialog_Title", "Turn Off Child-Safe Settings"));

				switch (ReturnType)
				{
					case EAppReturnType::Yes:
					{
						FYapScopedTransaction T("ChangeChildSafeSettings", LOCTEXT("ResetChildSafeSettings", "Reset child-safe settings"), GetDialogueNode());
						
						GetBit().SetSafeDialogueAudioAsset(nullptr);
						GetBit().SetTextData(&GetBit().SafeDialogueText, FText::GetEmpty());
						GetBit().SetTitleText(&GetBit().SafeTitleText, FText::GetEmpty());

						GetBit().bNeedsChildSafeData = false;
					}
					case EAppReturnType::No:
					{
						FYapScopedTransaction T("ChangeChildSafeSettings", LOCTEXT("TurnOffChildSafe", "Disable child-safe settings"), GetDialogueNode());
						
						GetBit().bNeedsChildSafeData = false;
					}
					default:
					{
						// Do nothing, just close the dialog
					}
				}
			}
		}
	}

	FYapTransactions::EndModify();
}

FSlateColor SFlowGraphNode_YapFragmentWidget::ColorAndOpacity_ChildSafeSettingsCheckBox() const
{
	if (!NeedsChildSafeData())
	{
		return HasAnyChildSafeData() ? YapColor::YellowGray_SemiGlass : YapColor::DarkGray;
	}

	if (!HasCompleteChildSafeData())
	{
		return GetBit().bIgnoreChildSafeErrors ? YapColor::LightGreen_SemiGlass : YapColor::OrangeRed;
	}
	
	return YapColor::LightBlue_SemiGlass;
}

FSlateColor SFlowGraphNode_YapFragmentWidget::BorderBackgroundColor_DirectedAtImage() const
{
	const FYapBit& Bit = GetBit();

	FLinearColor Color;
	
	if (Bit.GetDirectedAtAsset().IsValid())
	{
		Color = Bit.GetDirectedAtAsset().Get()->GetEntityColor();
	}
	else
	{
		Color = YapColor::Transparent;		
	}

	float A = UYapDeveloperSettings::GetPortraitBorderAlpha();
	
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
		FYapTransactions::BeginModify(LOCTEXT("SetDirectedAtCharacter", "Set directed-at character"), GetDialogueNode());

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
		FYapTransactions::BeginModify(LOCTEXT("SetDirectedAtCharacter", "Set directed-at character"), GetDialogueNode());

		GetBit().SetDirectedAt(nullptr);

		FYapTransactions::EndModify();

		return FReply::Handled();
	}

	return FReply::Unhandled();
}

TSharedRef<SWidget> SFlowGraphNode_YapFragmentWidget::PopupContentGetter_DirectedAtWidget()
{
	return SNew(SBorder)
	.Padding(1, 1, 1, 1)
	.BorderImage(FYapEditorStyle::GetImageBrush(YapBrushes.Box_SolidLightGray_Rounded))
	.BorderBackgroundColor(YapColor::DimGray)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Center)
		.Padding(6, 0, 6, 0)
		[
			SNew(SBox)
			.WidthOverride(15) // Rotated widgets are laid out per their original transform, use negative padding and a width override for rotated text
			.Padding(-80)
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Center)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("DirectedAt_PopupLabel", "DIRECTED AT"))
				.RenderTransformPivot(FVector2D(0.5, 0.5))
				.RenderTransform(FSlateRenderTransform(FQuat2D(FMath::DegreesToRadians(-90.0f))))
				.Font(YapFonts.Font_SectionHeader)
			]
		]
		
		+ SHorizontalBox::Slot()
		[
			SNew(SYapPropertyMenuAssetPicker)
			.AllowedClasses({UYapCharacter::StaticClass()})
			.AllowClear(true)
			.InitialObject(GetBit().GetDirectedAt())
			.OnSet(this, &SFlowGraphNode_YapFragmentWidget::OnSetNewDirectedAtAsset)
		]
	];
}

const FSlateBrush* SFlowGraphNode_YapFragmentWidget::Image_DirectedAtWidget() const
{
	const FSlateBrush* PortraitBrush = UYapEditorSubsystem::GetCharacterPortraitBrush(GetBit().GetDirectedAt(), FGameplayTag::EmptyTag);

	if (PortraitBrush && PortraitBrush->GetResourceObject())
	{
		return PortraitBrush;
	}
	else
	{
		return nullptr;
	}
}

TSharedRef<SWidget> SFlowGraphNode_YapFragmentWidget::CreateDirectedAtWidget()
{
	int32 PortraitSize = UYapProjectSettings::GetPortraitSize() / 3;
	
	return SNew(SBorder)
	.Cursor(EMouseCursor::Default)
	.HAlign(HAlign_Center)
	.VAlign(VAlign_Center)
	.BorderImage(FYapEditorStyle::GetImageBrush(YapBrushes.Panel_Rounded))
	.BorderBackgroundColor(this, &SFlowGraphNode_YapFragmentWidget::BorderBackgroundColor_DirectedAtImage)
	.Padding(2)
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
					SNew(SYapButtonPopup)
					.PopupPlacement(MenuPlacement_BelowAnchor)
					.OnClicked(this, &SFlowGraphNode_YapFragmentWidget::OnClicked_DirectedAtWidget)
					.PopupContentGetter(FPopupContentGetter::CreateSP(this, &SFlowGraphNode_YapFragmentWidget::PopupContentGetter_DirectedAtWidget))
					.ButtonStyle(FYapEditorStyle::Get(), YapStyles.ButtonStyle_HoverHintOnly)
					.ButtonBackgroundColor(YapColor::DarkGray)
					.ButtonContent()
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

bool SFlowGraphNode_YapFragmentWidget::OnAreAssetsAcceptableForDrop_ChildSafeButton(TArrayView<FAssetData> AssetDatas) const
{
	return OnAreAssetsAcceptableForDrop_TextWidget(AssetDatas);
}

void SFlowGraphNode_YapFragmentWidget::OnAssetsDropped_ChildSafeButton(const FDragDropEvent& DragDropEvent, TArrayView<FAssetData> AssetDatas)
{
	if (AssetDatas.Num() != 1)
	{
		return;
	}

	UObject* Object = AssetDatas[0].GetAsset();
	
	FYapTransactions::BeginModify(LOCTEXT("SetAudioAsset", "Set audio asset"), GetDialogueNode());

	GetBit().SetSafeDialogueAudioAsset(Object);

	FYapTransactions::EndModify();	
}

// ================================================================================================
// FRAGMENT WIDGET
// ------------------------------------------------------------------------------------------------

TSharedRef<SWidget> SFlowGraphNode_YapFragmentWidget::CreateFragmentWidget()
{
	int32 PortraitSize = UYapProjectSettings::GetPortraitSize();
	int32 PortraitBorder = 2;

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
							SNew(SAssetDropTarget)
							.bSupportsMultiDrop(false)
							.OnAreAssetsAcceptableForDrop(this, &SFlowGraphNode_YapFragmentWidget::OnAreAssetsAcceptableForDrop_ChildSafeButton)
							.OnAssetsDropped(this, &SFlowGraphNode_YapFragmentWidget::OnAssetsDropped_ChildSafeButton)
							[
								SNew(SCheckBox)
								.Cursor(EMouseCursor::Default)
								.Style(FYapEditorStyle::Get(), YapStyles.CheckBoxStyle_Skippable)
								.Type(ESlateCheckBoxType::ToggleButton)
								.Padding(FMargin(0, 0))
								.CheckBoxContentUsesAutoWidth(true)
								.ToolTip(nullptr) // Don't show a tooltip, it's distracting
								.IsChecked(this, &SFlowGraphNode_YapFragmentWidget::IsChecked_ChildSafeSettings)
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
							CreateMoodTagSelectorWidget()
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
					.HeightOverride(PortraitSize + 2 * PortraitBorder)
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
								CreateSpeakerWidget()
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
							CreateCentreTextDisplayWidget()
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

EVisibility SFlowGraphNode_YapFragmentWidget::Visibility_EmptyTextIndicator(const FText* Text) const
{
	return Text->IsEmpty() ? EVisibility::HitTestInvisible : EVisibility::Hidden;
}


TOptional<float> SFlowGraphNode_YapFragmentWidget::Value_TimeSetting_Default(EYapMaturitySetting MaturitySetting) const
{
	return GetBit().GetTime(MaturitySetting);
}

TOptional<float> SFlowGraphNode_YapFragmentWidget::Value_TimeSetting_AudioTime(EYapMaturitySetting MaturitySetting) const
{
	return GetBit().GetAudioTime(MaturitySetting);
}

TOptional<float> SFlowGraphNode_YapFragmentWidget::Value_TimeSetting_TextTime(EYapMaturitySetting MaturitySetting) const
{
	return GetBit().GetTextTime(MaturitySetting);
}

TOptional<float> SFlowGraphNode_YapFragmentWidget::Value_TimeSetting_ManualTime(EYapMaturitySetting MaturitySetting) const
{
	return GetBit().GetManualTime();
}

void SFlowGraphNode_YapFragmentWidget::OnValueCommitted_ManualTime(float NewValue, ETextCommit::Type CommitType)
{
	FYapTransactions::BeginModify(LOCTEXT("EnterManualTimeValue", "Enter manual time value"), GetDialogueNode());

	if (CommitType != ETextCommit::OnCleared)
	{
		GetBit().SetManualTime(NewValue);
	}

	FYapTransactions::EndModify();
}

TSharedRef<SWidget> SFlowGraphNode_YapFragmentWidget::MakeTimeSettingRow(EYapTimeMode TimeMode, EYapMaturitySetting MaturitySetting)
{
	using FLabelText =				FText;
	using FToolTipText =			FText;
	using FSlateBrushIcon =			const FSlateBrush*;
	using FValueFunction =			TOptional<float>	(SFlowGraphNode_YapFragmentWidget::*) (EYapMaturitySetting) const;
	using FValueUpdatedFunction =	void				(SFlowGraphNode_YapFragmentWidget::*) (float);
	using FValueCommittedFunction =	void				(SFlowGraphNode_YapFragmentWidget::*) (float, ETextCommit::Type);

	constexpr FSlateBrush* NoBrush = nullptr;
	constexpr FValueFunction NoValueFunc = nullptr;
	constexpr FValueUpdatedFunction NoValueUpdatedFunc = nullptr;
	constexpr FValueCommittedFunction NoValueCommittedFunc = nullptr;
	
	using TimeModeData = TTuple
	<
		FLabelText,
		FToolTipText,
		FSlateBrushIcon,
		FValueFunction,
		FValueUpdatedFunction,
		FValueCommittedFunction
	>;

	static const TMap<EYapTimeMode, TimeModeData> Data
	{
		{
			EYapTimeMode::Default,
			{
				LOCTEXT("DialogueTimeMode_Default_Label", "Use Default Time"),
				LOCTEXT("DialogueTimeMode_Default_ToolTip", "Use default time method set in project settings"),
				FYapEditorStyle::GetImageBrush(YapBrushes.Icon_ProjectSettings_TabIcon),
				NoValueFunc,
				NoValueUpdatedFunc,
				NoValueCommittedFunc,
			}
		},
		{
			EYapTimeMode::None, // This entry isn't used for anything
			{
				INVTEXT(""),
				INVTEXT(""),
				NoBrush,
				NoValueFunc,
				NoValueUpdatedFunc,
				NoValueCommittedFunc,
			},
		},
		{
			EYapTimeMode::AudioTime,
			{
				LOCTEXT("DialogueTimeMode_Audio_Label", "Use Audio Time"),
				LOCTEXT("DialogueTimeMode_Audio_ToolTip", "Use a time read from the audio asset"),
				FYapEditorStyle::GetImageBrush(YapBrushes.Icon_AudioTime),
				&SFlowGraphNode_YapFragmentWidget::Value_TimeSetting_AudioTime,
				NoValueUpdatedFunc,
				NoValueCommittedFunc,
			},
		},
		{
			EYapTimeMode::TextTime,
			{
				LOCTEXT("DialogueTimeMode_Text_Label", "Use Text Time"),
				LOCTEXT("DialogueTimeMode_Text_ToolTip", "Use a time calculated from text length"),
				FYapEditorStyle::GetImageBrush(YapBrushes.Icon_TextTime),
				&SFlowGraphNode_YapFragmentWidget::Value_TimeSetting_TextTime,
				NoValueUpdatedFunc,
				NoValueCommittedFunc,
			},
		},
		{
			EYapTimeMode::ManualTime,
			{
				LOCTEXT("DialogueTimeMode_Manual_Label", "Use Specified Time"),
				LOCTEXT("DialogueTimeMode_Manual_ToolTip", "Use a manually entered time"),
				FYapEditorStyle::GetImageBrush(YapBrushes.Icon_Timer),
				&SFlowGraphNode_YapFragmentWidget::Value_TimeSetting_ManualTime,
				&SFlowGraphNode_YapFragmentWidget::OnValueUpdated_ManualTime,
				&SFlowGraphNode_YapFragmentWidget::OnValueCommitted_ManualTime,
			},
		}
	};

	const FText& LabelText =					Data[TimeMode].Get<0>();
	const FText& ToolTipText =					Data[TimeMode].Get<1>();
	const FSlateBrush* Icon =					Data[TimeMode].Get<2>();
	FValueFunction ValueFunction =				Data[TimeMode].Get<3>();
	FValueUpdatedFunction UpdatedFunction =		Data[TimeMode].Get<4>();
	FValueCommittedFunction CommittedFunction = Data[TimeMode].Get<5>();
	
	bool bHasCommittedDelegate =	Data[TimeMode].Get<5>() != nullptr;

	auto OnClickedDelegate = FOnClicked::CreateRaw(this, &SFlowGraphNode_YapFragmentWidget::OnClicked_SetTimeModeButton, TimeMode);
	auto ButtonColorDelegate = TAttribute<FSlateColor>::CreateRaw(this, &SFlowGraphNode_YapFragmentWidget::ButtonColorAndOpacity_UseTimeMode, TimeMode, TimeModeButtonColors[TimeMode]);
	auto ForegroundColorDelegate = TAttribute<FSlateColor>::CreateRaw(this, &SFlowGraphNode_YapFragmentWidget::ForegroundColor_TimeSettingButton, TimeMode, YapColor::White);

	TSharedPtr<SHorizontalBox> RowBox = SNew(SHorizontalBox);

	// On the left pane, toss in a filler spacer to let widgets flow to the right side
	if (MaturitySetting == EYapMaturitySetting::Mature)
	{
		RowBox->AddSlot()
		.FillWidth(1.0)
		[
			SNew(SSpacer)
		];
	}

	// On the left pane, add label texts and buttons
	if (MaturitySetting == EYapMaturitySetting::Mature)
	{
		RowBox->AddSlot()
		.AutoWidth()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Center)
		.Padding(0, 0, 8, 0)
		[
			SNew(STextBlock)
			.Text(LabelText)
		];
		
		RowBox->AddSlot()
		.AutoWidth()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Center)
		.Padding(0, 0, 8, 0)
		[
			SNew(SButton)
			.Cursor(EMouseCursor::Default)
			.ButtonStyle(FYapEditorStyle::Get(), YapStyles.ButtonStyle_TimeSetting)
			.ContentPadding(FMargin(4, 3))
			.ToolTipText(ToolTipText)
			.OnClicked(OnClickedDelegate)
			.ButtonColorAndOpacity(ButtonColorDelegate)
			.ForegroundColor(ForegroundColorDelegate)
			.HAlign(HAlign_Center)
			[
				SNew(SImage)
				.DesiredSizeOverride(FVector2D(16, 16))
				.Image(Icon)
				.ColorAndOpacity(FSlateColor::UseForeground())
			]
		];
	}

	auto X = SNew(SNumericEntryBox<float>);

	// Add a numeric box if this row has a value getter -- for mature side we always still add it; for child-safe we add it if it's not the manual time box
	if (ValueFunction && (MaturitySetting == EYapMaturitySetting::Mature || TimeMode != EYapTimeMode::ManualTime))
	{
		RowBox->AddSlot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			SNew(SBox)
			.WidthOverride(60)
			[
				bHasCommittedDelegate
					?
				SNew(SNumericEntryBox<float>)
				.ToolTipText(LOCTEXT("FragmentTimeEntry_Tooltip", "Time this dialogue fragment will play for"))
				//.Justification(ETextJustify::Center) // Numeric Entry Box has a bug, when spinbox is turned on this doesn't work. So don't use it for any of the rows.
				.AllowSpin(true)
				.Delta(0.05f)
				.MaxValue(60) // TODO project setting?
				.MaxSliderValue(10) // TODO project setting?
				.MaxFractionalDigits(1) // TODO project setting?
				.OnValueChanged(this, UpdatedFunction)
				.MinValue(0)
				.Value(this, ValueFunction, MaturitySetting)
				.OnValueCommitted(this, CommittedFunction)
					:
				SNew(SNumericEntryBox<float>)
				.IsEnabled(false)
				.ToolTipText(LOCTEXT("FragmentTimeEntry_Tooltip", "Time this dialogue fragment will play for"))
				//.Justification(ETextJustify::Center) // Numeric Entry Box has a bug, when spinbox is turned on this doesn't work. So don't use it for any of the rows.
				.MaxFractionalDigits(1) // TODO project setting?
				.Value(this, ValueFunction, MaturitySetting)
			]
		];
	}
	else
	{
		RowBox->AddSlot()
		.AutoWidth()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Center)
		[
			SNew(SSpacer)
			.Size(60)
		];
	}
	
	return SNew(SBox)
	.HeightOverride(24)
	[
		RowBox.ToSharedRef()
	];
}

FSlateColor SFlowGraphNode_YapFragmentWidget::ButtonColor_TimeSettingButton() const
{
	EYapTimeMode TimeMode = GetBit().TimeMode;

	if (TimeMode == EYapTimeMode::Default)
	{
		return YapColor::DimGray;
	}
	
	return TimeModeButtonColors[GetBit().TimeMode];
}

// ================================================================================================
// DIALOGUE WIDGET
// ------------------------------------------------------------------------------------------------


TSharedRef<SWidget> SFlowGraphNode_YapFragmentWidget::CreateDialogueDisplayWidget()
{
	int32 TimeSliderSize = 8;
	//FMargin TimerSliderPadding = GetDialogueNode()->UsesTitleText() ? FMargin(0, 0, 0, -(TimeSliderSize / 2) - 2) : FMargin(0, 0, 0, -(TimeSliderSize / 2));
	FMargin TimerSliderPadding = FMargin(0, 0, 0, -(TimeSliderSize / 2) - 2);

	TSharedRef<SWidget> ASDF = SNew(SBox);

	TSharedRef<SWidget> Widget = SNew(SLevelOfDetailBranchNode)
	.UseLowDetailSlot(Owner, &SFlowGraphNode_YapDialogueWidget::UseLowDetail)
	.HighDetail()
	[
		SNew(SOverlay)
		+ SOverlay::Slot()
		[
			SNew(SAssetDropTarget)
			.bSupportsMultiDrop(false)
			.OnAreAssetsAcceptableForDrop(this, &SFlowGraphNode_YapFragmentWidget::OnAreAssetsAcceptableForDrop_TextWidget)
			.OnAssetsDropped(this, &SFlowGraphNode_YapFragmentWidget::OnAssetsDropped_TextWidget)
			[
				SNew(SBorder)
				.Cursor(EMouseCursor::Default)
				.BorderImage(FYapEditorStyle::GetImageBrush(YapBrushes.Box_SolidWhite))
				.BorderBackgroundColor(FSlateColor::UseForeground())
				.ToolTipText(this, &SFlowGraphNode_YapFragmentWidget::ToolTipText_TextDisplayWidget, LOCTEXT("DialogueText_Header", "Dialogue Text"), &GetBitConst().MatureDialogueText, &GetBitConst().SafeDialogueText)
				.Padding(0)
				[
					SNew(SOverlay)
					+ SOverlay::Slot()
					.Padding(2, 2, 2, 2)
					[
						SNew(STextBlock)
						.AutoWrapText_Lambda([] () { return UYapProjectSettings::GetWrapDialogueText(); })
						.TextStyle(FYapEditorStyle::Get(), YapStyles.TextBlockStyle_DialogueText)
						.Text(this, &SFlowGraphNode_YapFragmentWidget::Text_TextDisplayWidget, &GetBitConst().MatureDialogueText, &GetBitConst().SafeDialogueText)
						.ColorAndOpacity(this, &SFlowGraphNode_YapFragmentWidget::ColorAndOpacity_TextDisplayWidget, YapColor::LightGray, &GetBitConst().MatureDialogueText, &GetBitConst().SafeDialogueText)
					]
					+ SOverlay::Slot()
					.VAlign(VAlign_Center)
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Visibility_Lambda( [this] ()
						{
							const FText& Text = GetBit().MatureDialogueText;
							return Text.IsEmpty() ? EVisibility::HitTestInvisible : EVisibility::Hidden;;
						})
						.Justification(ETextJustify::Center)
						.TextStyle(FYapEditorStyle::Get(), YapStyles.TextBlockStyle_DialogueText)
						.Text(LOCTEXT("DialogueText_None", "Dialogue Text (None)"))
						.ColorAndOpacity(YapColor::White_Glass)
					]
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
				.ToolTipText(this, &SFlowGraphNode_YapFragmentWidget::ToolTipText_TextDisplayWidget, LOCTEXT("DialogueText_Header", "Dialogue Text"), &GetBitConst().MatureDialogueText, &GetBitConst().SafeDialogueText)
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
				.HeightOverride(TimeSliderSize)
				.WidthOverride(TimeSliderSize)
				.Padding(0)
				.ToolTipText(this, &SFlowGraphNode_YapFragmentWidget::ToolTipText_FragmentTimePadding)
				[
					SNew(SImage)
					.Image(FYapEditorStyle::GetImageBrush(YapBrushes.Icon_FilledCircle))
					.ColorAndOpacity(this, &SFlowGraphNode_YapFragmentWidget::ButtonColor_TimeSettingButton)
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
			.Text(LOCTEXT("Ellipsis", "..."))
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
		return (UYapSubsystem::GetGameMaturitySetting() == EYapMaturitySetting::Mature) ? *MatureText : *SafeText;
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

TSharedRef<SWidget> SFlowGraphNode_YapFragmentWidget::PopupContentGetter_ExpandedEditor()
{
	float Width = NeedsChildSafeData() ? 350 : 500; // TODO developer setting

	FText MatureEditorTitle = NeedsChildSafeData() ? LOCTEXT("MatureDataEditor_Title", "MATURE DIALOGUE") : LOCTEXT("DialogueDataEditor_Title", "DIALOGUE");

	TSharedRef<SVerticalBox> Boxes = SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 0, 0, 0)
		[
			BuildDialogueEditors_ExpandedEditor(Width)
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 4, 0, 4)
		[
			SNew(SSeparator)
			.Thickness(2)
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 0, 0, 0)
		[
			BuildTimeSettings_ExpandedEditor(Width)
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 4, 0, 4)
		[
			SNew(SSeparator)
			.Thickness(2)
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 0, 0, 0)
		[
			BuildPaddingSettings_ExpandedEditor(Width)
		];

	return Boxes;
}

TSharedRef<SWidget> SFlowGraphNode_YapFragmentWidget::BuildDialogueEditors_ExpandedEditor(float Width)
{
	bool bMature = !NeedsChildSafeData();

	FText MatureEditorTitle = (bMature) ? LOCTEXT("DialogueDataEditor_Title", "DIALOGUE") : LOCTEXT("MatureDataEditor_Title", "MATURE DIALOGUE");

	FYapBit& Bit = GetBit();

	FText& DialogueText = Bit.MatureDialogueText;
	FText& TitleText = Bit.MatureTitleText;
	
	TSharedRef<IEditableTextProperty> DialogueTextProperty = MakeShareable(new FYapEditableTextPropertyHandle(DialogueText));
	TSharedRef<IEditableTextProperty> TitleTextProperty = MakeShareable(new FYapEditableTextPropertyHandle(TitleText));

	TSoftObjectPtr<UObject>& AudioAsset = Bit.MatureAudioAsset;
	
	float MinDesiredHeight = 66; // fits ~4 lines of text
	
	TSharedRef<SHorizontalBox> Box = SNew(SHorizontalBox)
	+ SHorizontalBox::Slot()
	[
		SNew(SBox)
		.WidthOverride(Width)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(MatureEditorTitle)
				.Font(YapFonts.Font_SectionHeader)
				.Justification(ETextJustify::Center)	
			]
			+ SVerticalBox::Slot()
			.FillHeight(1.0)
			.Padding(0, 6, 0, 0)
			[
				SNew(SBox)
				.MinDesiredHeight(MinDesiredHeight) // Fits ~4 lines of text
				[
					SNew(SYapTextPropertyEditableTextBox, DialogueTextProperty)
					.Style(FYapEditorStyle::Get(), YapStyles.EditableTextBoxStyle_Dialogue)
					.Text(this, &SFlowGraphNode_YapFragmentWidget::Text_EditedText, &DialogueText)
					.OnTextCommitted(this, &SFlowGraphNode_YapFragmentWidget::OnTextCommitted_EditedText, &FYapBit::SetTextData, &DialogueText)
					.ForegroundColor(YapColor::White)
					.Cursor(EMouseCursor::Default)
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 8, 0, 0)
			[
				SNew(SBox)
				.Visibility(GetDialogueNode()->UsesTitleText() ? EVisibility::Visible : EVisibility::Collapsed)
				[
					SNew(SYapTextPropertyEditableTextBox, TitleTextProperty)
					.Style(FYapEditorStyle::Get(), YapStyles.EditableTextBoxStyle_TitleText)
					.Text(this, &SFlowGraphNode_YapFragmentWidget::Text_EditedText, &TitleText)
					.OnTextCommitted(this, &SFlowGraphNode_YapFragmentWidget::OnTextCommitted_EditedText, &FYapBit::SetTextData, &TitleText)
					.ForegroundColor(YapColor::YellowGray)
					.Cursor(EMouseCursor::Default)
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 8, 0, 0)
			[
				SNew(SBox)
				.Visibility(EVisibility::Visible) // TODO project settings to disable audio
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.HAlign(HAlign_Fill)
					[
						CreateAudioAssetWidget(AudioAsset)
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					[
						CreateAudioPreviewWidget(&AudioAsset, EVisibility::Visible)
					]
				]
			]
		]
	];

	if (!bMature)
	{
		Box->AddSlot()
		.AutoWidth()
		.Padding(8, 0, 8, 0)
		[
			SNew(SSeparator)
			.Orientation(EOrientation::Orient_Vertical)
			.Thickness(2)
		];
	
		FText& SafeDialogueText = Bit.SafeDialogueText;
		FText& SafeTitleText = Bit.SafeTitleText;
		
		TSharedRef<IEditableTextProperty> SafeDialogueTextProperty = MakeShareable(new FYapEditableTextPropertyHandle(SafeDialogueText));
		TSharedRef<IEditableTextProperty> SafeTitleTextProperty = MakeShareable(new FYapEditableTextPropertyHandle(SafeTitleText));

		TSoftObjectPtr<UObject>& SafeAudioAsset = Bit.SafeAudioAsset;

		Box->AddSlot()
		[
			SNew(SBox)
			.WidthOverride(Width)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(STextBlock)
					.Text(LOCTEXT("SafeDataEditor_Title", "CHILD-SAFE"))
					.Font(YapFonts.Font_SectionHeader)
					.Justification(ETextJustify::Center)	
				]
				+ SVerticalBox::Slot()
				.FillHeight(1.0)
				.Padding(0, 6, 0, 0)
				[
					SNew(SBox)
					.HeightOverride(66) // Fits ~4 lines of text
					[
						SNew(SYapTextPropertyEditableTextBox, SafeDialogueTextProperty)
						.Style(FYapEditorStyle::Get(), YapStyles.EditableTextBoxStyle_Dialogue)
						.Text(this, &SFlowGraphNode_YapFragmentWidget::Text_EditedText, &SafeDialogueText)
						.OnTextCommitted(this, &SFlowGraphNode_YapFragmentWidget::OnTextCommitted_EditedText, &FYapBit::SetTextData, &SafeDialogueText)
						.ForegroundColor(YapColor::White)
						.Cursor(EMouseCursor::Default)
					]
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0, 8, 0, 0)
				[
					SNew(SBox)
					.Visibility(GetDialogueNode()->UsesTitleText() ? EVisibility::Visible : EVisibility::Collapsed)
					[
						SNew(SYapTextPropertyEditableTextBox, SafeTitleTextProperty)
						.Style(FYapEditorStyle::Get(), YapStyles.EditableTextBoxStyle_TitleText)
						.Text(this, &SFlowGraphNode_YapFragmentWidget::Text_EditedText, &SafeTitleText)
						.OnTextCommitted(this, &SFlowGraphNode_YapFragmentWidget::OnTextCommitted_EditedText, &FYapBit::SetTextData, &SafeTitleText)
						.ForegroundColor(YapColor::YellowGray)
						.Cursor(EMouseCursor::Default)
					]
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0, 8, 0, 0)
				[
					SNew(SBox)
					.Visibility(EVisibility::Visible) // TODO project settings to disable audio
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.HAlign(HAlign_Fill)
						[
							CreateAudioAssetWidget(SafeAudioAsset)
						]
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(VAlign_Center)
						[
							CreateAudioPreviewWidget(&SafeAudioAsset, EVisibility::Visible)
						]
					]
				]
			]
		];
	};

	return Box;
}

TSharedRef<SWidget> SFlowGraphNode_YapFragmentWidget::BuildTimeSettings_ExpandedEditor(float Width)
{
	bool bMature = !NeedsChildSafeData();

	TSharedRef<SHorizontalBox> Box = SNew(SHorizontalBox)
	+ SHorizontalBox::Slot()
	[
		SNew(SBox)
		.WidthOverride(Width)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.Padding(0, 2, 0, 2)
			.AutoHeight()
			[
				MakeTimeSettingRow(EYapTimeMode::Default, EYapMaturitySetting::Mature)
			]
			+ SVerticalBox::Slot()
			.Padding(0, 2, 0, 2)
			.AutoHeight()
			[
				MakeTimeSettingRow(EYapTimeMode::AudioTime, EYapMaturitySetting::Mature)
			]
			+ SVerticalBox::Slot()
			.Padding(0, 2, 0, 2)
			.AutoHeight()
			[
				MakeTimeSettingRow(EYapTimeMode::TextTime, EYapMaturitySetting::Mature)
			]
			+ SVerticalBox::Slot()
			.Padding(0, 2, 0, 2)
			.AutoHeight()
			[
				MakeTimeSettingRow(EYapTimeMode::ManualTime, EYapMaturitySetting::Mature)
			]
		]
	];

	if (!bMature)
	{
		Box->AddSlot()
		.AutoWidth()
		.Padding(8, 0, 8, 0)
		[
			SNew(SSeparator)
			.Orientation(EOrientation::Orient_Vertical)
			.Thickness(2)
		];
		
		Box->AddSlot()
		[
			SNew(SBox)
			.WidthOverride(Width)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.Padding(0, 2, 0, 2)
				.AutoHeight()
				[
					MakeTimeSettingRow(EYapTimeMode::Default, EYapMaturitySetting::ChildSafe)
				]
				+ SVerticalBox::Slot()
				.Padding(0, 2, 0, 2)
				.AutoHeight()
				[
					MakeTimeSettingRow(EYapTimeMode::AudioTime, EYapMaturitySetting::ChildSafe)
				]
				+ SVerticalBox::Slot()
				.Padding(0, 2, 0, 2)
				.AutoHeight()
				[
					MakeTimeSettingRow(EYapTimeMode::TextTime, EYapMaturitySetting::ChildSafe)
				]
				+ SVerticalBox::Slot()
				.Padding(0, 2, 0, 2)
				.AutoHeight()
				[
					MakeTimeSettingRow(EYapTimeMode::ManualTime, EYapMaturitySetting::ChildSafe)
				]
			]
		];
	};

	return Box;
}

TSharedRef<SWidget> SFlowGraphNode_YapFragmentWidget::BuildPaddingSettings_ExpandedEditor(float Width)
{
	bool bMature = !NeedsChildSafeData();

	TSharedRef<SHorizontalBox> Box = SNew(SHorizontalBox)
	+ SHorizontalBox::Slot()
	.HAlign(HAlign_Fill)
	[
		SNew(SBox)
		.WidthOverride(Width)
		.HAlign(HAlign_Right)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(0, 0, 8, 0)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("PaddingTime_Header", "Padding Time"))
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(0, 0, 8, 0)
			[				
				SNew(SButton)
				.Cursor(EMouseCursor::Default)
				.ButtonStyle(FYapEditorStyle::Get(), YapStyles.ButtonStyle_TimeSetting)
				.ContentPadding(FMargin(4, 3))
				.ToolTipText(LOCTEXT("UseDefault_Button", "Use Default"))
				.OnClicked_Lambda( [this] ()
				{
					if (GetFragment().PaddingToNextFragment < 0)
					{
						float CurrentPadding_Default = GetFragment().GetPaddingToNextFragment();
						GetFragmentMutable().SetPaddingToNextFragment(CurrentPadding_Default);
					}
					else
					{
						GetFragmentMutable().SetPaddingToNextFragment(-1);
					}
					return FReply::Handled();
				})// TODO transactions
				.ButtonColorAndOpacity(this, &SFlowGraphNode_YapFragmentWidget::ButtonColorAndOpacity_PaddingButton) // TODO coloring
				.HAlign(HAlign_Center)
				[
					SNew(SImage)
					.DesiredSizeOverride(FVector2D(16, 16))
					.Image(FYapEditorStyle::GetImageBrush(YapBrushes.Icon_ProjectSettings_TabIcon))
					.ColorAndOpacity(FSlateColor::UseForeground())
				]
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			//.Padding(0, 0, 8, 0)
			[
				SNew(SBox)
				.WidthOverride(60)
				[
					// -----------------------------
					// TIME DISPLAY
					// -----------------------------
					SNew(SNumericEntryBox<float>)
					.IsEnabled(true)
					.AllowSpin(true)
					.Delta(0.01f)
					.MaxSliderValue(UYapProjectSettings::GetFragmentPaddingSliderMax())
					.MinValue(0)
					.ToolTipText(LOCTEXT("FragmentTimeEntry_Tooltip", "Time this dialogue fragment will play for"))
					.Justification(ETextJustify::Center)
					.Value_Lambda( [this] () { return GetFragment().GetPaddingToNextFragment(); } )
					.OnValueChanged_Lambda( [this] (float NewValue) { GetFragmentMutable().SetPaddingToNextFragment(NewValue); } )
					.OnValueCommitted_Lambda( [this] (float NewValue, ETextCommit::Type) { GetFragmentMutable().SetPaddingToNextFragment(NewValue); } ) // TODO transactions
				]
			]
		]
	];

	if (!bMature)
	{
		Box->AddSlot()
		.AutoWidth()
		.Padding(8, 0, 8, 0)
		[
			SNew(SSeparator)
			.Orientation(EOrientation::Orient_Vertical)
			.Thickness(2)
		];

		Box->AddSlot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Center)
		[
			SNew(SBox)
			.WidthOverride(Width)
			//SNew(SSpacer) // Wtf, this is becoming a square, why?
			//.Size(Width)
		];
	}

	return Box;
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
		return FText::GetEmpty();
	}
	else
	{
		return Text;
	}
}

// ================================================================================================
// FRAGMENT TIME PADDING WIDGET
// ------------------------------------------------------------------------------------------------

TOptional<float> SFlowGraphNode_YapFragmentWidget::FragmentTimePadding_Percent() const
{
	const float MaxPaddedSetting = UYapProjectSettings::GetFragmentPaddingSliderMax();
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
		else if (GetDialogueNode()->GetFinishedFragments().Contains(&GetFragment()))
		{
			return 0.0;
		}

		return FragmentPadding / MaxPaddedSetting;
	}

	return FragmentPadding / MaxPaddedSetting;		
}

TOptional<float> SFlowGraphNode_YapFragmentWidget::FragmentTime_Percent() const
{
	const float MaxTimeSetting = UYapProjectSettings::GetDialogueTimeSliderMax();

	const TOptional<float> FragmentTimeIn = GetBit().GetTime(EYapMaturitySetting::Mature);

	if (!FragmentTimeIn.IsSet())
	{
		return 0;
	}

	const float FragmentTime = FragmentTimeIn.GetValue();
	
	if (GEditor->PlayWorld)
	{
		const FYapFragment* RunningFragment = GetDialogueNode()->GetRunningFragment();
		
		if (RunningFragment == &GetFragment())
		{
			if (GetDialogueNode()->FragmentStartedTime > GetDialogueNode()->FragmentEndedTime)
			{
				double ElapsedPaddingTime = GEditor->PlayWorld->GetTimeSeconds() - GetDialogueNode()->FragmentStartedTime;
				return (1 - (ElapsedPaddingTime / FragmentTime)) * (FragmentTime / MaxTimeSetting);
			}
			else
			{
				return 0.0;
			}
		}
		else if (GetDialogueNode()->GetFinishedFragments().Contains(&GetFragment()))
		{
			return 0.0;
		}

		return FragmentTime / MaxTimeSetting;
	}

	return FragmentTime / MaxTimeSetting;		
}

float SFlowGraphNode_YapFragmentWidget::Value_FragmentTimePadding() const
{
	const float MaxPaddedSetting =  UYapProjectSettings::GetFragmentPaddingSliderMax();

	return GetFragment().GetPaddingToNextFragment() / MaxPaddedSetting;
}

void SFlowGraphNode_YapFragmentWidget::OnValueChanged_FragmentTimePadding(float X)
{
	const float MaxPaddedSetting =  UYapProjectSettings::GetFragmentPaddingSliderMax();
	float NewValue = X * MaxPaddedSetting;

	// We will attempt to snap to the default time unless you hold ctrl
	if (!bCtrlPressed)
	{
		float DefaultFragmentPaddingTime = UYapProjectSettings::GetDefaultFragmentPaddingTime();
		
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
	if (GEditor->PlayWorld)
	{
		const FYapFragment* RunningFragment = GetDialogueNode()->GetRunningFragment();

		if (RunningFragment == &GetFragment())
		{
			return YapColor::White_Trans;
		}

		return YapColor::DarkGray_Trans;
	}
		
	return YapColor::DimGray_Trans;
}

FText SFlowGraphNode_YapFragmentWidget::ToolTipText_FragmentTimePadding() const
{
	FNumberFormattingOptions Formatting;
	Formatting.MaximumFractionalDigits = 3;
	
	return FText::Format(LOCTEXT("Fragment", "Post-delay: {0}"), FText::AsNumber(GetFragment().GetPaddingToNextFragment(), &Formatting));
}

FLinearColor SFlowGraphNode_YapFragmentWidget::BorderBackgroundColor_CharacterImage() const
{
	const FYapBit& Bit = GetBit();

	FLinearColor Color;
	
	if (Bit.GetSpeakerAsset().IsValid())
	{
		Color = Bit.GetSpeakerAsset().Get()->GetEntityColor();
	}
	else
	{
		Color = YapColor::Gray_Glass;		
	}

	Color.A *= UYapDeveloperSettings::GetPortraitBorderAlpha();

	if (!GetDialogueNode()->IsPlayerPrompt())
	{
		Color.A *= 0.75f;
	}

	return Color;
}

void SFlowGraphNode_YapFragmentWidget::OnSetNewSpeakerAsset(const FAssetData& AssetData)
{
	FYapTransactions::BeginModify(LOCTEXT("SetSpeakerCharacter", "Set speaker character"), GetDialogueNode());

	GetBit().SetSpeaker(AssetData.GetAsset());

	FYapTransactions::EndModify();
}

void SFlowGraphNode_YapFragmentWidget::OnSetNewDirectedAtAsset(const FAssetData& AssetData)
{
	FYapTransactions::BeginModify(LOCTEXT("SetDirectedAtCharacter", "Set directed-at character"), GetDialogueNode());

	GetBit().SetDirectedAt(AssetData.GetAsset());

	FYapTransactions::EndModify();
}

TSharedRef<SWidget> SFlowGraphNode_YapFragmentWidget::PopupContentGetter_SpeakerWidget(TSoftObjectPtr<UYapCharacter>* CharacterAsset, const UYapCharacter* Character)
{
	return SNew(SBorder)
	.Padding(1, 1, 1, 1)
	.BorderImage(FYapEditorStyle::GetImageBrush(YapBrushes.Box_SolidLightGray_Rounded))
	.BorderBackgroundColor(YapColor::DimGray)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Center)
		.Padding(6, 0, 6, 0)
		[
			SNew(SBox)
			.WidthOverride(15) // Rotated widgets are laid out per their original transform, use negative padding and a width override for rotated text
			.Padding(-80) 
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Center)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("Speaker_PopupLabel", "SPEAKER"))
				.RenderTransformPivot(FVector2D(0.5, 0.5))
				.RenderTransform(FSlateRenderTransform(FQuat2D(FMath::DegreesToRadians(-90.0f))))
				.Font(YapFonts.Font_SectionHeader)
			]
		]
		+ SHorizontalBox::Slot()
		[
			SNew(SYapPropertyMenuAssetPicker)
			.AllowedClasses({UYapCharacter::StaticClass()})
			.AllowClear(true)
			.InitialObject(Character)
			.OnSet(this, &SFlowGraphNode_YapFragmentWidget::OnSetNewSpeakerAsset)
		]
	];
}

FText SFlowGraphNode_YapFragmentWidget::Text_SpeakerWidget() const
{
	const FYapBit& Bit = GetBit();
	
	if (Bit.GetSpeakerAsset().IsNull())
	{
		return LOCTEXT("SpeakerUnset_Label","Speaker\nUnset");
	}
	
	if (Image_SpeakerImage() == nullptr)
	{
		TSharedPtr<FGameplayTagNode> GTN = UGameplayTagsManager::Get().FindTagNode(Bit.GetMoodTag());
		
		FText CharacterName = Bit.GetSpeakerAsset().IsValid() ? Bit.GetSpeakerAsset().Get()->GetEntityName() : LOCTEXT("Unloaded", "Unloaded");
		
		if (CharacterName.IsEmpty())
		{
			CharacterName = LOCTEXT("Unnamed", "Unnamed");
		}

		FText MoodTagLabel;
		
		if (GTN.IsValid())
		{
			MoodTagLabel = FText::FromName(GTN->GetSimpleTagName());
		}
		else
		{
			TSharedPtr<FGameplayTagNode> DefaultGTN = UGameplayTagsManager::Get().FindTagNode(UYapProjectSettings::GetDefaultMoodTag());

			FText MoodTagNameAsText = DefaultGTN.IsValid() ? FText::FromName(DefaultGTN->GetSimpleTagName()) : LOCTEXT("MoodTag_None_Label", "None");
			
			MoodTagLabel = FText::Format(LOCTEXT("DefaultMoodTag_Label", "{0}(D)"), MoodTagNameAsText);
		}
		
		return FText::Format(LOCTEXT("SpeakerMoodImageMissing_Label", "{0}\n\n{1}\n<missing>"), CharacterName, MoodTagLabel);
	}
	
	return FText::GetEmpty();
}

FText SFlowGraphNode_YapFragmentWidget::ToolTipText_SpeakerWidget() const
{
	const FYapBit& Bit = GetBit();
	
	if (Bit.GetSpeakerAsset().IsNull())
	{
		return LOCTEXT("SpeakerUnset_Label","Speaker Unset");
	}
	
	TSharedPtr<FGameplayTagNode> GTN = UGameplayTagsManager::Get().FindTagNode(Bit.GetMoodTag());
	
	FText CharacterName = Bit.GetSpeakerAsset().IsValid() ? Bit.GetSpeakerAsset().Get()->GetEntityName() : LOCTEXT("Unloaded", "Unloaded");
	
	if (CharacterName.IsEmpty())
	{
		CharacterName = LOCTEXT("Unnamed", "Unnamed");
	}

	FText MoodTagLabel;
	
	if (GTN.IsValid())
	{
		MoodTagLabel = FText::FromName(GTN->GetSimpleTagName());
	}
	else
	{
		TSharedPtr<FGameplayTagNode> DefaultGTN = UGameplayTagsManager::Get().FindTagNode(UYapProjectSettings::GetDefaultMoodTag());

		FText MoodTagNameAsText = DefaultGTN.IsValid() ? FText::FromName(DefaultGTN->GetSimpleTagName()) : LOCTEXT("MoodTag_None_Label", "None");
		
		MoodTagLabel = FText::Format(LOCTEXT("DefaultMoodTag_Label", "{0}(D)"), MoodTagNameAsText);
	}
	
	return FText::Format(LOCTEXT("SpeakerMoodImageMissing_Label", "{0}\n\n{1}\n<missing>"), CharacterName, MoodTagLabel);
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
	
	return false;
}

void SFlowGraphNode_YapFragmentWidget::OnAssetsDropped_SpeakerWidget(const FDragDropEvent& DragDropEvent, TArrayView<FAssetData> AssetDatas)
{
	if (AssetDatas.Num() != 1)
	{
		return;
	}

	UYapCharacter* Character = Cast<UYapCharacter>(AssetDatas[0].GetAsset());
	
	FYapTransactions::BeginModify(LOCTEXT("SetSpeakerCharacter", "Set speaker character"), GetDialogueNode());

	GetBit().SetSpeaker(Character);

	FYapTransactions::EndModify();
}

bool SFlowGraphNode_YapFragmentWidget::OnAreAssetsAcceptableForDrop_TextWidget(TArrayView<FAssetData> AssetDatas) const
{
	if (AssetDatas.Num() != 1)
	{
		return false;
	}

	UClass* AssetClass = AssetDatas[0].GetClass();
	
	const TArray<TSoftClassPtr<UObject>>& AllowableClasses = UYapProjectSettings::GetAudioAssetClasses();
	
	for (const TSoftClassPtr<UObject>& AllowableClass : AllowableClasses)
	{
		if (AssetClass->IsChildOf(AllowableClass.LoadSynchronous())) // TODO return false instead and request an async load
		{
			return true;
		}
	}  

	return false;
}

void SFlowGraphNode_YapFragmentWidget::OnAssetsDropped_TextWidget(const FDragDropEvent& DragDropEvent, TArrayView<FAssetData> AssetDatas)
{
	if (AssetDatas.Num() != 1)
	{
		return;
	}

	UObject* Object = AssetDatas[0].GetAsset();
	
	FYapTransactions::BeginModify(LOCTEXT("SetAudioAsset", "Set audio asset"), GetDialogueNode());

	GetBit().SetMatureDialogueAudioAsset(Object);

	FYapTransactions::EndModify();	
}

// ================================================================================================
// PORTRAIT WIDGET
// ------------------------------------------------------------------------------------------------

TSharedRef<SOverlay> SFlowGraphNode_YapFragmentWidget::CreateSpeakerWidget()
{
	int32 PortraitSize = UYapProjectSettings::GetPortraitSize();
	int32 PortraitBorder = 2;

	int32 MinHeight = 72; // Hardcoded minimum height - this keeps pins aligned with the graph snapping grid

	int32 FinalHeight = FMath::Max(PortraitSize + 2 * PortraitBorder, MinHeight);
	
	return SNew(SOverlay)
	+ SOverlay::Slot()
	.Padding(0, 0, 0, 0)
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
				SNew(SYapButtonPopup)
				.ButtonForegroundColor(this, &SFlowGraphNode_YapFragmentWidget::BorderBackgroundColor_CharacterImage)
				.PopupPlacement(MenuPlacement_BelowAnchor)
				.PopupContentGetter(FPopupContentGetter::CreateSP(this, &SFlowGraphNode_YapFragmentWidget::PopupContentGetter_SpeakerWidget, &GetBit().SpeakerAsset, GetBit().GetSpeaker(EYapWarnings::Ignore)))
				.ButtonStyle(FYapEditorStyle::Get(), YapStyles.ButtonStyle_SpeakerPopup)
				.ButtonContent()
				[
					SNew(SBox)
					.WidthOverride(FinalHeight)
					.HeightOverride(FinalHeight)
					[
						SNew(SOverlay)
						+ SOverlay::Slot()
						.HAlign(HAlign_Fill)
						.VAlign(VAlign_Fill)
						[
							SNew(SBorder)
							.HAlign(HAlign_Center)
							.VAlign(VAlign_Center)
							.BorderImage(FYapEditorStyle::GetImageBrush(YapBrushes.Border_Thick_RoundedSquare))
							.BorderBackgroundColor(FSlateColor::UseForeground())
						]
						+ SOverlay::Slot()
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Center)
						.Padding(PortraitBorder)
						[
							SNew(SImage)
							.DesiredSizeOverride(FVector2D(PortraitSize, PortraitSize))
							.Image(this, &SFlowGraphNode_YapFragmentWidget::Image_SpeakerImage)
							.ToolTipText(this, &SFlowGraphNode_YapFragmentWidget::ToolTipText_SpeakerWidget)
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
		]
		.LowDetail()
		[
			SNew(SImage)
			.DesiredSizeOverride(FVector2D(PortraitSize, PortraitSize))
			.Image(this, &SFlowGraphNode_YapFragmentWidget::Image_SpeakerImage)
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
	const FSlateBrush* PortraitBrush = UYapEditorSubsystem::GetCharacterPortraitBrush(GetBit().GetSpeaker(), GetBit().GetMoodTag());

	if (PortraitBrush && PortraitBrush->GetResourceObject())
	{
		return PortraitBrush;
	}
	else
	{
		return nullptr;
	}
}

EVisibility SFlowGraphNode_YapFragmentWidget::Visibility_MissingPortraitWarning() const
{
	const FSlateBrush* Brush = UYapEditorSubsystem::GetCharacterPortraitBrush(GetBit().GetSpeaker(), GetBit().GetMoodTag());
	
	return (Brush->GetResourceObject()) ? EVisibility::Hidden : EVisibility::Visible;
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

FText SFlowGraphNode_YapFragmentWidget::ToolTipText_MoodTagSelector() const
{
	TSharedPtr<FGameplayTagNode> TagNode = UGameplayTagsManager::Get().FindTagNode(GetBit().GetMoodTag());

	if (TagNode.IsValid())
	{
		return FText::FromName(TagNode->GetSimpleTagName());
	}

	return LOCTEXT("Default", "Default");
}

FSlateColor SFlowGraphNode_YapFragmentWidget::ForegroundColor_MoodTagSelectorWidget() const
{
	if (GetBit().GetMoodTag() == FGameplayTag::EmptyTag)
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
	return SNew(SBorder)
	.Cursor(EMouseCursor::Default)
	.BorderImage(FYapEditorStyle::GetImageBrush(YapBrushes.Box_SolidWhite))
	.BorderBackgroundColor(FSlateColor::UseForeground())
	.ToolTipText(LOCTEXT("TitleText_ToolTip", "Title text"))
	[
		SNew(SOverlay)
		+ SOverlay::Slot()
		[
			SNew(STextBlock)
			.TextStyle(FYapEditorStyle::Get(), YapStyles.TextBlockStyle_TitleText)
			.Text_Lambda( [this] () { return GetBit().GetTitleText(EYapMaturitySetting::Mature); } )
			.ToolTipText(this, &SFlowGraphNode_YapFragmentWidget::ToolTipText_TextDisplayWidget, LOCTEXT("TitleText_ToolTip", "Title text"), &GetBitConst().MatureTitleText, &GetBitConst().SafeTitleText)
			.ColorAndOpacity(this, &SFlowGraphNode_YapFragmentWidget::ColorAndOpacity_TextDisplayWidget, YapColor::YellowGray, &GetBitConst().MatureTitleText, &GetBitConst().SafeTitleText)
		]
		+ SOverlay::Slot()
		.VAlign(VAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Visibility_Lambda( [this] ()
			{
				const FText& Text = GetBit().MatureTitleText;
				return Text.IsEmpty() ? EVisibility::HitTestInvisible : EVisibility::Hidden;;
			})
			.Justification(ETextJustify::Center)
			.TextStyle(FYapEditorStyle::Get(), YapStyles.TextBlockStyle_TitleText)
			.Text(LOCTEXT("TitleText_None", "Title text (none)"))
			.ColorAndOpacity(YapColor::White_Glass)
		]
	];
}
 
FText SFlowGraphNode_YapFragmentWidget::Text_EditedText(FText* Text) const
{
	return *Text;
}

void SFlowGraphNode_YapFragmentWidget::OnTextCommitted_EditedText(const FText& NewValue, ETextCommit::Type CommitType, void (FYapBit::*Func)(FText* TextToSet, const FText& NewValue), FText* TextToSet)
{
	FYapTransactions::BeginModify(LOCTEXT("ChangeDialogueText", "Text changed"), GetDialogueNode());

	if (CommitType != ETextCommit::OnCleared)
	{
		(GetBit().*Func)(TextToSet, NewValue);
	}

	FYapTransactions::EndModify();
}

FText SFlowGraphNode_YapFragmentWidget::ToolTipText_TextDisplayWidget(FText Label, const FText* MatureText, const FText* SafeText) const
{
	// TODO Label is not currently used, remove it eventually or put it back
	
	const FText Unset = LOCTEXT("NoTextWarning_ToolTip", "\u26A0 No text \u26A0");// 26A0 Warning sign
	
	if (NeedsChildSafeData())
	{
		return FText::Format(LOCTEXT("DialogueText_ToolTip_Both", "\u2668{1}\n\n\u26F9{2}"), Label, MatureText->IsEmpty() ? Unset : *MatureText, SafeText->IsEmpty() ? Unset : *SafeText);
	}
	else
	{
		return FText::Format(LOCTEXT("DialogueText_ToolTip_MatureOnly", "\u2756{1}"), Label, MatureText->IsEmpty() ? Unset : *MatureText);
	}
}

FSlateColor SFlowGraphNode_YapFragmentWidget::ColorAndOpacity_TextDisplayWidget(FLinearColor BaseColor, const FText* MatureText, const FText* SafeText) const
{
	FLinearColor Color = BaseColor;
	
	if (GEditor->PlayWorld)
	{
		if (UYapSubsystem::GetGameMaturitySetting() == EYapMaturitySetting::ChildSafe && !SafeText->IsEmpty())
		{
			Color *= YapColor::LightBlue;
		}
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
	.ToolTipText(LOCTEXT("FragmentTag_ToolTip", "Fragment tag"));
}

FGameplayTag SFlowGraphNode_YapFragmentWidget::Value_FragmentTag() const
{
	return GetFragment().FragmentTag;
}

void SFlowGraphNode_YapFragmentWidget::OnTagChanged_FragmentTag(FGameplayTag GameplayTag)
{
	FYapTransactions::BeginModify(LOCTEXT("ChangeFragmentTag", "Change fragment tag"), GetDialogueNode());

	GetFragment().FragmentTag = GameplayTag;

	FYapTransactions::EndModify();

	Owner->RequestUpdateGraphNode();
}

FReply SFlowGraphNode_YapFragmentWidget::OnClicked_SetTimeModeButton(EYapTimeMode TimeMode)
{
	FYapTransactions::BeginModify(LOCTEXT("TimeModeChanged", "Time mode changed"), GetDialogueNode());

	if (GetBit().TimeMode == TimeMode)
	{
		GetBit().SetBitTimeMode(EYapTimeMode::None);
	}
	else
	{
		GetBit().SetBitTimeMode(TimeMode);
	}
	
	FYapTransactions::EndModify();

	return FReply::Handled();
}

void SFlowGraphNode_YapFragmentWidget::OnValueUpdated_ManualTime(float NewValue)
{
	GetBit().SetManualTime(NewValue);
	GetBit().SetBitTimeMode(EYapTimeMode::ManualTime);
}

// ---------------------


FSlateColor SFlowGraphNode_YapFragmentWidget::ButtonColorAndOpacity_UseTimeMode(EYapTimeMode TimeMode, FLinearColor ColorTint) const
{
	if (GetBit().TimeMode == TimeMode)
	{
		// Exact setting match
		return ColorTint;
	}
	
	if (GetBit().GetTimeMode(EYapMaturitySetting::Mature) == TimeMode)
	{
		// Implicit match through project defaults
		return ColorTint.Desaturate(0.50);
	}
	
	return YapColor::DarkGray;
}

FSlateColor SFlowGraphNode_YapFragmentWidget::ButtonColorAndOpacity_PaddingButton() const
{
	if (GetFragment().PaddingToNextFragment < 0)
	{
		return YapColor::LightGreen;
	}
	
	return YapColor::DarkGray;
}

FSlateColor SFlowGraphNode_YapFragmentWidget::ForegroundColor_TimeSettingButton(EYapTimeMode TimeMode, FLinearColor ColorTint) const
{
	if (GetBit().TimeMode == TimeMode)
	{
		// Exact setting match
		return ColorTint;
	}
	
	if (GetBit().GetTimeMode(EYapMaturitySetting::Mature) == TimeMode)
	{
		// Implicit match through project defaults
		return ColorTint;
	}
	
	return YapColor::Gray;
}

bool SFlowGraphNode_YapFragmentWidget::OnShouldFilterAsset_AudioAssetWidget(const FAssetData& AssetData) const
{
	const TArray<TSoftClassPtr<UObject>>& Classes = UYapProjectSettings::GetAudioAssetClasses();

	// TODO async loading
	if (Classes.ContainsByPredicate( [&AssetData] (const TSoftClassPtr<UObject>& Class) { return AssetData.GetClass(EResolveClass::Yes) == Class; } ))
	{
		return true;
	}

	return false;	
}

// ================================================================================================
// AUDIO ASSET WIDGET
// ------------------------------------------------------------------------------------------------

TSharedRef<SWidget> SFlowGraphNode_YapFragmentWidget::CreateAudioAssetWidget(const TSoftObjectPtr<UObject>& Asset)
{
	EYapMaturitySetting Type = (&Asset == &GetBit().SafeAudioAsset) ? EYapMaturitySetting::ChildSafe : EYapMaturitySetting::Mature; 
	
	UClass* DialogueAssetClass = nullptr;
	
	const TArray<TSoftClassPtr<UObject>>& DialogueAssetClassPtrs = UYapProjectSettings::GetAudioAssetClasses();
	
	bool bFoundAssetClass = false;
	for (const TSoftClassPtr<UObject>& Ptr : DialogueAssetClassPtrs)
	{
		if (!Ptr.IsNull())
		{
			UClass* LoadedClass = Ptr.LoadSynchronous();
			bFoundAssetClass = true;

			if (DialogueAssetClass == nullptr)
			{
				DialogueAssetClass = LoadedClass;
			}
		}
	}

	if (DialogueAssetClass == nullptr)
	{
		DialogueAssetClass = UObject::StaticClass(); // Note: if I use nullptr then SObjectPropertyEntryBox throws a shitfit
	}
	
	bool bSingleDialogueAssetClass = bFoundAssetClass && DialogueAssetClassPtrs.Num() == 1;

	TDelegate<void(const FAssetData&)> OnObjectChangedDelegate;

	OnObjectChangedDelegate = TDelegate<void(const FAssetData&)>::CreateLambda( [this, Type] (const FAssetData& InAssetData)
	{
		FYapTransactions::BeginModify(LOCTEXT("SettingAudioAsset", "Setting audio asset"), GetDialogueNode());

		if (Type == EYapMaturitySetting::Mature)
		{
			GetFragment().GetBitMutable().SetMatureDialogueAudioAsset(InAssetData.GetAsset());
		}
		else
		{
			GetFragment().GetBitMutable().SetSafeDialogueAudioAsset(InAssetData.GetAsset());
		}
		
		FYapTransactions::EndModify();
	});
	
	TSharedRef<SObjectPropertyEntryBox> AudioAssetProperty = SNew(SObjectPropertyEntryBox)
		.IsEnabled(bFoundAssetClass)
		.AllowedClass(bSingleDialogueAssetClass ? DialogueAssetClass : UObject::StaticClass()) // Use this feature if the project only has one dialogue asset class type
		.DisplayBrowse(true)
		.DisplayUseSelected(true)
		.DisplayThumbnail(true)
		.OnShouldFilterAsset(this, &SFlowGraphNode_YapFragmentWidget::OnShouldFilterAsset_AudioAssetWidget)
		.EnableContentPicker(true)
		.ObjectPath_Lambda( [&Asset] () { return Asset->GetPathName(); })
		.OnObjectChanged(OnObjectChangedDelegate)
		.ToolTipText(LOCTEXT("DialogueAudioAsset_Tooltip", "Select an audio asset."));

	TSharedRef<SWidget> Widget = SNew(SOverlay)
	//.Visibility_Lambda()
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
		.Visibility(this, &SFlowGraphNode_YapFragmentWidget::Visibility_AudioAssetErrorState, &Asset)
		.ColorAndOpacity(this, &SFlowGraphNode_YapFragmentWidget::ColorAndOpacity_AudioAssetErrorState, &Asset)
	];
	
	return Widget;
}

FText SFlowGraphNode_YapFragmentWidget::ObjectPathText_AudioAsset() const
{
	return FText::FromString(ObjectPath_AudioAsset());
}

FString SFlowGraphNode_YapFragmentWidget::ObjectPath_AudioAsset() const
{
	const UObject* Asset = GetBit().GetAudioAsset<UObject>(EYapMaturitySetting::Mature);

	if (!Asset) { return ""; }

	return Asset->GetPathName();
}


EVisibility SFlowGraphNode_YapFragmentWidget::Visibility_AudioAssetErrorState(const TSoftObjectPtr<UObject>* Asset) const
{
	if (GetAudioAssetErrorLevel(*Asset) > EYapErrorLevel::OK)
	{
		return EVisibility::HitTestInvisible;
	}
	
	return EVisibility::Hidden;
}

FSlateColor SFlowGraphNode_YapFragmentWidget::ColorAndOpacity_AudioSettingsButton() const
{
	FLinearColor Color;

	switch (GetFragmentAudioErrorLevel())
	{
		case EYapErrorLevel::OK:
		{
			Color = YapColor::DarkGray;
			break;
		}
		case EYapErrorLevel::Warning:
		{
			Color = YapColor::Orange;
			break;
		}
		case EYapErrorLevel::Error:
		{
			Color = YapColor::Red;
			break;
		}
	}
	
	if (!TextExpanderButton->IsHovered())
	{
		Color *= YapColor::LightGray;
	}
	else if (TextExpanderButton->IsPressed())
	{
		Color *= YapColor::LightGray;
	}
	
	return Color;
}

// TODO handle child safe settings somehow
EYapErrorLevel SFlowGraphNode_YapFragmentWidget::GetFragmentAudioErrorLevel() const
{
	const TSoftObjectPtr<UObject>& MatureAsset = GetBit().GetMatureDialogueAudioAsset_SoftPtr<UObject>();
	const TSoftObjectPtr<UObject>& SafeAsset = GetBit().GetSafeDialogueAudioAsset_SoftPtr<UObject>();

	EYapErrorLevel ErrorLevel = GetAudioAssetErrorLevel(MatureAsset);

	if (ErrorLevel < EYapErrorLevel::Error && NeedsChildSafeData())
	{
		// See if the other audio asset raises the error level any further
		ErrorLevel = FMath::Max(ErrorLevel, GetAudioAssetErrorLevel(SafeAsset));
	}

	return ErrorLevel;
}

FSlateColor SFlowGraphNode_YapFragmentWidget::ColorAndOpacity_AudioAssetErrorState(const TSoftObjectPtr<UObject>* Asset) const
{
	FLinearColor Color;

	EYapErrorLevel ErrorLevel = GetAudioAssetErrorLevel(*Asset);

	switch (ErrorLevel)
	{
		case EYapErrorLevel::OK:
		{
			Color = YapColor::DarkGray;
			break;
		}
		case EYapErrorLevel::Warning:
		{
			Color = YapColor::Orange;
			break;
		}
		case EYapErrorLevel::Error:
		{
			Color = YapColor::Red;
			break;
		}
	}
	
	return Color;
}

EYapErrorLevel SFlowGraphNode_YapFragmentWidget::GetAudioAssetErrorLevel(const TSoftObjectPtr<UObject>& Asset) const
{
	EYapMissingAudioErrorLevel MissingAudioBehavior = UYapProjectSettings::GetMissingAudioBehavior();

	// If we have an asset all we need to do is check if it's the correct class type. Always return an error if it's an improper type.
	if (!Asset.IsNull())
	{
		const TArray<TSoftClassPtr<UObject>>& AllowedAssetClasses = UYapProjectSettings::GetAudioAssetClasses();

		/*
		if (AllowedAssetClasses.ContainsByPredicate(&Test))
		{
			return EYapErrorLevel::OK;
		}
		*/
		if (AllowedAssetClasses.ContainsByPredicate( [Asset] (const TSoftClassPtr<UObject>& Class)
		{
			return Asset.LoadSynchronous()->IsA(Class.LoadSynchronous());

			/*
			if (Asset.IsNull())
			{
				return false;
			}
			
			return Asset->GetClass()->IsChildOf(Class.LoadSynchronous());
			*/
			
		} )) // TODO async loading??
		{
			return EYapErrorLevel::OK;
		}
		else
		{
			return EYapErrorLevel::Error;
		}
	}

	// We know that we don't have any audio asset set now. If the dialogue is set to use audio time but does NOT have an audio asset, we either indicate an error (prevent packaging) or indicate a warning (allow packaging) 
	if (
		(GetBit().TimeMode == EYapTimeMode::AudioTime)
		||
		(GetBit().TimeMode == EYapTimeMode::Default && UYapProjectSettings::GetDefaultTimeModeSetting() == EYapTimeMode::AudioTime)
		||
		(GetBit().GetTimeMode(EYapMaturitySetting::Mature) == EYapTimeMode::AudioTime)
	) // TODO handle edge case of only having child-safe asset and no mature asset
	{
		switch (MissingAudioBehavior)
		{
			case EYapMissingAudioErrorLevel::OK:
			{
				return EYapErrorLevel::OK;
			}
			case EYapMissingAudioErrorLevel::Warning:
			{
				return EYapErrorLevel::Warning;
			}
			case EYapMissingAudioErrorLevel::Error:
			{
				return EYapErrorLevel::Error;
			}
		}
	}

	return EYapErrorLevel::OK;
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

// TODO oh my god can I reduce this cruft at all
const FYapFragment& SFlowGraphNode_YapFragmentWidget::GetFragment() const
{
	return GetDialogueNode()->GetFragmentByIndex(FragmentIndex);
}

FYapFragment& SFlowGraphNode_YapFragmentWidget::GetFragment()
{
	return const_cast<FYapFragment&>(const_cast<const SFlowGraphNode_YapFragmentWidget*>(this)->GetFragment());
}

FYapFragment& SFlowGraphNode_YapFragmentWidget::GetFragmentMutable() const
{
	return const_cast<FYapFragment&>(GetFragment());
}

const FYapBit& SFlowGraphNode_YapFragmentWidget::GetBit() const
{
	return GetFragment().GetBit();
}

FYapBit& SFlowGraphNode_YapFragmentWidget::GetBit()
{
	return const_cast<FYapBit&>(const_cast<const SFlowGraphNode_YapFragmentWidget*>(this)->GetBit());
}

const FYapBit& SFlowGraphNode_YapFragmentWidget::GetBitConst()
{
	return GetBit();
}

FYapBit& SFlowGraphNode_YapFragmentWidget::GetBitMutable() const
{
	return const_cast<FYapBit&>(GetBit());
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
					.ToolTipText(LOCTEXT("ClickToEnableOnStartPin_Label", "Click to enable 'On Start' Pin"))
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
					.ToolTipText(LOCTEXT("ClickToEnableOnEndPin_Label", "Click to enable 'On End' Pin"))
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
// MOOD TAG SELECTOR WIDGET
// ------------------------------------------------------------------------------------------------

TSharedRef<SWidget> SFlowGraphNode_YapFragmentWidget::CreateMoodTagSelectorWidget()
{
	FGameplayTag SelectedMoodTag = GetCurrentMoodTag();

	TSharedRef<SUniformWrapPanel> MoodTagSelectorPanel = SNew(SUniformWrapPanel)
		.NumColumnsOverride(4); // TODO use maff

	MoodTagSelectorPanel->AddSlot()
	[
		CreateMoodTagMenuEntryWidget(FGameplayTag::EmptyTag, SelectedMoodTag == FGameplayTag::EmptyTag)
	];
	
	for (const FGameplayTag& MoodTag : UYapProjectSettings::GetMoodTags())
	{
		if (!MoodTag.IsValid())
		{
			UE_LOG(LogYap, Warning, TEXT("Warning: Portrait keys contains an invalid entry. Clean this up!"));
			continue;
		}
		
		bool bSelected = MoodTag == SelectedMoodTag;
		
		MoodTagSelectorPanel->AddSlot()
		[
			CreateMoodTagMenuEntryWidget(MoodTag, bSelected)
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
		.OnMenuOpenChanged(this, &SFlowGraphNode_YapFragmentWidget::OnMenuOpenChanged_MoodTagSelector)
		.ToolTipText(this, &SFlowGraphNode_YapFragmentWidget::ToolTipText_MoodTagSelector)
		.ForegroundColor(this, &SFlowGraphNode_YapFragmentWidget::ForegroundColor_MoodTagSelectorWidget)
		.ButtonContent()
		[
			SNew(SImage)
			.ColorAndOpacity(FSlateColor::UseSubduedForeground())
			.Image(this, &SFlowGraphNode_YapFragmentWidget::Image_MoodTagSelector)
		]
		.MenuContent()
		[
			MoodTagSelectorPanel
		];
}

EVisibility SFlowGraphNode_YapFragmentWidget::Visibility_MoodTagSelector() const
{
	return IsHovered() || MoodTagSelectorMenuOpen ? EVisibility::Visible : EVisibility::Collapsed;
}

void SFlowGraphNode_YapFragmentWidget::OnMenuOpenChanged_MoodTagSelector(bool bMenuOpen)
{
	MoodTagSelectorMenuOpen = bMenuOpen;
}

const FSlateBrush* SFlowGraphNode_YapFragmentWidget::Image_MoodTagSelector() const
{
	return GEditor->GetEditorSubsystem<UYapEditorSubsystem>()->GetMoodTagBrush(GetCurrentMoodTag());
}

FGameplayTag SFlowGraphNode_YapFragmentWidget::GetCurrentMoodTag() const
{
	return GetBit().GetMoodTag();
}

// ================================================================================================
// MOOD TAG MENU ENTRY WIDGET
// ------------------------------------------------------------------------------------------------

TSharedRef<SWidget> SFlowGraphNode_YapFragmentWidget::CreateMoodTagMenuEntryWidget(FGameplayTag MoodTag, bool bSelected, const FText& InLabel, FName InTextStyle)
{
	TSharedPtr<SHorizontalBox> HBox = SNew(SHorizontalBox);

	TSharedPtr<SImage> PortraitIconImage;
		
	TSharedPtr<FSlateImageBrush> MoodTagBrush = GEditor->GetEditorSubsystem<UYapEditorSubsystem>()->GetMoodTagIcon(MoodTag);
	
	if (MoodTag.IsValid())
	{
		HBox->AddSlot()
		.AutoWidth()
		.Padding(0, 0, 0, 0)
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Center)
		[
			SAssignNew(PortraitIconImage, SImage)
			.ColorAndOpacity(FSlateColor::UseForeground())
			.Image(MoodTagBrush.Get())
		];
	}

	FText ToolTipText;
	
	if (MoodTag.IsValid())
	{
		TSharedPtr<FGameplayTagNode> TagNode = UGameplayTagsManager::Get().FindTagNode(MoodTag);
		ToolTipText = FText::FromName(TagNode->GetSimpleTagName());
	}
	else
	{
		ToolTipText = LOCTEXT("Default", "Default");
	}
	
	return SNew(SButton)
	.Cursor(EMouseCursor::Default)
	.ContentPadding(FMargin(4, 4))
	.ButtonStyle(FAppStyle::Get(), "SimpleButton")
	.ClickMethod(EButtonClickMethod::MouseDown)
	.OnClicked(this, &SFlowGraphNode_YapFragmentWidget::OnClicked_MoodTagMenuEntry, MoodTag)
	.ToolTipText(ToolTipText)
	[
		SNew(SOverlay)
		+ SOverlay::Slot()
		.Padding(-3)
		[
			SNew(SBorder)
			.Visibility_Lambda([this, MoodTag]()
			{
				if (GetBit().GetMoodTag() == MoodTag)
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
			.Image(MoodTagBrush.Get())
		]
	];
}

FReply SFlowGraphNode_YapFragmentWidget::OnClicked_MoodTagMenuEntry(FGameplayTag NewValue)
{	
	FYapTransactions::BeginModify(LOCTEXT("NodeMoodTagChanged", "Portrait Key Changed"), GetDialogueNode());

	GetBit().SetMoodTag(NewValue);

	FYapTransactions::EndModify();

	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE