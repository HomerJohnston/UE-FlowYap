// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license.

#include "YapEditor/NodeWidgets/SFlowGraphNode_YapFragmentWidget.h"

#include "FlowAsset.h"
#include "Engine/World.h"
#include "PropertyCustomizationHelpers.h"
#include "SAssetDropTarget.h"
#include "SLevelOfDetailBranchNode.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "Widgets/Notifications/SProgressBar.h"
#include "Yap/YapCharacter.h"
#include "YapEditor/YapEditorColor.h"
#include "YapEditor/YapEditorSubsystem.h"
#include "Yap/YapFragment.h"
#include "Yap/YapProjectSettings.h"
#include "YapEditor/YapTransactions.h"
#include "Yap/YapUtil.h"
#include "YapEditor/YapEditorStyle.h"
#include "Yap/Enums/YapMissingAudioErrorLevel.h"
#include "Yap/Nodes/FlowNode_YapDialogue.h"
#include "YapEditor/NodeWidgets/SFlowGraphNode_YapDialogueWidget.h"
#include "Yap/YapBitReplacement.h"
#include "YapEditor/YapInputTracker.h"
#include "YapEditor/SlateWidgets/SYapTextPropertyEditableTextBox.h"
#include "YapEditor/Helpers/YapEditableTextPropertyHandle.h"
#include "YapEditor/SlateWidgets/SYapActivationCounterWidget.h"
#include "YapEditor/SlateWidgets/SYapConditionsScrollBox.h"
#include "YapEditor/SlateWidgets/SYapButtonPopup.h"
#include "YapEditor/SlateWidgets/SYapPropertyMenuAssetPicker.h"
#include "Templates/FunctionFwd.h"
#include "Yap/YapSubsystem.h"
#include "Yap/Enums/YapErrorLevel.h"
#include "YapEditor/YapDeveloperSettings.h"
#include "Framework/MultiBox/SToolBarButtonBlock.h"
#include "Widgets/Text/SMultiLineEditableText.h"
#include "YapEditor/YapEditorLog.h"
#include "YapEditor/Globals/YapEditorFuncs.h"
#include "YapEditor/GraphNodes/FlowGraphNode_YapDialogue.h"
#include "YapEditor/Helpers/ProgressionSettingWidget.h"
#include "YapEditor/SlateWidgets/SYapGameplayTagTypedPicker.h"

FSlateFontInfo SFlowGraphNode_YapFragmentWidget::DialogueTextFont;

#define LOCTEXT_NAMESPACE "YapEditor"

SLATE_IMPLEMENT_WIDGET(SFlowGraphNode_YapFragmentWidget)
void SFlowGraphNode_YapFragmentWidget::PrivateRegisterAttributes(struct FSlateAttributeDescriptor::FInitializer&)
{
	// TODO wtf does anything go in here?
}

bool SFlowGraphNode_YapFragmentWidget::NeedsChildSafeData() const
{
	return GetFragment().bEnableChildSafe;
}

bool SFlowGraphNode_YapFragmentWidget::HasAnyChildSafeData() const
{
	const FYapBit& ChildSafeBit = GetFragment().GetChildSafeBit();

	return (ChildSafeBit.HasDialogueText() || ChildSafeBit.HasTitleText() || ChildSafeBit.HasAudioAsset());
}

bool SFlowGraphNode_YapFragmentWidget::HasCompleteChildSafeData() const
{
	if (!HasAnyChildSafeData())
	{
		return false;
	}

	if (!NeedsChildSafeData())
	{
		return true;
	}

	const FYapBit& MatureBit = GetFragment().GetMatureBit();
	const FYapBit& ChildSafeBit = GetFragment().GetChildSafeBit();

	// If any one item is set, then both items must be set
	bool bDialogueTextOK = MatureBit.HasDialogueText() == ChildSafeBit.HasDialogueText();
	bool bTitleTextOK = MatureBit.HasTitleText() == ChildSafeBit.HasTitleText(); 
	bool bAudioAssetOK = MatureBit.HasAudioAsset() == ChildSafeBit.HasAudioAsset();

	return bDialogueTextOK && bTitleTextOK && bAudioAssetOK;
}

TSharedRef<SWidget> SFlowGraphNode_YapFragmentWidget::CreateCentreTextDisplayWidget()
{
	return SNew(SYapButtonPopup)
	.ButtonStyle(FYapEditorStyle::Get(), YapStyles.ButtonStyle_NoBorder)
	.PopupContentGetter(FPopupContentGetter::CreateSP(this, &ThisClass::PopupContentGetter_ExpandedEditor))
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
			.Visibility(this, &ThisClass::Visibility_TitleText)
			[
				CreateTitleTextDisplayWidget()
			]
		]
	];
}

EYapMaturitySetting SFlowGraphNode_YapFragmentWidget::GetDisplayMaturitySetting() const
{
	if (GEditor->PlayWorld)
	{
		return UYapSubsystem::GetCurrentMaturitySetting();
	}

	if (!NeedsChildSafeData())
	{
		return EYapMaturitySetting::Mature;
	}
	
	return bChildSafeCheckBoxHovered ? EYapMaturitySetting::ChildSafe : EYapMaturitySetting::Mature;
}

void SFlowGraphNode_YapFragmentWidget::Construct(const FArguments& InArgs, SFlowGraphNode_YapDialogueWidget* InOwner, uint8 InFragmentIndex)
{	
	Owner = InOwner;
	
	FragmentIndex = InFragmentIndex;

	TimeModeButtonColors =
	{
		{ EYapTimeMode::None, YapColor::Red },
		{ EYapTimeMode::Default, YapColor::Green },
		{ EYapTimeMode::AudioTime, YapColor::Cyan },
		{ EYapTimeMode::TextTime, YapColor::LightBlue },
		{ EYapTimeMode::ManualTime, YapColor::Orange },
	};

	const FSlateFontInfo& DialogueFont = UYapDeveloperSettings::GetGraphDialogueFont();

	if (DialogueFont.HasValidFont())
	{
		DialogueTextFont = DialogueFont;
	}
	else
	{
		DialogueTextFont = YapFonts.Font_DialogueText;
	}
	
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
	.Visibility(this, &ThisClass::Visibility_FragmentControlsWidget)
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
			.Visibility(this, &ThisClass::Visibility_FragmentShiftWidget, EYapFragmentControlsDirection::Up)
			.OnClicked(this, &ThisClass::OnClicked_FragmentShift, EYapFragmentControlsDirection::Up)
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
			.OnClicked(this, &ThisClass::OnClicked_FragmentDelete)
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
			.Visibility(this, &ThisClass::Visibility_FragmentShiftWidget, EYapFragmentControlsDirection::Down)
			.OnClicked(this, &ThisClass::OnClicked_FragmentShift, EYapFragmentControlsDirection::Down)
			[
				SNew(SImage)
				.Image(FAppStyle::Get().GetBrush("Icons.ChevronDown"))
				.DesiredSizeOverride(FVector2D(16, 16))
				.ColorAndOpacity(FSlateColor::UseForeground())
			]
		]
	];
}

bool SFlowGraphNode_YapFragmentWidget::Enabled_AudioPreviewButton(const TSoftObjectPtr<UObject>* Object) const
{
	if (!Object)
	{
		return false;
	}

	if (!Object->IsValid())
	{
		return false;
	}

	return true;
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
				Yap::EditorFuncs::PostNotificationInfo_Warning
				(
					LOCTEXT("AudioPreview_UnknownWarning_Title", "Cannot Play Audio Preview"),
					LOCTEXT("AudioPreview_UnknownWarning_Description", "Unknown error!")
				);
			}
		}
		else
		{
			Yap::EditorFuncs::PostNotificationInfo_Warning
			(
				LOCTEXT("AudioPreview_BrokerPlayFunctionMissingWarning_Title", "Cannot Play Audio Preview"),
				LOCTEXT("AudioPreview_BrokerPlayFunctionMissingWarning_Description", "Your Broker Class must implement the \"PlayDialogueAudioAssetInEditor\" function.")
			);
		}
	}
	else
	{
		Yap::EditorFuncs::PostNotificationInfo_Warning
		(
			LOCTEXT("AudioPreview_BrokerPlayFunctionMissingWarning_Title", "Cannot Play Audio Preview"),
			LOCTEXT("AudioPreview_BrokerMissingWarning_Description", "Yap Broker class missing - you must set a Yap Broker class in project settings.")
		);
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
		.IsEnabled(this, &ThisClass::Enabled_AudioPreviewButton, AudioAsset)
		.ToolTipText(LOCTEXT("PlayAudio", "Play audio"))
		.OnClicked(this, &ThisClass::OnClicked_AudioPreviewWidget, AudioAsset)
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
	.Visibility(this, &ThisClass::Visibility_RowHighlight)
	.BorderBackgroundColor(this, &ThisClass::BorderBackgroundColor_RowHighlight);
}

void SFlowGraphNode_YapFragmentWidget::OnTextCommitted_FragmentActivationLimit(const FText& Text, ETextCommit::Type Arg)
{
	FYapTransactions::BeginModify(LOCTEXT("ChangeActivationLimit", "Change activation limit"), GetDialogueNode());

	GetFragmentMutable().ActivationLimit = FCString::Atoi(*Text.ToString());

	GetDialogueNode()->OnReconstructionRequested.Execute();
	
	FYapTransactions::EndModify();
}

TSharedRef<SWidget> SFlowGraphNode_YapFragmentWidget::CreateUpperFragmentBar()
{
	UFlowNode_YapDialogue* DialogueNode = GetDialogueNode();

	TOptional<bool>* SkippableSettingRaw = &GetFragmentMutable().Skippable;
	const TAttribute<bool> SkippableDefaultAttr = TAttribute<bool>::CreateLambda( [this] () { return GetDialogueNode()->GetSkippable(); });
	const TAttribute<bool> SkippableEvaluatedAttr = TAttribute<bool>::CreateLambda( [this, SkippableDefaultAttr] ()
	{
		return GetFragment().GetSkippable(SkippableDefaultAttr.Get());
	});
	TOptional<bool>* AutoAdvanceSettingRaw = &GetFragmentMutable().AutoAdvance;
	const TAttribute<bool> AutoAdvanceDefaultAttr = TAttribute<bool>::CreateLambda( [this] () { return GetDialogueNode()->GetAutoAdvance(); });
	const TAttribute<bool> AutoAdvanceEvaluatedAttr = TAttribute<bool>::CreateLambda( [this, AutoAdvanceDefaultAttr] ()
	{
		return GetFragment().GetAutoAdvance(AutoAdvanceDefaultAttr.Get());
	});

	TSharedRef<SWidget> ProgressionPopupButton = MakeProgressionPopupButton(SkippableSettingRaw, SkippableEvaluatedAttr, AutoAdvanceSettingRaw, AutoAdvanceEvaluatedAttr);
	
	TSharedRef<SWidget> Box = SNew(SBox)
	.Padding(0, 0, 32, 4)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Center)
		.Padding(6, -8, 0, -8)
		.AutoWidth()
		[
			SNew(SBox)
			.WidthOverride(20)
			[
				SNew(SYapActivationCounterWidget, FOnTextCommitted::CreateSP(this, &ThisClass::OnTextCommitted_FragmentActivationLimit))
				.ActivationCount(this, &ThisClass::GetFragmentActivationCount)
				.ActivationLimit(this, &ThisClass::GetFragmentActivationLimit)
				.FontHeight(10)
			]
		]
		+ SHorizontalBox::Slot()
		.HAlign(HAlign_Fill)
		.Padding(6, 0, 0, 0)
		[
			SAssignNew(ConditionsScrollBox, SYapConditionsScrollBox)
			.DialogueNode(DialogueNode)
			.FragmentIndex(FragmentIndex)
			.ConditionsArrayProperty(FindFProperty<FArrayProperty>(FYapFragment::StaticStruct(), GET_MEMBER_NAME_CHECKED(FYapFragment, Conditions)))
			.ConditionsContainer(&GetFragmentMutable())
			.OnConditionsArrayChanged(Owner, &SFlowGraphNode_YapDialogueWidget::OnConditionsArrayChanged)
		]
		+ SHorizontalBox::Slot()
		.HAlign(HAlign_Right)
		.AutoWidth()
		.VAlign(VAlign_Fill)
		.Padding(4, 0, 1, 0)
		[
			SNew(SLevelOfDetailBranchNode)
			.Visibility(this, &ThisClass::Visibility_FragmentTagWidget)
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
				ProgressionPopupButton
			]
		]
	];
	
	return Box;
}

EVisibility SFlowGraphNode_YapFragmentWidget::Visibility_FragmentTagWidget() const
{
	return GetDialogueNode()->GetDialogueTag().IsValid() ? EVisibility::Visible : EVisibility::Collapsed;
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
	if (!NeedsChildSafeData())
	{
		FYapScopedTransaction T("ChangeChildSafeSettings", LOCTEXT("TurnOnChildSafe", "Enable child-safe settings"), GetDialogueNode());

		GetFragmentMutable().bEnableChildSafe = true;
	}
	else
	{
		// Turn off child safety settings if we don't have any data assigned; otherwise flash a warning
		if (!HasAnyChildSafeData())
		{
			FYapScopedTransaction T("ChangeChildSafeSettings", LOCTEXT("TurnOffChildSafe", "Disable child-safe settings"), GetDialogueNode());

			GetFragmentMutable().bEnableChildSafe = false;
		}
		else
		{
			EAppReturnType::Type ReturnType = FMessageDialog::Open(EAppMsgType::YesNoCancel, LOCTEXT("TurnOffChildSafeSettingsDialog_DataWarning", "Node contains child-safe data: do you want to reset it? Press 'Yes' to remove child-safe data, or 'No' to leave it hidden."), LOCTEXT("TurnOffChildSafeSettingsDialog_Title", "Turn Off Child-Safe Settings"));

			switch (ReturnType)
			{
				case EAppReturnType::Yes:
				{
					FYapScopedTransaction T("ChangeChildSafeSettings", LOCTEXT("ResetChildSafeSettings", "Reset child-safe settings"), GetDialogueNode());

					GetFragmentMutable().GetChildSafeBitMutable().ClearAllData();
					GetFragmentMutable().bEnableChildSafe = false;

					break;
				}
				case EAppReturnType::No:
				{
					FYapScopedTransaction T("ChangeChildSafeSettings", LOCTEXT("TurnOffChildSafe", "Disable child-safe settings"), GetDialogueNode());
					
					GetFragmentMutable().bEnableChildSafe = false;

					break;
				}
				default: // Cancel
				{
					// Do nothing, just close the dialog
					break;
				}
			}
		}
	}
}

FSlateColor SFlowGraphNode_YapFragmentWidget::ColorAndOpacity_ChildSafeSettingsCheckBox() const
{
	if (NeedsChildSafeData())
	{
		return HasCompleteChildSafeData() ? YapColor::LightBlue : YapColor::Red;
	}
	else
	{
		return HasAnyChildSafeData() ? YapColor::YellowGray : YapColor::Button_Unset();
	}
}

FSlateColor SFlowGraphNode_YapFragmentWidget::BorderBackgroundColor_DirectedAtImage() const
{
	const TSoftObjectPtr<UYapCharacter> DirectedAtAsset = GetFragment().GetDirectedAtAsset();

	FLinearColor Color;
	
	if (DirectedAtAsset.IsValid())
	{
		Color = DirectedAtAsset.Get()->GetEntityColor();
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

		GetFragmentMutable().SetDirectedAt(Character);

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

		GetFragmentMutable().SetDirectedAt(nullptr);

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
			.InitialObject(GetFragmentMutable().GetDirectedAt())
			.OnSet(this, &ThisClass::OnSetNewDirectedAtAsset)
		]
	];
}

const FSlateBrush* SFlowGraphNode_YapFragmentWidget::Image_DirectedAtWidget() const
{
	TSharedPtr<FSlateImageBrush> PortraitBrush = UYapEditorSubsystem::GetCharacterPortraitBrush(GetFragmentMutable().GetDirectedAt(), FGameplayTag::EmptyTag);

	if (PortraitBrush && PortraitBrush->GetResourceObject())
	{
		return PortraitBrush.Get();
	}
	else
	{
		return FYapEditorStyle::GetImageBrush(YapBrushes.None);
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
	.BorderBackgroundColor(this, &ThisClass::BorderBackgroundColor_DirectedAtImage)
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
				.OnAreAssetsAcceptableForDrop(this, &ThisClass::OnAreAssetsAcceptableForDrop_DirectedAtWidget)
				.OnAssetsDropped(this, &ThisClass::OnAssetsDropped_DirectedAtWidget)
				[
					SNew(SYapButtonPopup)
					.PopupPlacement(MenuPlacement_BelowAnchor)
					.OnClicked(this, &ThisClass::OnClicked_DirectedAtWidget)
					.PopupContentGetter(FPopupContentGetter::CreateSP(this, &ThisClass::PopupContentGetter_DirectedAtWidget))
					.ButtonStyle(FYapEditorStyle::Get(), YapStyles.ButtonStyle_HoverHintOnly)
					.ButtonBackgroundColor(YapColor::DarkGray)
					.ButtonContent()
					[
						SNew(SImage)
						.DesiredSizeOverride(FVector2D(PortraitSize, PortraitSize))
						.Image(this, &ThisClass::Image_DirectedAtWidget)
					]
				]
			]
			.LowDetail()
			[
				SNew(SImage)
				.DesiredSizeOverride(FVector2D(PortraitSize, PortraitSize))
				.Image(this, &ThisClass::Image_DirectedAtWidget)
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

	GetFragmentMutable().GetChildSafeBitMutable().SetDialogueAudioAsset(Object);

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
							.OnAreAssetsAcceptableForDrop(this, &ThisClass::OnAreAssetsAcceptableForDrop_ChildSafeButton)
							.OnAssetsDropped(this, &ThisClass::OnAssetsDropped_ChildSafeButton)
							[
								SAssignNew(ChildSafeCheckBox, SCheckBox)
								.Cursor(EMouseCursor::Default)
								.Style(FYapEditorStyle::Get(), YapStyles.CheckBoxStyle_Skippable)
								.Type(ESlateCheckBoxType::ToggleButton)
								.Padding(FMargin(0, 0))
								.CheckBoxContentUsesAutoWidth(true)
								.ToolTip(nullptr) // Don't show a tooltip, it's distracting
								.IsChecked(this, &ThisClass::IsChecked_ChildSafeSettings)
								.OnCheckStateChanged(this, &ThisClass::OnCheckStateChanged_MaturitySettings)
								.Content()
								[
									SNew(SBox)
									.WidthOverride(20)
									.HeightOverride(20)
									.HAlign(HAlign_Center)
									.VAlign(VAlign_Center)
									[
										SNew(SImage)
										.ColorAndOpacity(this, &ThisClass::ColorAndOpacity_ChildSafeSettingsCheckBox)
										.DesiredSizeOverride(FVector2D(16, 16))
										.Image(FYapEditorStyle::GetImageBrush(YapBrushes.Icon_Baby))
									]
								]
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

void SFlowGraphNode_YapFragmentWidget::OnValueCommitted_ManualTime(float NewValue, ETextCommit::Type CommitType, EYapMaturitySetting MaturitySetting)
{
	FYapTransactions::BeginModify(LOCTEXT("EnterManualTimeValue", "Enter manual time value"), GetDialogueNode());

	if (CommitType != ETextCommit::OnCleared)
	{
		GetFragmentMutable().GetBitMutable(MaturitySetting).SetManualTime(NewValue);
	}

	FYapTransactions::EndModify();
}

TSharedRef<SWidget> SFlowGraphNode_YapFragmentWidget::MakeTimeSettingRow(EYapTimeMode TimeMode, EYapMaturitySetting MaturitySetting)
{
	using FLabelText =				FText;
	using FToolTipText =			FText;
	using FSlateBrushIcon =			const FSlateBrush*;
	using FValueFunction =			TOptional<float>	(SFlowGraphNode_YapFragmentWidget::*) (EYapMaturitySetting) const;
	using FValueUpdatedFunction =	void				(SFlowGraphNode_YapFragmentWidget::*) (float, EYapMaturitySetting);
	using FValueCommittedFunction =	void				(SFlowGraphNode_YapFragmentWidget::*) (float, ETextCommit::Type, EYapMaturitySetting);

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

	// Each entry in this represents how to display each row.
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

	auto OnClickedDelegate = FOnClicked::CreateRaw(this, &ThisClass::OnClicked_SetTimeModeButton, TimeMode);
	auto ButtonColorDelegate = TAttribute<FSlateColor>::CreateRaw(this, &ThisClass::ButtonColorAndOpacity_UseTimeMode, TimeMode, TimeModeButtonColors[TimeMode], MaturitySetting);
	auto ForegroundColorDelegate = TAttribute<FSlateColor>::CreateRaw(this, &ThisClass::ForegroundColor_TimeSettingButton, TimeMode, YapColor::White);

	TSharedPtr<SHorizontalBox> RowBox = SNew(SHorizontalBox);

	// Toss in a filler spacer to let widgets flow to the right side
	RowBox->AddSlot()
	.FillWidth(1.0)
	[
		SNew(SSpacer)
	];

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
	}

	if (MaturitySetting == EYapMaturitySetting::Mature || TimeMode != EYapTimeMode::Default)
	{
		RowBox->AddSlot()
		.AutoWidth()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Center)
		.Padding(0, 0, 8, 0)
		[
			SNew(SButton)
			.Cursor(EMouseCursor::Default)
			.IsEnabled(MaturitySetting == EYapMaturitySetting::Mature)
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

	// Add a numeric box if this row has a value getter
	if (ValueFunction)
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
					.IsEnabled_Lambda( [this] () { return GetFragment().GetTimeModeSetting() == EYapTimeMode::ManualTime; } )
					.ToolTipText(LOCTEXT("FragmentTimeEntry_Tooltip", "Time this dialogue fragment will play for"))
					//.Justification(ETextJustify::Center) // Numeric Entry Box has a bug, when spinbox is turned on this doesn't work. So don't use it for any of the rows.
					.AllowSpin(true)
					.Delta(0.05f)
					.MaxValue(60) // TODO project setting?
					.MaxSliderValue(10) // TODO project setting?
					.MaxFractionalDigits(1) // TODO project setting?
					.OnValueChanged(this, UpdatedFunction, MaturitySetting)
					.MinValue(0)
					.Value(this, ValueFunction, MaturitySetting)
					.OnValueCommitted(this, CommittedFunction, MaturitySetting)
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

TOptional<float> SFlowGraphNode_YapFragmentWidget::Value_TimeSetting_AudioTime(EYapMaturitySetting MaturitySetting) const
{
	return GetFragment().GetBit(MaturitySetting).GetAudioTime();
}

TOptional<float> SFlowGraphNode_YapFragmentWidget::Value_TimeSetting_TextTime(EYapMaturitySetting MaturitySetting) const
{
	return GetFragment().GetBit(MaturitySetting).GetTextTime();
}

TOptional<float> SFlowGraphNode_YapFragmentWidget::Value_TimeSetting_ManualTime(EYapMaturitySetting MaturitySetting) const
{
	return GetFragment().GetBit(MaturitySetting).GetManualTime();
}

FSlateColor SFlowGraphNode_YapFragmentWidget::ButtonColor_TimeSettingButton() const
{
	EYapTimeMode TimeMode = GetFragment().GetTimeModeSetting();

	if (TimeMode == EYapTimeMode::Default)
	{
		return YapColor::DimGray;
	}
	
	return TimeModeButtonColors[GetFragment().GetTimeModeSetting()];
}

EVisibility SFlowGraphNode_YapFragmentWidget::Visibility_AudioSettingsButton() const
{
	if (UYapProjectSettings::GetMissingAudioBehavior() != EYapMissingAudioErrorLevel::OK)
	{
		return EVisibility::Visible;
	}

	if (GetFragment().HasAudio())
	{
		return EVisibility::Visible;
	}

	return EVisibility::Collapsed;
}

EVisibility SFlowGraphNode_YapFragmentWidget::Visibility_DialogueErrorState() const
{
	if (!NeedsChildSafeData())
	{
		return EVisibility::Collapsed;
	}

	const FYapBit& MatureBit = GetFragment().GetMatureBit();
	const FYapBit& ChildSafeBit = GetFragment().GetChildSafeBit();
	
	if (MatureBit.HasDialogueText() != ChildSafeBit.HasDialogueText())
	{
		return EVisibility::Visible;
	}

	return EVisibility::Collapsed;
}

// TODO code duplication with UFlowGraphNode_YapDialogue::AutoAssignAudioOnAllFragments()
bool CheckAudioAssetUsesAudioID(const UFlowNode_YapDialogue* Node, int32 FragmentIndex, TSoftObjectPtr<UObject> Asset, bool& bCorrectMatch)
{
	int32 AudioIDLen = Node->GetAudioID().Len();
	int32 FragmentIDLen = 3; // TODO magic number move this to project settings or some other constant
	
	FRegexPattern RegexActual(FString::Format(TEXT("[a-zA-Z]{{0}}-\\d{{1}}"), {AudioIDLen, FragmentIDLen}));
	FRegexMatcher RegexMatcher(RegexActual, *Asset.ToString());

	if (RegexMatcher.FindNext())
	{
		FString ID = RegexMatcher.GetCaptureGroup(0);

		FString AudioID = ID.LeftChop(FragmentIDLen + 1);
				
		int32 IDInt = FCString::Atoi(*ID.RightChop(AudioIDLen + 1));

		if (AudioID == Node->GetAudioID() && IDInt - 1 == FragmentIndex)
		{
			bCorrectMatch = true;
		}
		else
		{
			bCorrectMatch = false;
		}
		
		return true;
	}

	return false;
}

FSlateColor SFlowGraphNode_YapFragmentWidget::ColorAndOpacity_AudioID() const
{
	const TSoftObjectPtr<UObject>& MatureAudioAsset = GetFragment().GetMatureBit().GetAudioAsset<UObject>();
	const TSoftObjectPtr<UObject>& SafeAudioAsset = GetFragment().GetChildSafeBit().GetAudioAsset<UObject>();

	bool bHasAudio = false;
	
	if (!MatureAudioAsset.IsNull())
	{
		bool bCorrectMatch;
		
		if (CheckAudioAssetUsesAudioID(GetDialogueNode(), FragmentIndex, MatureAudioAsset, bCorrectMatch))
		{
			if (bCorrectMatch)
			{
				bHasAudio = true;
			}
			else
			{
				return YapColor::Red;
			}
		}
	}

	if (bHasAudio && NeedsChildSafeData() && !SafeAudioAsset.IsNull())
	{
		bool bCorrectMatch;
		
		if (CheckAudioAssetUsesAudioID(GetDialogueNode(), FragmentIndex, SafeAudioAsset, bCorrectMatch))
		{
			if (!bCorrectMatch)
			{
				return YapColor::Red;
			}
		}
	}

	FLinearColor Color = bHasAudio ? YapColor::LightBlue : YapColor::DimGray;

	if (!FragmentOverlay->IsHovered())
	{
		Color *= YapColor::LightGray;
	}

	return Color;
}

// ================================================================================================
// DIALOGUE WIDGET
// ------------------------------------------------------------------------------------------------


TSharedRef<SWidget> SFlowGraphNode_YapFragmentWidget::CreateDialogueDisplayWidget()
{
	int32 TimeSliderSize = 8;
	//FMargin TimerSliderPadding = GetDialogueNode()->UsesTitleText() ? FMargin(0, 0, 0, -(TimeSliderSize / 2) - 2) : FMargin(0, 0, 0, -(TimeSliderSize / 2));
	FMargin TimerSliderPadding = FMargin(0, 0, 0, -(TimeSliderSize / 2) - 2);

	const FText* MatureDialogueText = &GetFragment().GetMatureBit().GetDialogueText();
	const FText* SafeDialogueText = &GetFragment().GetChildSafeBit().GetDialogueText();

	FNumberFormattingOptions Args;
	Args.UseGrouping = false;
	Args.MinimumIntegralDigits = 3;

	const FString FragmentIndexText = FText::AsNumber(FragmentIndex + 1, &Args).ToString();
	UFlowNode_YapDialogue* DialogueNode = GetDialogueNode();
	
	TSharedRef<SWidget> Widget = SNew(SLevelOfDetailBranchNode)
	.UseLowDetailSlot(Owner, &SFlowGraphNode_YapDialogueWidget::UseLowDetail)
	.HighDetail()
	[
		SNew(SOverlay)
		+ SOverlay::Slot()
		[
			SNew(SAssetDropTarget)
			.bSupportsMultiDrop(false)
			.OnAreAssetsAcceptableForDrop(this, &ThisClass::OnAreAssetsAcceptableForDrop_TextWidget)
			.OnAssetsDropped(this, &ThisClass::OnAssetsDropped_TextWidget)
			[
				SNew(SBorder)
				.Cursor(EMouseCursor::Default)
				.BorderImage(FYapEditorStyle::GetImageBrush(YapBrushes.Box_SolidWhite))
				.BorderBackgroundColor(FSlateColor::UseForeground())
				.ToolTipText(this, &ThisClass::ToolTipText_TextDisplayWidget, LOCTEXT("DialogueText_Header", "Dialogue Text"), MatureDialogueText, SafeDialogueText)
				.Padding(0)
				[
					SNew(SOverlay)
					+ SOverlay::Slot()
					.Padding(4, 0, 4, 2)
					[
						SNew(STextBlock)
						.AutoWrapText_Lambda([] () { return UYapProjectSettings::GetWrapDialogueText(); })
						.TextStyle(FYapEditorStyle::Get(), YapStyles.TextBlockStyle_DialogueText)
						.Font(DialogueTextFont)
						.Text(this, &ThisClass::Text_TextDisplayWidget, MatureDialogueText, SafeDialogueText)
						.ColorAndOpacity(this, &ThisClass::ColorAndOpacity_TextDisplayWidget, YapColor::LightGray, MatureDialogueText, SafeDialogueText)
					]
					+ SOverlay::Slot()
					.VAlign(VAlign_Center)
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Visibility_Lambda( [this] ()
						{
							return GetFragment().GetBit(GetDisplayMaturitySetting()).HasDialogueText() ? EVisibility::Hidden : EVisibility::HitTestInvisible;
						})
						.Justification(ETextJustify::Center)
						.TextStyle(FYapEditorStyle::Get(), YapStyles.TextBlockStyle_DialogueText)
						.Text_Lambda( [this] ()
						{
							if (!NeedsChildSafeData())
							{
								return LOCTEXT("DialogueText_None", "Dialogue Text (None)");
							}
							
							return GetDisplayMaturitySetting() == EYapMaturitySetting::Mature ? LOCTEXT("MatureDialogueText_None", "Mature Dialogue Text (None)") : LOCTEXT("SafeDialogueText_None", "Child-Safe Dialogue Text (None)");
						})
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
				.ToolTipText(this, &ThisClass::ToolTipText_TextDisplayWidget, LOCTEXT("DialogueText_Header", "Dialogue Text"), MatureDialogueText, SafeDialogueText)
				[
					SNew(SProgressBar)
					.BorderPadding(0)
					.Percent(this, &ThisClass::FragmentTime_Percent)
					.Style(FYapEditorStyle::Get(), YapStyles.ProgressBarStyle_FragmentTimePadding)
					.FillColorAndOpacity(this, &ThisClass::FillColorAndOpacity_FragmentTimePadding)
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
				.ToolTipText(this, &ThisClass::ToolTipText_FragmentTimePadding)
				[
					SNew(SImage)
					.Image(FYapEditorStyle::GetImageBrush(YapBrushes.Icon_FilledCircle))
					.ColorAndOpacity(this, &ThisClass::ButtonColor_TimeSettingButton)
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
					.Percent(this, &ThisClass::FragmentTimePadding_Percent)
					.Style(FYapEditorStyle::Get(), YapStyles.ProgressBarStyle_FragmentTimePadding)
					.FillColorAndOpacity(this, &ThisClass::FillColorAndOpacity_FragmentTimePadding)
					.BarFillType(EProgressBarFillType::LeftToRight)
				]
			]
		]
		+ SOverlay::Slot()
		.VAlign(VAlign_Top)
		.HAlign(HAlign_Right)
		[
			SNew(SImage)
			.Image(FYapEditorStyle::GetImageBrush(YapBrushes.Icon_CornerDropdown_Right))
			.Visibility(this, &ThisClass::Visibility_AudioSettingsButton)
			.ColorAndOpacity(this, &ThisClass::ColorAndOpacity_AudioSettingsButton)
		]
		+ SOverlay::Slot()
		.VAlign(VAlign_Fill)
		.HAlign(HAlign_Fill)
		.Padding(-2)
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("MarqueeSelection"))
			.Visibility(this, &ThisClass::Visibility_DialogueErrorState)
			.BorderBackgroundColor(YapColor::Red)
		]
		+ SOverlay::Slot()
		.VAlign(VAlign_Bottom)
		.HAlign(HAlign_Right)
		.Padding(0)
		[
			SNew(SBorder)
			.BorderImage(FYapEditorStyle::GetImageBrush(YapBrushes.Icon_IDTag))
			.BorderBackgroundColor(YapColor::Noir)
			.Padding(4)
			[
				SNew(STextBlock)
				.ColorAndOpacity(this, &ThisClass::ColorAndOpacity_AudioID)
				.Text_Lambda( [FragmentIndexText, DialogueNode] () { return FText::AsCultureInvariant(DialogueNode->GetAudioID() + "-" + FragmentIndexText); } )
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
	return (GetDisplayMaturitySetting()) == EYapMaturitySetting::Mature ? *MatureText : *SafeText; 
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
	float Width = NeedsChildSafeData() ? 500 : 600; // TODO developer setting

	return SNew(SSplitter)
	.Orientation(Orient_Vertical)
	.PhysicalSplitterHandleSize(2.0)
	+ SSplitter::Slot()
	.Resizable(false)
	.SizeRule(SSplitter::SizeToContent)
	[
		BuildDialogueEditors_ExpandedEditor(Width)
	]
	+ SSplitter::Slot()
	.Resizable(false)
	.SizeRule(SSplitter::SizeToContent)
	[
		BuildTimeSettings_ExpandedEditor(Width)
	]
	+ SSplitter::Slot()
	.Resizable(false)
	.SizeRule(SSplitter::SizeToContent)
	[
		BuildPaddingSettings_ExpandedEditor(Width)
	];
}

// TODO this is all doubled-up for mature and child safe. Simplify it.
TSharedRef<SWidget> SFlowGraphNode_YapFragmentWidget::BuildDialogueEditors_ExpandedEditor(float Width)
{
	bool bMatureOnly = !NeedsChildSafeData();

	TSharedRef<SSplitter> Splitter = SNew(SSplitter)
	.Orientation(Orient_Horizontal)
	.PhysicalSplitterHandleSize(2.0f);
	
	{
		FYapBit& Bit = GetFragmentMutable().GetMatureBitMutable();

		FText Title = (bMatureOnly) ? LOCTEXT("DialogueDataEditor_Title", "DIALOGUE") : LOCTEXT("MatureDialogueDataEditor_Title", "MATURE DIALOGUE");
		FText DialogueTextHint = LOCTEXT("DialogueTextEntryBox_Hint", "Dialogue text...");
		FText TitleTextHint = LOCTEXT("DialogueTextEntryBox_Hint", "Title text...");
		
		FMargin Padding(0, 4, 4, 4);
		
		
		Splitter->AddSlot()
		.Resizable(false)
		[
			BuildDialogueEditor_SingleSide(Title, DialogueTextHint, TitleTextHint, Width, Padding, Bit)
		];
	}
	
	if (!bMatureOnly)
	{
		FYapBit& Bit = GetFragmentMutable().GetChildSafeBitMutable();

		FText Title = LOCTEXT("ChildSafeDataEditor_Title", "CHILD-SAFE DIALOGUE");
		FText DialogueTextHint = LOCTEXT("DialogueTextEntryBox_Hint", "Dialogue text (child-safe)...");
		FText TitleTextHint = LOCTEXT("DialogueTextEntryBox_Hint", "Title text (child-safe)...");

		FMargin Padding(4, 4, 0, 4);
				
		Splitter->AddSlot()
		.Resizable(false)
		[
			BuildDialogueEditor_SingleSide(Title, DialogueTextHint, TitleTextHint, Width, Padding, Bit)
		];
	};

	return Splitter;
}

TSharedRef<SWidget> SFlowGraphNode_YapFragmentWidget::BuildDialogueEditor_SingleSide(const FText& Title, const FText& DialogueTextHint, const FText& TitleTextHint, float Width, FMargin Padding, FYapBit& Bit)
{
	FYapText& DialogueText = Bit.DialogueText;
	FYapText& TitleText = Bit.TitleText;
		
	TSoftObjectPtr<UObject>& AudioAsset = Bit.AudioAsset;
	FString& StageDirections = Bit.StageDirections;

	FString& DialogueLocalizationComments = Bit.DialogueLocalizationComments;
	FString& TitleTextLocalizationComments = Bit.TitleTextLocalizationComments;

	TSharedRef<IEditableTextProperty> DialogueTextProperty = MakeShareable(new FYapEditableTextPropertyHandle(DialogueText, Owner->GetFlowGraphNode_YapDialogueMutable()));
	TSharedRef<IEditableTextProperty> TitleTextProperty = MakeShareable(new FYapEditableTextPropertyHandle(TitleText, Owner->GetFlowGraphNode_YapDialogueMutable()));

	auto DialogueCommentAttribute = TAttribute<FString>::CreateLambda( [DialogueLocalizationComments] () { return *DialogueLocalizationComments; });
	FText DialogueCommentText = LOCTEXT("POComment_HintText", "Comments for translators...");
	
	auto TitleTextCommentAttribute = TAttribute<FString>::CreateLambda( [TitleTextLocalizationComments] () { return *TitleTextLocalizationComments; });
	FText TitleTextCommentText = LOCTEXT("POComment_HintText", "Comments for translators...");
	
	auto StageDirectionsAttribute = TAttribute<FString>::CreateLambda( [StageDirections] () { return *StageDirections; });
	FText StageDirectionsText = LOCTEXT("StageDirections_HintText", "Stage directions...");
	
	return SNew(SBox)
	.WidthOverride(Width)
	.Padding(Padding)
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(STextBlock)
			.Text(Title)
			.Font(YapFonts.Font_SectionHeader)
			.Justification(ETextJustify::Center)	
		]
		+ SVerticalBox::Slot()
		.FillHeight(1.0)
		.Padding(0, 6, 0, 0)
		[
			SNew(SBox)
			.HeightOverride(66) // Fits ~4 lines of text
			.VAlign(VAlign_Fill)
			[
				SNew(SYapTextPropertyEditableTextBox, DialogueTextProperty)
				.Style(FYapEditorStyle::Get(), YapStyles.EditableTextBoxStyle_Dialogue)
				.Owner(GetDialogueNode())
				.HintText(DialogueTextHint)
				.Font(YapFonts.Font_DialogueText)
				.ForegroundColor(YapColor::White)
				.Cursor(EMouseCursor::Default)
				.MaxDesiredHeight(66)
			]
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 2, 0, 0)
		[
			SNew(SBox)
			.Visibility(EVisibility::Visible) // TODO project settings to disable localization metadata
			.Padding(0, 0, 28, 0)
			[
					BuildCommentEditor(DialogueCommentAttribute, &DialogueLocalizationComments, LOCTEXT("POComment_HintText", "Comments for translators..."))
			]
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 8, 0, 0)
		[
			SNew(SVerticalBox)
			.Visibility(GetDialogueNode()->UsesTitleText() ? EVisibility::Visible : EVisibility::Collapsed)
			+ SVerticalBox::Slot()
			[
				SNew(SYapTextPropertyEditableTextBox, TitleTextProperty)
				.Style(FYapEditorStyle::Get(), YapStyles.EditableTextBoxStyle_TitleText)
				.Owner(GetDialogueNode())
				.HintText(TitleTextHint)
				.ForegroundColor(YapColor::YellowGray)
				.Cursor(EMouseCursor::Default)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 2, 0, 0)
			[
				SNew(SBox)
				.Visibility(EVisibility::Visible) // TODO project settings to disable localization metadata
				.Padding(0, 0, 28, 0)
				[
					BuildCommentEditor(TitleTextCommentAttribute, &TitleTextLocalizationComments, LOCTEXT("POComment_HintText", "Comments for translators..."))
				]
			]
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 8, 0, 0)
		[
			SNew(SBox)
			.Visibility(EVisibility::Visible) // TODO project settings to disable audio
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.HAlign(HAlign_Center)
				.Padding(0, 0, 0, 0)
				[							
					SNew(STextBlock)
					.Text(LOCTEXT("AudioAssetPicker_Title", "Audio Asset"))
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0, 2, 0, 0)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					[
						SNew(SBox)
						[
							CreateAudioAssetWidget(AudioAsset)
						]
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.HAlign(HAlign_Right)
					.VAlign(VAlign_Center)
					[
						CreateAudioPreviewWidget(&AudioAsset, EVisibility::Visible)
					]
				]
				+ SVerticalBox::Slot()
				.Padding(0, 2, 28, 0)
				.AutoHeight()
				[
					BuildCommentEditor(StageDirectionsAttribute, &StageDirections, LOCTEXT("StageDirections_HintText", "Stage directions..."))
				]
			]
		]
	];
}

TSharedRef<SWidget> SFlowGraphNode_YapFragmentWidget::BuildCommentEditor(TAttribute<FString> String, FString* StringProperty, FText HintText)
{
	UFlowNode_YapDialogue* DialogueNode = GetDialogueNode();
	
	return SNew(SBorder)
	.BorderImage(FYapEditorStyle::GetImageBrush(YapBrushes.Box_SolidWhite))
	.BorderBackgroundColor(YapColor::DeepGray_SemiGlass)
	.Visibility_Lambda( [this] () { return EVisibility::Visible; } ) // TODO project developer settings to show/hide this
	[
		SNew(SMultiLineEditableText)
		.HintText(HintText)
		.ClearKeyboardFocusOnCommit(false)
		.Text_Lambda( [String] () { return FText::FromString(String.Get()); } )
		.OnTextCommitted_Lambda( [StringProperty, DialogueNode] (const FText& NewText, ETextCommit::Type CommitType)
		{
			if (CommitType != ETextCommit::OnCleared)
			{
				FYapScopedTransaction Transaction("TODO", LOCTEXT("TransactionText_ChangeComment", "Change comment"), DialogueNode);
				*StringProperty = NewText.ToString();
			}
		})
	];
}

TSharedRef<SWidget> SFlowGraphNode_YapFragmentWidget::BuildTimeSettings_ExpandedEditor(float Width)
{
	bool bMatureOnly = !NeedsChildSafeData();

	TSharedRef<SSplitter> Splitter = SNew(SSplitter)
		.Orientation(Orient_Horizontal)
		.PhysicalSplitterHandleSize(2.0f);

	{
		Splitter->AddSlot()
		.Resizable(false)
		[
			BuildTimeSettings_SingleSide(Width, FMargin(4, 4, 28, 4), EYapMaturitySetting::Mature)
		];
	}
	
	if (!bMatureOnly)
	{
		Splitter->AddSlot()
		.Resizable(false)
		[
			BuildTimeSettings_SingleSide(Width, FMargin(4, 4, 28, 4), EYapMaturitySetting::ChildSafe)
		];
	};

	return Splitter;
}

TSharedRef<SWidget> SFlowGraphNode_YapFragmentWidget::BuildTimeSettings_SingleSide(float Width, FMargin Padding, EYapMaturitySetting MaturitySetting)
{
	return SNew(SBox)
	.WidthOverride(Width)
	.Padding(Padding)
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.Padding(0, 0, 0, 2)
		.AutoHeight()
		[
			MakeTimeSettingRow(EYapTimeMode::Default, MaturitySetting)
		]
		+ SVerticalBox::Slot()
		.Padding(0, 2, 0, 2)
		.AutoHeight()
		[
			MakeTimeSettingRow(EYapTimeMode::AudioTime, MaturitySetting)
		]
		+ SVerticalBox::Slot()
		.Padding(0, 2, 0, 2)
		.AutoHeight()
		[
			MakeTimeSettingRow(EYapTimeMode::TextTime, MaturitySetting)
		]
		+ SVerticalBox::Slot()
		.Padding(0, 2, 0, 0)
		.AutoHeight()
		[
			MakeTimeSettingRow(EYapTimeMode::ManualTime, MaturitySetting)
		]
	];
}

TSharedRef<SWidget> SFlowGraphNode_YapFragmentWidget::BuildPaddingSettings_ExpandedEditor(float Width)
{
	bool bMatureOnly = !NeedsChildSafeData();

	TSharedRef<SSplitter> Splitter = SNew (SSplitter)
		.Orientation(Orient_Horizontal)
		.PhysicalSplitterHandleSize(2.0f);

	{
		Splitter->AddSlot()
		.Resizable(false)
		[
			SNew(SBox)
			.Padding(4, 4, 28, 4)
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
						if (GetFragmentMutable().PaddingToNextFragment < 0)
						{
							float CurrentPadding_Default = GetFragmentMutable().GetPaddingToNextFragment();
							GetFragmentMutable().SetPaddingToNextFragment(CurrentPadding_Default);
						}
						else
						{
							GetFragmentMutable().SetPaddingToNextFragment(-1);
						}
						return FReply::Handled();
					})// TODO transactions
					.ButtonColorAndOpacity(this, &ThisClass::ButtonColorAndOpacity_PaddingButton) // TODO coloring
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
						.Value_Lambda( [this] () { return GetFragmentMutable().GetPaddingToNextFragment(); } )
						.OnValueChanged_Lambda( [this] (float NewValue) { GetFragmentMutable().SetPaddingToNextFragment(NewValue); } )
						.OnValueCommitted_Lambda( [this] (float NewValue, ETextCommit::Type) { GetFragmentMutable().SetPaddingToNextFragment(NewValue); } ) // TODO transactions
					]
				]
			]
		];
	}

	if (!bMatureOnly)
	{
		Splitter->AddSlot()
		.Resizable(false)
		[
			SNew(SBox)
			.WidthOverride(Width)
		];
	}

	return Splitter;
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

	EYapTimeMode TimeMode = GetFragment().GetTimeMode(GetDisplayMaturitySetting());
	const FYapBit& Bit = GetFragment().GetBit(GetDisplayMaturitySetting());
	
	const TOptional<float> FragmentTimeIn = Bit.GetTime(TimeMode);

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
			if (GetDialogueNode()->FragmentStartedTime >= GetDialogueNode()->FragmentEndedTime)
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
			GetFragmentMutable().SetPaddingToNextFragment(-1);
			return;
		}
	}
	
	GetFragmentMutable().SetPaddingToNextFragment(NewValue);
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
	const TSoftObjectPtr<UYapCharacter> SpeakerAsset = GetFragment().GetSpeakerAsset();
	
	FLinearColor Color;
	
	if (SpeakerAsset.IsValid())
	{
		Color = SpeakerAsset.Get()->GetEntityColor();
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

	GetFragmentMutable().SetSpeaker(AssetData.GetAsset());

	FYapTransactions::EndModify();
}

void SFlowGraphNode_YapFragmentWidget::OnSetNewDirectedAtAsset(const FAssetData& AssetData)
{
	FYapTransactions::BeginModify(LOCTEXT("SetDirectedAtCharacter", "Set directed-at character"), GetDialogueNode());

	GetFragmentMutable().SetDirectedAt(AssetData.GetAsset());

	FYapTransactions::EndModify();
}

TSharedRef<SWidget> SFlowGraphNode_YapFragmentWidget::PopupContentGetter_SpeakerWidget(const UYapCharacter* Character)
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
			.OnSet(this, &ThisClass::OnSetNewSpeakerAsset)
		]
	];
}

FText SFlowGraphNode_YapFragmentWidget::Text_SpeakerWidget() const
{
	TSoftObjectPtr<UYapCharacter> SpeakerAsset = GetFragment().GetSpeakerAsset();	

	if (SpeakerAsset.IsNull())
	{
		return LOCTEXT("SpeakerUnset_Label","Speaker\nUnset");
	}
	
	if (Image_SpeakerImage() == nullptr)
	{
		TSharedPtr<FGameplayTagNode> GTN = UGameplayTagsManager::Get().FindTagNode(GetFragment().GetMoodTag());
		
		FText CharacterName = SpeakerAsset.IsValid() ? SpeakerAsset.Get()->GetEntityName() : LOCTEXT("Unloaded", "Unloaded");
		
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
	TSoftObjectPtr<UYapCharacter> SpeakerAsset = GetFragment().GetSpeakerAsset();	

	if (SpeakerAsset.IsNull())
	{
		return LOCTEXT("SpeakerUnset_Label","Speaker Unset");
	}
	
	TSharedPtr<FGameplayTagNode> GTN = UGameplayTagsManager::Get().FindTagNode(GetFragment().GetMoodTag());
	
	FText CharacterName = SpeakerAsset.IsValid() ? SpeakerAsset.Get()->GetEntityName() : LOCTEXT("Unloaded", "Unloaded");
	
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

	GetFragmentMutable().SetSpeaker(Character);

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

	GetFragmentMutable().GetMatureBitMutable().SetDialogueAudioAsset(Object);

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

	const UYapCharacter* Character = GetFragmentMutable().GetSpeaker();
	
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
			.OnAreAssetsAcceptableForDrop(this, &ThisClass::OnAreAssetsAcceptableForDrop_SpeakerWidget)
			.OnAssetsDropped(this, &ThisClass::OnAssetsDropped_SpeakerWidget)
			[
				SNew(SYapButtonPopup)
				.ButtonForegroundColor(this, &ThisClass::BorderBackgroundColor_CharacterImage)
				.PopupPlacement(MenuPlacement_BelowAnchor)
				.PopupContentGetter(FPopupContentGetter::CreateSP(this, &ThisClass::PopupContentGetter_SpeakerWidget, Character))
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
							.Image(this, &ThisClass::Image_SpeakerImage)
							.ToolTipText(this, &ThisClass::ToolTipText_SpeakerWidget)
						]
						+ SOverlay::Slot()
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Center)
						[
							SNew(STextBlock)
							.Text(this, &ThisClass::Text_SpeakerWidget)
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
			//.Image(this, &ThisClass::Image_SpeakerImage)
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
	const UYapCharacter* Speaker = GetFragmentMutable().GetSpeaker();
	const FGameplayTag& MoodTag = GetFragment().GetMoodTag();
	
	TSharedPtr<FSlateImageBrush> PortraitBrush = UYapEditorSubsystem::GetCharacterPortraitBrush(Speaker, MoodTag);

	if (PortraitBrush && PortraitBrush->GetResourceObject())
	{
		return PortraitBrush.Get();
	}
	else
	{
		return FYapEditorStyle::GetImageBrush(YapBrushes.None);
	}
}

EVisibility SFlowGraphNode_YapFragmentWidget::Visibility_MissingPortraitWarning() const
{
	const UYapCharacter* Speaker = GetFragmentMutable().GetSpeaker();
	const FGameplayTag& MoodTag = GetFragment().GetMoodTag();
	
	const TSharedPtr<FSlateImageBrush> Brush = UYapEditorSubsystem::GetCharacterPortraitBrush(Speaker, MoodTag);

	if (!Brush.IsValid())
	{
		return EVisibility::Visible;
	}

	return (Brush->GetResourceObject()) ? EVisibility::Hidden : EVisibility::Visible;
}

EVisibility SFlowGraphNode_YapFragmentWidget::Visibility_CharacterSelect() const
{
	return IsHovered() ? EVisibility::Visible : EVisibility::Collapsed;
}

FString SFlowGraphNode_YapFragmentWidget::ObjectPath_CharacterSelect() const
{	
	const TSoftObjectPtr<UYapCharacter> Asset = GetFragment().GetSpeakerAsset();

	if (Asset.IsPending())
	{
		(void)Asset.LoadSynchronous();
	}
	
	if (!Asset) { return ""; }

	return Asset.ToString();
}

FText SFlowGraphNode_YapFragmentWidget::ToolTipText_MoodTagSelector() const
{
	TSharedPtr<FGameplayTagNode> TagNode = UGameplayTagsManager::Get().FindTagNode(GetFragment().GetMoodTag());

	if (TagNode.IsValid())
	{
		return FText::FromName(TagNode->GetSimpleTagName());
	}

	return LOCTEXT("Default", "Default");
}

FSlateColor SFlowGraphNode_YapFragmentWidget::ForegroundColor_MoodTagSelectorWidget() const
{
	if (GetFragment().GetMoodTag() == FGameplayTag::EmptyTag)
	{
		return YapColor::Button_Unset();
	}
	
	return YapColor::White_Trans;
}

// ================================================================================================
// TITLE TEXT WIDGET
// ------------------------------------------------------------------------------------------------

TSharedRef<SWidget> SFlowGraphNode_YapFragmentWidget::CreateTitleTextDisplayWidget()
{
	const FText* MatureTitleText = &GetFragment().GetMatureBit().GetTitleText();
	const FText* SafeTitleText = &GetFragment().GetChildSafeBit().GetTitleText();
	
	return SNew(SBorder)
	.Cursor(EMouseCursor::Default)
	.BorderImage(FYapEditorStyle::GetImageBrush(YapBrushes.Box_SolidWhite))
	.BorderBackgroundColor(FSlateColor::UseForeground())
	.ToolTipText(LOCTEXT("TitleText_ToolTip", "Title text"))
	.Padding(0)
	[
		SNew(SOverlay)
		+ SOverlay::Slot()
		.Padding(4, 0)
		[
			SNew(STextBlock)
			.TextStyle(FYapEditorStyle::Get(), YapStyles.TextBlockStyle_TitleText)
			.Text_Lambda( [MatureTitleText] () { return *MatureTitleText; } )
			.ToolTipText(this, &ThisClass::ToolTipText_TextDisplayWidget, LOCTEXT("TitleText_ToolTip", "Title text"), MatureTitleText, SafeTitleText)
			.ColorAndOpacity(this, &ThisClass::ColorAndOpacity_TextDisplayWidget, YapColor::YellowGray, MatureTitleText, SafeTitleText)
		]
		+ SOverlay::Slot()
		.VAlign(VAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Visibility_Lambda( [this] ()
			{
				const FText& Text = GetFragment().GetBit(GetDisplayMaturitySetting()).GetTitleText();
				return Text.IsEmpty() ? EVisibility::HitTestInvisible : EVisibility::Hidden;;
			})
			.Justification(ETextJustify::Center)
			.TextStyle(FYapEditorStyle::Get(), YapStyles.TextBlockStyle_TitleText)
			.Text_Lambda( [this] ()
			{
				if (!NeedsChildSafeData())
				{
					return LOCTEXT("TitleText_None", "Title Text (None)");
				}
							
				return GetDisplayMaturitySetting() == EYapMaturitySetting::Mature ? LOCTEXT("MatureTitleText_None", "Mature Title Text (None)") : LOCTEXT("SafeTitleText_None", "Child-Safe Title Text (None)");	
			})
			.ColorAndOpacity(YapColor::White_Glass)
		]
		+ SOverlay::Slot()
		.VAlign(VAlign_Fill)
		.HAlign(HAlign_Fill)
		.Padding(-1)
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("MarqueeSelection"))
			.Visibility(this, &ThisClass::Visibility_TitleTextErrorState)
			.BorderBackgroundColor(YapColor::Red)
		]
	];
}
 
FText SFlowGraphNode_YapFragmentWidget::Text_EditedText(FText* Text) const
{
	return *Text;
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

	if (GetDisplayMaturitySetting() == EYapMaturitySetting::ChildSafe)
	{
		Color *= YapColor::LightBlue;
	}
	
	return Color;
}

EVisibility SFlowGraphNode_YapFragmentWidget::Visibility_TitleTextErrorState() const
{
	if (!NeedsChildSafeData())
	{
		return EVisibility::Collapsed;
	}

	const FYapBit& MatureBit = GetFragment().GetMatureBit();
	const FYapBit& ChildSafeBit = GetFragment().GetChildSafeBit();

	if (MatureBit.HasTitleText() != ChildSafeBit.HasTitleText())
	{
		return EVisibility::Visible;
	}

	return EVisibility::Collapsed;
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
	auto TagAttribute = TAttribute<FGameplayTag>::CreateSP(this, &ThisClass::Value_FragmentTag);
	FString FilterString = GetDialogueNode()->GetDialogueTag().ToString();
	auto OnTagChanged = TDelegate<void(const FGameplayTag)>::CreateSP(this, &ThisClass::OnTagChanged_FragmentTag);

	return SNew(SYapGameplayTagTypedPicker)
		.Tag(TagAttribute)
		.Filter(FilterString)
		.OnTagChanged(OnTagChanged)
		.ToolTipText(LOCTEXT("FragmentTag_ToolTip", "Fragment tag"))
		.Asset(GetDialogueNode()->GetFlowAsset());
}

FGameplayTag SFlowGraphNode_YapFragmentWidget::Value_FragmentTag() const
{
	return GetFragment().FragmentTag;
}

void SFlowGraphNode_YapFragmentWidget::OnTagChanged_FragmentTag(FGameplayTag GameplayTag)
{
	FYapTransactions::BeginModify(LOCTEXT("ChangeFragmentTag", "Change fragment tag"), GetDialogueNode());

	GetFragmentMutable().FragmentTag = GameplayTag;

	FYapTransactions::EndModify();

	Owner->RequestUpdateGraphNode();
}

FReply SFlowGraphNode_YapFragmentWidget::OnClicked_SetTimeModeButton(EYapTimeMode TimeMode)
{
	FYapTransactions::BeginModify(LOCTEXT("TimeModeChanged", "Time mode changed"), GetDialogueNode());

	GetFragmentMutable().SetTimeModeSetting(TimeMode);

	FYapTransactions::EndModify();

	return FReply::Handled();
}

void SFlowGraphNode_YapFragmentWidget::OnValueUpdated_ManualTime(float NewValue, EYapMaturitySetting MaturitySetting)
{
	GetFragmentMutable().GetBitMutable(MaturitySetting).SetManualTime(NewValue);
	GetFragmentMutable().SetTimeModeSetting(EYapTimeMode::ManualTime);
}

// ---------------------


FSlateColor SFlowGraphNode_YapFragmentWidget::ButtonColorAndOpacity_UseTimeMode(EYapTimeMode TimeMode, FLinearColor ColorTint, EYapMaturitySetting MaturitySetting) const
{
	if (GetFragment().GetTimeModeSetting() == TimeMode)
	{
		// Exact setting match
		return ColorTint;
	}
	
	if (GetFragment().GetTimeMode(GetDisplayMaturitySetting()) == TimeMode)
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
		return YapColor::Green;
	}
	
	return YapColor::DarkGray;
}

FSlateColor SFlowGraphNode_YapFragmentWidget::ForegroundColor_TimeSettingButton(EYapTimeMode TimeMode, FLinearColor ColorTint) const
{
	if (GetFragment().GetTimeModeSetting() == TimeMode)
	{
		// Exact setting match
		return ColorTint;
	}
	
	if (GetFragment().GetTimeMode(GetDisplayMaturitySetting()) == TimeMode)
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
	EYapMaturitySetting Type = (&Asset == &GetFragment().GetChildSafeBit().AudioAsset) ? EYapMaturitySetting::ChildSafe : EYapMaturitySetting::Mature; 
	
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
			GetFragmentMutable().GetMatureBitMutable().SetDialogueAudioAsset(InAssetData.GetAsset());
		}
		else
		{
			GetFragmentMutable().GetChildSafeBitMutable().SetDialogueAudioAsset(InAssetData.GetAsset());
		}
		
		FYapTransactions::EndModify();
	});
	
	TSharedRef<SObjectPropertyEntryBox> AudioAssetProperty = SNew(SObjectPropertyEntryBox)
		.IsEnabled(bFoundAssetClass)
		.AllowedClass(bSingleDialogueAssetClass ? DialogueAssetClass : UObject::StaticClass()) // Use this feature if the project only has one dialogue asset class type
		.DisplayBrowse(true)
		.DisplayUseSelected(true)
		.DisplayThumbnail(true)
		.OnShouldFilterAsset(this, &ThisClass::OnShouldFilterAsset_AudioAssetWidget)
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
		.Visibility(this, &ThisClass::Visibility_AudioAssetErrorState, &Asset)
		.ColorAndOpacity(this, &ThisClass::ColorAndOpacity_AudioAssetErrorState, &Asset)
	];
	
	return Widget;
}

FText SFlowGraphNode_YapFragmentWidget::ObjectPathText_AudioAsset() const
{
	return FText::FromString(ObjectPath_AudioAsset());
}

FString SFlowGraphNode_YapFragmentWidget::ObjectPath_AudioAsset() const
{
	const UObject* Asset = GetFragment().GetBit(GetDisplayMaturitySetting()).GetAudioAsset<UObject>();

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
		case EYapErrorLevel::Unknown:
		{
			Color = YapColor::Error;
			break;
		}
	}
	
	return Color;
}

// TODO handle child safe settings somehow
EYapErrorLevel SFlowGraphNode_YapFragmentWidget::GetFragmentAudioErrorLevel() const
{
	const TSoftObjectPtr<UObject>& MatureAsset = GetFragment().GetMatureBit().GetAudioAsset<UObject>();
	const TSoftObjectPtr<UObject>& SafeAsset = GetFragment().GetChildSafeBit().GetAudioAsset<UObject>();

	// If we need child-safe data and if any audio is set, we need both set
	if (NeedsChildSafeData())
	{
		if ((!MatureAsset.IsNull() || !SafeAsset.IsNull()) && (MatureAsset.IsNull() || SafeAsset.IsNull()))
		{
			return EYapErrorLevel::Error;
		}
	}

	// We should never allow dialogue to only contain child-safe content; child-safe content is the special case
	if (MatureAsset.IsNull() && !SafeAsset.IsNull())
	{
		return EYapErrorLevel::Unknown;
	}
	
	// Determine the error level based on project settings (does the project want all fragments to have audio?)
	EYapErrorLevel ErrorLevel = GetAudioAssetErrorLevel(MatureAsset);

	return ErrorLevel;
}

FSlateColor SFlowGraphNode_YapFragmentWidget::ColorAndOpacity_AudioAssetErrorState(const TSoftObjectPtr<UObject>* Asset) const
{
	EYapErrorLevel ErrorLevel = GetAudioAssetErrorLevel(*Asset);

	switch (ErrorLevel)
	{
		case EYapErrorLevel::OK:
		{
			return YapColor::DarkGray;
		}
		case EYapErrorLevel::Warning:
		{
			return YapColor::Orange;
		}
		case EYapErrorLevel::Error:
		{
			return YapColor::Red;
		}
		case EYapErrorLevel::Unknown:
		{
			return YapColor::Error; 
		}
		default:
		{
			return YapColor::Noir;
		}
	}
}

EYapErrorLevel SFlowGraphNode_YapFragmentWidget::GetAudioAssetErrorLevel(const TSoftObjectPtr<UObject>& Asset) const
{
	// If asset isn't loaded, it's a mystery!!!!
	if (Asset.IsPending())
	{
		return EYapErrorLevel::Unknown;
	}
	
	// If we have an asset all we need to do is check if it's the correct class type. Always return an error if it's an improper type.
	if (Asset.IsValid())
	{
		const TArray<TSoftClassPtr<UObject>>& AllowedAssetClasses = UYapProjectSettings::GetAudioAssetClasses();

		if (AllowedAssetClasses.ContainsByPredicate( [Asset] (const TSoftClassPtr<UObject>& Class)
		{
			if (Class.IsPending())
			{
				UE_LOG(LogYapEditor, Warning, TEXT("Synchronously loading audio asset class"));
			}
			
			return Asset->IsA(Class.LoadSynchronous()); // TODO verify this works?
		}))
		{
			return EYapErrorLevel::OK;
		}
		else
		{
			return EYapErrorLevel::Error;
		}
	}

	EYapMissingAudioErrorLevel MissingAudioBehavior = UYapProjectSettings::GetMissingAudioBehavior();

	EYapTimeMode TimeModeSetting = GetFragment().GetTimeModeSetting();
	
	// We don't have any audio asset set. If the dialogue is set to use audio time but does NOT have an audio asset, we either indicate an error (prevent packaging) or indicate a warning (allow packaging) 
	if ((TimeModeSetting == EYapTimeMode::AudioTime) || (TimeModeSetting == EYapTimeMode::Default && UYapProjectSettings::GetDefaultTimeModeSetting() == EYapTimeMode::AudioTime))
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

FYapFragment& SFlowGraphNode_YapFragmentWidget::GetFragmentMutable()
{
	return const_cast<FYapFragment&>(const_cast<const SFlowGraphNode_YapFragmentWidget*>(this)->GetFragment());
}

FYapFragment& SFlowGraphNode_YapFragmentWidget::GetFragmentMutable() const
{
	return const_cast<FYapFragment&>(GetFragment());
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

	bChildSafeCheckBoxHovered = ChildSafeCheckBox->IsHovered();
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
				.Visibility(this, &ThisClass::Visibility_EnableOnStartPinButton)
				[
					SNew(SButton)
					.Cursor(EMouseCursor::Default)
					.OnClicked(this, &ThisClass::OnClicked_EnableOnStartPinButton)
					.ButtonColorAndOpacity(YapColor::Gray_Trans)
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
				.Visibility(this, &ThisClass::Visibility_EnableOnEndPinButton)
				[
					SNew(SButton)
					.Cursor(EMouseCursor::Default)
					.OnClicked(this, &ThisClass::OnClicked_EnableOnEndPinButton)
					.ButtonColorAndOpacity(YapColor::Gray_Trans)
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
	if (Pin == GetFragmentMutable().GetStartPin())
	{
		return StartPinBox;
	}

	if (Pin == GetFragmentMutable().GetEndPin())
	{
		return EndPinBox;
	}

	if (Pin == GetFragmentMutable().GetPromptPin())
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

	GetFragmentMutable().bShowOnStartPin = true;
	
	GetDialogueNode()->ForceReconstruction();
	
	FYapTransactions::EndModify();

	return FReply::Handled();
}

FReply SFlowGraphNode_YapFragmentWidget::OnClicked_EnableOnEndPinButton()
{
	FYapTransactions::BeginModify(LOCTEXT("YapDialogue", "Enable OnEnd Pin"), GetDialogueNode());

	GetFragmentMutable().bShowOnEndPin = true;

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
		.ButtonStyle(FYapEditorStyle::Get(), YapStyles.ButtonStyle_DialogueCornerFoldout) // TODO fix style
		.ContentPadding(FMargin(0.f, 0.f))
		.MenuPlacement(MenuPlacement_CenteredBelowAnchor)
		//.ButtonColorAndOpacity(FSlateColor(FLinearColor(0.f, 0.f, 0.f, 0.5f)))
		.HAlign(HAlign_Center)
		.ButtonStyle(FAppStyle::Get(), "SimpleButton")
		.OnMenuOpenChanged(this, &ThisClass::OnMenuOpenChanged_MoodTagSelector)
		.ToolTipText(this, &ThisClass::ToolTipText_MoodTagSelector)
		.ForegroundColor(this, &ThisClass::ForegroundColor_MoodTagSelectorWidget)
		.ButtonContent()
		[
			SNew(SImage)
			.ColorAndOpacity(FSlateColor::UseForeground())
			.Image(this, &ThisClass::Image_MoodTagSelector)
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
	return GetFragment().GetMoodTag();
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
	.OnClicked(this, &ThisClass::OnClicked_MoodTagMenuEntry, MoodTag)
	.ToolTipText(ToolTipText)
	[
		SNew(SOverlay)
		+ SOverlay::Slot()
		.Padding(-3)
		[
			SNew(SBorder)
			.Visibility_Lambda([this, MoodTag]()
			{
				if (GetFragmentMutable().GetMoodTag() == MoodTag)
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

	GetFragmentMutable().SetMoodTag(NewValue);

	FYapTransactions::EndModify();

	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE