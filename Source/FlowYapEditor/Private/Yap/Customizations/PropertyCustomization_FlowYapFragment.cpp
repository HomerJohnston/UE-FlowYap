#include "Yap/Customizations/PropertyCustomization_FlowYapFragment.h"

#include "DetailWidgetRow.h"
#include "PropertyCustomizationHelpers.h"
#include "Slate/DeferredCleanupSlateBrush.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "Widgets/Input/SSlider.h"
#include "Widgets/Notifications/SProgressBar.h"
#include "Yap/FlowYapCharacter.h"
#include "Yap/FlowYapColors.h"
#include "Yap/FlowYapEditorSubsystem.h"
#include "Yap/FlowYapProjectSettings.h"
#include "Yap/FlowYapTransactions.h"
#include "Yap/FlowYapUtil.h"
#include "Yap/YapEditorStyle.h"
#include "Yap/Enums/FlowYapErrorLevel.h"
#include "Yap/Nodes/FlowNode_YapDialogue.h"
#include "Yap/SlateWidgets/SGameplayTagComboFiltered.h"

#define LOCTEXT_NAMESPACE "FlowYap"


void FPropertyCustomization_FlowYapFragment::CustomizeHeader(TSharedRef<IPropertyHandle> InPropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	Init(InPropertyHandle);
	
	HeaderRow.WholeRowContent()
	[
		//SNew(FPropertyCustomization_FlowYapFragment, PropertyHandle.ToSharedPtr(), Dialogue)
		
		SNew(SBox)
		.WidthOverride(this, &FPropertyCustomization_FlowYapFragment::Fragment_WidthOverride)
		[
			SNew(SVerticalBox)
			// ===================
			// TOP SECTION
			// ===================
			+ SVerticalBox::Slot()
			.Padding(0, 0, 0, 0)
			.AutoHeight()
			[
				SNew(SHorizontalBox)
				// -------------------
				// DIALOGUE
				// -------------------
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				.FillWidth(1.0f)
				.Padding(0, 0, 0, 0)
				[
					SNew(SOverlay)
					+ SOverlay::Slot()
					[
						CreateDialogueWidget()
					]
					+ SOverlay::Slot()
					.HAlign(HAlign_Right)
					.VAlign(VAlign_Bottom)
					.Padding(4, 4, 4, 4)
					[
						CreateFragmentTagPreviewWidget()
					]
					+ SOverlay::Slot()
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Bottom)
					.Padding(0, 0, 0, -2)
					[
						CreateFragmentTimePaddingWidget()
					]
					+ SOverlay::Slot()
					.HAlign(HAlign_Right)
					.VAlign(VAlign_Top)
					.Padding(0, -4, -4, 0)
					[
						CreateActivationLimiterWidget()
					]
				]
				// -------------------
				// PORTRAIT
				// -------------------
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Top)
				.AutoWidth()
				.Padding(0, 0, 0, 0)
				[
					// TODO this function eats up almost 2ms... optimize it 
					CreatePortraitWidget()
				]
			]
			// ===================
			// BOTTOM SECTION
			// ===================
			+ SVerticalBox::Slot()
			.AutoHeight()
			//.HAlign(HAlign_Fill)
			.Padding(0, 0, 0, 0)
			[
				SNew(SVerticalBox)
				.Visibility(this, &FPropertyCustomization_FlowYapFragment::FragmentBottomSection_Visibility)
				// -------------------
				// MIDDLE ROW
				// -------------------
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Fill)
				.Padding(0, 4, -1, 0)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.HAlign(HAlign_Fill)
					.Padding(0, 0, 2, 0)
					[
						CreateTitleTextWidget()
					]
					+ SHorizontalBox::Slot()
					.HAlign(HAlign_Right)
					.Padding(2, 0, 0, 0)
					.AutoWidth()
					[
						CreateFragmentTagWidget()
					]
				]
				// -------------------
				// BOTTOM ROW
				// -------------------
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0, 2, -1, 0)
				[
					// TODO this whole function call takes 2 ms... optimize somehow?
					CreateBottomRowWidget()
				]
			]
		]
	];
}

void FPropertyCustomization_FlowYapFragment::Init(TSharedRef<IPropertyHandle> InPropertyHandle)
{
	FragmentHandle = InPropertyHandle;
	
	TSharedPtr<IPropertyHandle> ParentHandle = FragmentHandle->GetParentHandle();

	void* Data;
	ParentHandle->GetValueData(Data);

	Dialogue = static_cast<UFlowNode_YapDialogue*>(Data);

	BitHandle = FragmentHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FFlowYapFragment, Bit));
	Bit_CharacterHandle = BitHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FFlowYapBit, Character));
	Bit_TitleTextHandle = BitHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FFlowYapBit, TitleText));
	Bit_DialogueTextHandle = BitHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FFlowYapBit, DialogueText));
	Bit_DialogueAudioAssetHandle = BitHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FFlowYapBit, DialogueAudioAsset));
	Bit_MoodKeyHandle = BitHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FFlowYapBit, MoodKey));
	Bit_bUseProjectDefaultTimeSettingsHandle = BitHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FFlowYapBit, bUseProjectDefaultTimeSettings));
	Bit_TimeModeHandle = BitHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FFlowYapBit, TimeMode));
	Bit_InterruptibleHandle = BitHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FFlowYapBit, Interruptible));
	Bit_ManualTimeHandle = BitHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FFlowYapBit, ManualTime));
	Bit_CachedWordCountHandle = BitHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FFlowYapBit, CachedWordCount));
	Bit_CachedAudioTimeHandle = BitHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FFlowYapBit, CachedAudioTime));
	ActivationLimitHandle = FragmentHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FFlowYapFragment, ActivationLimit));
	FragmentTagHandle = FragmentHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FFlowYapFragment, FragmentTag));
	PaddingToNextFragmentHandle = FragmentHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FFlowYapFragment, PaddingToNextFragment));
	CommonPaddingSettingHandle = FragmentHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FFlowYapFragment, CommonPaddingSetting));
}


FOptionalSize FPropertyCustomization_FlowYapFragment::Fragment_WidthOverride() const
{
	return 360 + UFlowYapProjectSettings::Get()->GetDialogueWidthAdjustment();
}

EVisibility FPropertyCustomization_FlowYapFragment::FragmentBottomSection_Visibility() const
{
	return EVisibility::Visible;
}

// ================================================================================================
// DIALOGUE WIDGET
// ------------------------------------------------------------------------------------------------

TSharedRef<SBox> FPropertyCustomization_FlowYapFragment::CreateDialogueWidget()
{
	TSharedRef<SScrollBar> HScrollBar = SNew(SScrollBar)
	.Orientation(Orient_Horizontal)
	.Thickness(this, &FPropertyCustomization_FlowYapFragment::DialogueScrollBar_Thickness)
	.Padding(-1);
	
	TSharedRef<SScrollBar> VScrollBar = SNew(SScrollBar)
	.Orientation(Orient_Vertical)
	.Thickness(this, &FPropertyCustomization_FlowYapFragment::DialogueScrollBar_Thickness)
	.Padding(-1);
	
	return SNew(SBox)
	.MinDesiredHeight(58) // This is the normal height of the full portrait widget
	.MaxDesiredHeight(this, &FPropertyCustomization_FlowYapFragment::Dialogue_MaxDesiredHeight)
	.Padding(0, 0, 0, 0)
	[
		SNew(SOverlay)
		+ SOverlay::Slot()
		[
			SAssignNew(DialogueBox, SMultiLineEditableTextBox)
			.Text(this, &FPropertyCustomization_FlowYapFragment::Dialogue_Text)
			.ModiferKeyForNewLine(EModifierKey::Shift)
			.OnTextCommitted(this, &FPropertyCustomization_FlowYapFragment::Dialogue_OnTextCommitted)
			.OverflowPolicy(ETextOverflowPolicy::Clip)
			.HintText(LOCTEXT("DialogueText_Hint", "Enter dialogue text"))
			.ToolTipText(this, &FPropertyCustomization_FlowYapFragment::Dialogue_ToolTipText)
			.Margin(FMargin(0,0,0,0))
			.Padding(FMargin(4))
			.BackgroundColor(this, &FPropertyCustomization_FlowYapFragment::Dialogue_BackgroundColor)
			.ForegroundColor(this, &FPropertyCustomization_FlowYapFragment::Dialogue_ForegroundColor)
			.HScrollBar(HScrollBar)
			.VScrollBar(VScrollBar)
		]
		// TODO hatch overlay for disabled fragments?
		+ SOverlay::Slot()
		.Padding(-2)
		[
			SNew(SBorder)
			//.BorderImage(FAppStyle::GetBrush("Menu.Background")) // Filled, Square, bit dark
			.BorderImage(FAppStyle::GetBrush("Graph.StateNode.Body")) // Filled, rounded nicely
			//.BorderImage(FAppStyle::GetBrush("Brushes.Panel")) // Filled, Square, Dark
			.Visibility(this, &FPropertyCustomization_FlowYapFragment::DialogueBackground_Visibility)
			.BorderBackgroundColor(this, &FPropertyCustomization_FlowYapFragment::Dialogue_BorderBackgroundColor)
		]
		
	];
}

FVector2D FPropertyCustomization_FlowYapFragment::DialogueScrollBar_Thickness() const
{
	/*
	if (FragmentFocused())
	{
		return FVector2D(8, 8);
	}
	
	return FVector2D(0, 0);
	*/
	return FVector2D(8, 8);
}

FOptionalSize FPropertyCustomization_FlowYapFragment::Dialogue_MaxDesiredHeight() const
{
	/*
	if (FragmentFocused())
	{
		int16 DeadSpace = 15;
		int16 LineHeight = 15;
		int16 FocusedLines = 9;
	
		return DeadSpace + FocusedLines * LineHeight;
	}

	return 58; // TODO fluctuate with portrait widget height
	*/

	int16 DeadSpace = 15;
	int16 LineHeight = 15;
	int16 FocusedLines = 9;
	
	return DeadSpace + FocusedLines * LineHeight;
}

FText FPropertyCustomization_FlowYapFragment::Dialogue_Text() const
{
	FText Text;
	Bit_DialogueTextHandle->GetValue(Text);
	return Text;
}

void FPropertyCustomization_FlowYapFragment::Dialogue_OnTextCommitted(const FText& CommittedText, ETextCommit::Type CommitType)
{
	FFlowYapTransactions::BeginModify(LOCTEXT("NodeDialogueTextChanged", "Dialogue Text Changed"), Dialogue.Get());

	if (CommitType == ETextCommit::OnEnter || CommitType == ETextCommit::OnUserMovedFocus)
	{
		Bit_DialogueTextHandle->SetValue(CommittedText);
	}

	FFlowYapTransactions::EndModify();
}

FText FPropertyCustomization_FlowYapFragment::Dialogue_ToolTipText() const
{/*
	uint8 FocusedFragmentIndex;
	if (Owner->GetFocusedFragmentIndex(FocusedFragmentIndex) && FocusedFragmentIndex == FragmentIndex)
	{
		return LOCTEXT("DialogueText_Tooltip", "To be displayed during speaking");
	}
	*/

	FText TitleText;
	Bit_TitleTextHandle->GetValue(TitleText);
	return TitleText.IsEmptyOrWhitespace() ? LOCTEXT("DialogueText_Tooltip", "No title text") : FText::Format(LOCTEXT("DialogueText_Tooltip", "Title Text: {0}"), GetFragment()->Bit.GetTitleText());
}

FSlateColor FPropertyCustomization_FlowYapFragment::Dialogue_BackgroundColor() const
{
	if (InErrorState())
	{
		return YapColor::Error;
	}
	
	if (GetFragment()->GetBitReplaced())
	{
		return YapColor::Yellow;
	}
	
	return Dialogue.Get()->GetIsPlayerPrompt() ? YapColor::White : YapColor::Noir;
}

FSlateColor FPropertyCustomization_FlowYapFragment::Dialogue_ForegroundColor() const
{
	bool bInErrorState = InErrorState();
	
	if (bInErrorState)
	{
		return YapColor::Error;
	}
	
	FLinearColor Color = YapColor::Error;
	
	Color = Dialogue.Get()->GetIsPlayerPrompt() ? YapColor::White : YapColor::LightGray;
	
	if (GEditor->PlayWorld)
	{
		if (Dialogue.Get()->GetRunningFragmentIndex() == GetFragment()->IndexInDialogue)
		{
			Color = YapColor::White;
		}
		
		Color = (GetFragment()->IsActivationLimitMet()) ? YapColor::DarkRed : YapColor::LightGray;
	}

	FFlowYapFragment* Fragment = GetFragment();
	
	if (Fragment->GetBitReplaced())
	{
		Color *= YapColor::LightBlue;
	}

	return Color;
}

EVisibility FPropertyCustomization_FlowYapFragment::DialogueBackground_Visibility() const
{
	if (InErrorState())
	{
		return EVisibility::Collapsed;
	}
	
	return GetFragment()->GetBitReplaced() ? EVisibility::Visible : EVisibility::Collapsed;
}

FSlateColor FPropertyCustomization_FlowYapFragment::Dialogue_BorderBackgroundColor() const
{
	return YapColor::LightYellow_SuperGlass;
}

// ================================================================================================
// LOCAL ACTIVATION LIMITER WIDGET
// ------------------------------------------------------------------------------------------------

TSharedRef<SBox> FPropertyCustomization_FlowYapFragment::CreateActivationLimiterWidget()
{
	TSharedRef<SVerticalBox> ActivationLimiter_VerticalBox = SNew(SVerticalBox);

	if (InErrorState())
	{
		return SNew(SBox)
		.Visibility(this, &FPropertyCustomization_FlowYapFragment::ActivationLimiter_Visibility)
		[
			ActivationLimiter_VerticalBox
		];
	}
	
	if (GetFragment()->ActivationLimit <= 1)
	{
		int32 Size = 16;// i < ActivationCount ? 16 : 12;
		ActivationLimiter_VerticalBox->AddSlot()
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Center)
		.AutoHeight()
		.Padding(0)
		[
			SNew(SBox)
			.WidthOverride(Size)
			.HeightOverride(Size)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			.Padding(0)
			.Visibility(this, &FPropertyCustomization_FlowYapFragment::ActivationDot_Visibility)
			[
				SNew(SButton)
				.ButtonStyle(FYapEditorStyle::Get(), "ButtonStyle.ActivationLimit")
				.ContentPadding(0)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				.OnClicked(this, &FPropertyCustomization_FlowYapFragment::ActivationDot_OnClicked)
				.ToolTipText(LOCTEXT("DialogueNode_Tooltip", "Toggle local activation limit"))
				[
					SNew(SOverlay)
					+ SOverlay::Slot()
					[
						SNew(SImage)
						.DesiredSizeOverride(FVector2D(Size, Size))
						//.Image(FYapEditorStyle::Get().GetBrush("ImageBrush.Icon.LocalLimit"))
						.Image(FAppStyle::GetBrush("GenericCommands.Redo"))
						.ColorAndOpacity(YapColor::LightGreen)
					]
					+ SOverlay::Slot()
					[
						SNew(SImage)
						.DesiredSizeOverride(FVector2D(Size, Size))
						//.Image(FYapEditorStyle::Get().GetBrush("ImageBrush.Icon.LocalLimit"))
						.Image(FAppStyle::GetBrush("SourceControl.StatusIcon.Off"))
						.ColorAndOpacity(this, &FPropertyCustomization_FlowYapFragment::ActivationDot_ColorAndOpacity)
					]
				]
			]
		];
	}
	else
	{			
		ActivationLimiter_VerticalBox->AddSlot()
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Center)
		.AutoHeight()
		[
			SNew(STextBlock)
			.Text(this, &FPropertyCustomization_FlowYapFragment::ActivationLimiter_Text)
			.ColorAndOpacity(YapColor::White)
			//.TextStyle(&NormalText)
			.Justification(ETextJustify::Center)
		];
	}

	return SNew(SBox)
	.Visibility(this, &FPropertyCustomization_FlowYapFragment::ActivationLimiter_Visibility)
	[
		ActivationLimiter_VerticalBox
	];
}

EVisibility FPropertyCustomization_FlowYapFragment::ActivationLimiter_Visibility() const
{
	if (InErrorState())
	{
		return EVisibility::Collapsed;
	}
	
	if (GEditor->PlayWorld)
	{
		return (GetFragment()->ActivationLimit > 0) ? EVisibility::Visible : EVisibility::Collapsed;		
	}
	else
	{
		if (IsHovered() || GetFragment()->ActivationLimit > 0)
		{
			return EVisibility::Visible;
		}
	}

	return EVisibility::Collapsed;
}

FText FPropertyCustomization_FlowYapFragment::ActivationLimiter_Text() const
{
	if (InErrorState())
	{
		return INVTEXT("ERR");
	}
	
	return FText::Join
	(
		FText::FromString("/"),
		FText::AsNumber(GetFragment()->GetActivationCount()),
		FText::AsNumber(GetFragment()->GetActivationLimit())
	);
}

EVisibility FPropertyCustomization_FlowYapFragment::ActivationDot_Visibility() const
{
	if (InErrorState())
	{
		return EVisibility::Collapsed;
	}
	
	if (IsHovered() || GetFragment()->GetActivationLimit() > 0)
	{
		return EVisibility::Visible;
	}

	return EVisibility::Collapsed;
}

FSlateColor FPropertyCustomization_FlowYapFragment::ActivationDot_ColorAndOpacity() const
{
	if (InErrorState())
	{
		return YapColor::Error;
	}
	
	int32 ActivationLimit = GetFragment()->ActivationLimit;

	if (ActivationLimit == 0)
	{
		return YapColor::DarkGray_Glass;
	}
	
	FLinearColor Color = YapColor::Red;
	
	if (GEditor->PlayWorld)
	{
		if (GetFragment()->IsActivationLimitMet())
		{
			Color = YapColor::Red;
		}
		else
		{
			Color = YapColor::Transparent;
		}
	}
	
	return Color;
}

FReply FPropertyCustomization_FlowYapFragment::ActivationDot_OnClicked()
{
	if (InErrorState())
	{
		return FReply::Unhandled();
	}
	
	FFlowYapFragment* Fragment = GetFragment();
	
	FFlowYapTransactions::BeginModify(LOCTEXT("Dialogue", "Change activation limit"), Dialogue.Get());
	
	// TODO ignore input during PIE?
	if (Fragment->ActivationLimit > 0)
	{
		Fragment->ActivationLimit = 0;
	}
	else
	{
		Fragment->ActivationLimit = 1;
	}

	FFlowYapTransactions::EndModify();
	
	return FReply::Handled();
}

// ================================================================================================
// FRAGMENT TAG OVERLAY WIDGET (OVER DIALOGUE, PREVIEW PURPOSE ONLY)
// ------------------------------------------------------------------------------------------------

TSharedRef<SWidget> FPropertyCustomization_FlowYapFragment::CreateFragmentTagPreviewWidget()
{	
	return SNew(SBorder)
	.BorderImage(FAppStyle::Get().GetBrush("DetailsView.CategoryMiddle"))
	.BorderBackgroundColor(this, &FPropertyCustomization_FlowYapFragment::FragmentTagPreview_BorderBackgroundColor)
	.Padding(6, 5, 6, 5)
	.Visibility(this, &FPropertyCustomization_FlowYapFragment::FragmentTagPreview_Visibility)
	.ColorAndOpacity(YapColor::White)//this, &FPropertyCustomization_FlowYapFragment::FragmentTagPreview_ColorAndOpacity)
	[
		SNew(STextBlock)
		.Text(this, &FPropertyCustomization_FlowYapFragment::FragmentTagPreview_Text)
		.IsEnabled(false)
		.Font(FAppStyle::GetFontStyle("SmallFont"))
	];
}

EVisibility FPropertyCustomization_FlowYapFragment::FragmentTagPreview_Visibility() const
{
	if (InErrorState())
	{
		return EVisibility::Collapsed;
	}
	
	if (InErrorState())
	{
	}
	if (GetFragment()->FragmentTag == FGameplayTag::EmptyTag || DialogueBox->HasKeyboardFocus())
	{
		return EVisibility::Collapsed;
	}

	if (!GetFragment()->FragmentTag.IsValid())
	{
		return EVisibility::HitTestInvisible;
	}
		
	return EVisibility::HitTestInvisible;
}

FText FPropertyCustomization_FlowYapFragment::FragmentTagPreview_Text() const
{
	if (InErrorState())
	{
		return INVTEXT("ERR");
	}
	
	// Pass tag from the properties

	FString Filter = Dialogue.Get()->GetDialogueTag().ToString();
	
	return FText::FromString(FlowYapUtil::GetFilteredSubTag(Filter, GetFragment()->FragmentTag));
}

FSlateColor FPropertyCustomization_FlowYapFragment::FragmentTagPreview_BorderBackgroundColor() const
{
	if (DialogueBox->HasKeyboardFocus())
	{
		return YapColor::DeepGray_Glass;
	}
	
	return YapColor::DeepGray_SemiTrans;
}

FLinearColor FPropertyCustomization_FlowYapFragment::FragmentTagPreview_ColorAndOpacity() const
{
	if (DialogueBox->HasKeyboardFocus())
	{
		return YapColor::Gray_Trans;
	}
	
	return YapColor::White;
}

// ================================================================================================
// FRAGMENT TIME PADDING WIDGET
// ------------------------------------------------------------------------------------------------

TSharedRef<SWidget> FPropertyCustomization_FlowYapFragment::CreateFragmentTimePaddingWidget()
{
	return SNew(SBox)
	[
		SNew(SOverlay)
		+ SOverlay::Slot()
		.Padding(8, 0)
		[
			SNew(SBox)
			.HeightOverride(2)
			[
				SNew(SProgressBar)
				.BorderPadding(0)
				.Percent(this, &FPropertyCustomization_FlowYapFragment::FragmentTimePadding_Percent)
				.Style(FYapEditorStyle::Get(), "ProgressBarStyle.FragmentTimePadding")
				.FillColorAndOpacity(this, &FPropertyCustomization_FlowYapFragment::FragmentTimePadding_FillColorAndOpacity)
			]
		]
		+ SOverlay::Slot()
		.Padding(0, -2)
		[
			SNew(SSlider)
			.Visibility(this, &FPropertyCustomization_FlowYapFragment::FragmentTimePaddingSlider_Visibility)
			.Value(this, &FPropertyCustomization_FlowYapFragment::FragmentTimePadding_Value)
			.OnValueChanged(this, &FPropertyCustomization_FlowYapFragment::FragmentTimePadding_OnValueChanged)
			.Style(FYapEditorStyle::Get(), "SliderStyle.FragmentTimePadding")
			.SliderHandleColor(YapColor::Gray)
			.ToolTipText(this, &FPropertyCustomization_FlowYapFragment::FragmentTimePadding_ToolTipText)
		]
	];
}

TOptional<float> FPropertyCustomization_FlowYapFragment::FragmentTimePadding_Percent() const
{
	if (InErrorState())
	{
		return 0;
	}
	
	const float MaxPaddedSetting =  UFlowYapProjectSettings::Get()->GetFragmentPaddingSliderMax();
	const float FragmentPadding = GetFragment()->GetPaddingToNextFragment();

	if (GEditor->PlayWorld)
	{
		const TOptional<uint8>& RunningIndex = Dialogue.Get()->RunningFragmentIndex;

		if (!RunningIndex.IsSet() || RunningIndex.GetValue() < GetFragment()->IndexInDialogue)
		{
			return FragmentPadding / MaxPaddedSetting;
		}

		if (RunningIndex == GetFragment()->IndexInDialogue)
		{
			if (Dialogue.Get()->FragmentStartedTime < Dialogue.Get()->FragmentEndedTime)
			{
				double ElapsedPaddingTime = GEditor->PlayWorld->GetTimeSeconds() - Dialogue.Get()->FragmentEndedTime;
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

float FPropertyCustomization_FlowYapFragment::FragmentTimePadding_Value() const
{
	if (InErrorState())
	{
		return 0;
	}
	
	const float MaxPaddedSetting =  UFlowYapProjectSettings::Get()->GetFragmentPaddingSliderMax();

	return GetFragment()->GetPaddingToNextFragment() / MaxPaddedSetting;
}

EVisibility FPropertyCustomization_FlowYapFragment::FragmentTimePaddingSlider_Visibility() const
{
	if (GEditor->PlayWorld)
	{
		return EVisibility::Hidden;
	}

	return EVisibility::Visible;
}

void FPropertyCustomization_FlowYapFragment::FragmentTimePadding_OnValueChanged(float X)
{	
	const float MaxPaddedSetting =  UFlowYapProjectSettings::Get()->GetFragmentPaddingSliderMax();
	float NewValue = X * MaxPaddedSetting;
	
	bool bSetCommon = false;
	
	if (!bCtrlPressed)
	{
		const TArray<float>& CommonPaddings = UFlowYapProjectSettings::Get()->GetCommonFragmentPaddings();
		
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
				
				GetFragment()->GetCommonPaddingSettingMutable() = Index;
				GetFragment()->SetPaddingToNextFragment(0);
				
				bSetCommon = true;
			}
		}
	}
	
	if (!bSetCommon)
	{
		GetFragment()->SetPaddingToNextFragment(NewValue);
		GetFragment()->GetCommonPaddingSettingMutable().Reset();
	}
}

FSlateColor FPropertyCustomization_FlowYapFragment::FragmentTimePadding_FillColorAndOpacity() const
{
	if (InErrorState())
	{
		return YapColor::Error;
	}
	
	if (GEditor->PlayWorld)
	{
		const TOptional<uint8>& RunningIndex = Dialogue.Get()->RunningFragmentIndex;

		if (!RunningIndex.IsSet() || RunningIndex.GetValue() < GetFragment()->IndexInDialogue)
		{
		}

		if (RunningIndex == GetFragment()->IndexInDialogue)
		{
			return YapColor::White;
		}

		return YapColor::DarkGray;
	}
	
	return GetFragment()->GetCommonPaddingSetting().IsSet() ? YapColor::LightBlue : YapColor::White;
}

FText FPropertyCustomization_FlowYapFragment::FragmentTimePadding_ToolTipText() const
{
	if (InErrorState())
	{
		return INVTEXT("ERR");
	}
	
	return FText::Format(LOCTEXT("Fragment", "Delay: {0}"), GetFragment()->GetPaddingToNextFragment());
}

// ================================================================================================
// PORTRAIT WIDGET
// ------------------------------------------------------------------------------------------------

TSharedRef<SBox> FPropertyCustomization_FlowYapFragment::CreatePortraitWidget()
{
	double StartTime = FPlatformTime::Seconds();
	
	TSharedRef<SBox> Box = SNew(SBox)
	.Padding(4, 0, 0, 0)
	[
		SNew(SOverlay)
		.Visibility(this, &FPropertyCustomization_FlowYapFragment::PortraitImage_Visibility)
		+ SOverlay::Slot()
		.Padding(0, 0, 0, 0)
		[
			SNew(SBox)
			.WidthOverride(58)//(74)
			.HeightOverride(58)//(74)
			[
				SNew(SBorder)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				.Padding(0.0f)
				.BorderImage(FAppStyle::Get().GetBrush("PropertyEditor.AssetThumbnailBorder"))
				[
					SNew(SOverlay)
					+ SOverlay::Slot()
					.VAlign(VAlign_Center)
					.HAlign(HAlign_Center)
					[
						SNew(SImage)
						.DesiredSizeOverride(FVector2D(48, 48))
						.Image(this, &FPropertyCustomization_FlowYapFragment::PortraitImage_Image)
					]		
					+ SOverlay::Slot()
					.VAlign(VAlign_Center)
					.HAlign(HAlign_Center)
					[
						SNew(STextBlock)
						.RenderTransformPivot(FVector2D(0.5, 0.5))
						.RenderTransform(FSlateRenderTransform(FQuat2D(FMath::DegreesToRadians(-30.0f))))
						.Visibility(this, &FPropertyCustomization_FlowYapFragment::MissingPortraitWarning_Visibility)
						.Text(LOCTEXT("FragmentCharacterMissing", "Missing"))
					]
				]
			]
		]
		/*
		+ SOverlay::Slot()
		.VAlign(VAlign_Bottom)
		.HAlign(HAlign_Right)
		.Padding(6, 0, 2, 2)
		[
			CreateMoodKeySelectorWidget()
		]
		*/
		/*
		+ SOverlay::Slot()
		.VAlign(VAlign_Top)
		.HAlign(HAlign_Fill)
		.Padding(2, 2, 2, 0)
		[
			SNew(SBox)
			.MaxDesiredWidth(54)
			.Visibility(this, &FPropertyCustomization_FlowYapFragment::CharacterSelect_Visibility)
			[
				SNew(SObjectPropertyEntryBox)
				.DisplayBrowse(false)
				.DisplayThumbnail(false)
				.DisplayUseSelected(false)
				.AllowedClass(UFlowYapCharacter::StaticClass())
				.EnableContentPicker(true)
				.ObjectPath(this, &FPropertyCustomization_FlowYapFragment::CharacterSelect_ObjectPath)
				.OnObjectChanged(this, &FPropertyCustomization_FlowYapFragment::CharacterSelect_OnObjectChanged)
				.ToolTipText(LOCTEXT("FragmentBitCharacter_Tooltip", "Select a Character."))
			]
		]
		*/
	];

	double EndTime = FPlatformTime::Seconds();

	//UE_LOG(LogTemp, Warning, TEXT("CreatePortraitWidget took %f ms"), 1000 * (EndTime - StartTime));
	
	return Box;
}

EVisibility FPropertyCustomization_FlowYapFragment::PortraitImage_Visibility() const
{
	return EVisibility::Visible;
}

const FSlateBrush* FPropertyCustomization_FlowYapFragment::PortraitImage_Image() const
{
	if (InErrorState())
	{
		return nullptr;
	}
	
	return GetFragment()->GetBit().GetSpeakerPortraitBrush();
}

EVisibility FPropertyCustomization_FlowYapFragment::MissingPortraitWarning_Visibility() const
{
	if (InErrorState())
	{
		return EVisibility::Collapsed;
	}
	
	const FSlateBrush* Brush = GetFragment()->GetBit().GetSpeakerPortraitBrush();

	if (Brush)
	{
		return (Brush->GetResourceObject()) ? EVisibility::Hidden : EVisibility::Visible;
	}
	
	return EVisibility::Visible;
}

EVisibility FPropertyCustomization_FlowYapFragment::CharacterSelect_Visibility() const
{
	return IsHovered() ? EVisibility::Visible : EVisibility::Collapsed;
}

FString FPropertyCustomization_FlowYapFragment::CharacterSelect_ObjectPath() const
{
	if (InErrorState())
	{
		return "";
	}
	
	const TSoftObjectPtr<UFlowYapCharacter> Asset = GetFragment()->Bit.GetCharacterAsset();

	if (Asset.IsPending())
	{
		Asset.LoadSynchronous();
	}
	
	if (!Asset) { return ""; }

	return Asset.ToString();
}

void FPropertyCustomization_FlowYapFragment::CharacterSelect_OnObjectChanged(const FAssetData& InAssetData)
{
	UObject* Asset = InAssetData.GetAsset();

	UFlowYapCharacter* Character = Cast<UFlowYapCharacter>(Asset);

	if (Character)
	{
		FFlowYapTransactions::BeginModify(LOCTEXT("NodeCharacterChanged", "Character Changed"), Dialogue.Get());

		GetFragment()->Bit.SetCharacter(Character);

		FFlowYapTransactions::EndModify();
	}

}

// ================================================================================================
// MOOD KEY SELECTOR WIDGET
// ------------------------------------------------------------------------------------------------

TSharedRef<SBox> FPropertyCustomization_FlowYapFragment::CreateMoodKeySelectorWidget()
{
	TSharedPtr<SBox> Box;
	FMenuBuilder MenuBuilder(true, nullptr);
	FName SelectedMoodKey = GetCurrentMoodKey();

	for (const FName& MoodKey : UFlowYapProjectSettings::Get()->GetMoodKeys())
	{
		if (MoodKey == NAME_None)
		{
			UE_LOG(FlowYap, Warning, TEXT("Warning: Portrait keys contains a 'NONE' entry. Clean this up!"));
			continue;
		}
		
		bool bSelected = MoodKey == SelectedMoodKey;
		MenuBuilder.AddWidget(CreateMoodKeyMenuEntryWidget(MoodKey, bSelected), FText::GetEmpty());
	}

	TSharedPtr<SImage> PortraitIconImage;
	
	FString IconPath = UFlowYapProjectSettings::Get()->GetPortraitIconPath(GetCurrentMoodKey());

	// TODO ensure that system works and displays labels if user does not supply icons but only FNames. Use Generic mood icon?
	SAssignNew(Box, SBox)
	.Visibility(this, &FPropertyCustomization_FlowYapFragment::MoodKeySelector_Visibility)
	[
		SNew(SComboButton)
		.HasDownArrow(false)
		.ContentPadding(FMargin(0.f, 0.f))
		.MenuPlacement(MenuPlacement_CenteredBelowAnchor)
		.ButtonColorAndOpacity(FSlateColor(FLinearColor(0.f, 0.f, 0.f, 0.75f)))
		.HAlign(HAlign_Center)
		.ButtonStyle(FAppStyle::Get(), "SimpleButton")
		.OnMenuOpenChanged(this, &FPropertyCustomization_FlowYapFragment::MoodKeySelector_OnMenuOpenChanged)
		.ToolTipText(LOCTEXT("Fragment", "Choose a mood key"))
		.ButtonContent()
		[
			SNew(SBox)
			.Padding(4, 4)
			[
				SAssignNew(PortraitIconImage, SImage)
				.ColorAndOpacity(FSlateColor::UseForeground())
				.Image(this, &FPropertyCustomization_FlowYapFragment::MoodKeyBrush_GetBrush)
			]
		]
		.MenuContent()
		[
			MenuBuilder.MakeWidget()
		]
	];
	
	return Box.ToSharedRef();
}

EVisibility FPropertyCustomization_FlowYapFragment::MoodKeySelector_Visibility() const
{
	return IsHovered() || MoodKeySelectorMenuOpen ? EVisibility::Visible : EVisibility::Collapsed;
}

void FPropertyCustomization_FlowYapFragment::MoodKeySelector_OnMenuOpenChanged(bool bMenuOpen)
{
	MoodKeySelectorMenuOpen = bMenuOpen;
}

const FSlateBrush* FPropertyCustomization_FlowYapFragment::MoodKeyBrush_GetBrush() const
{
	return GEditor->GetEditorSubsystem<UFlowYapEditorSubsystem>()->GetMoodKeyBrush(GetCurrentMoodKey());
}

FName FPropertyCustomization_FlowYapFragment::GetCurrentMoodKey() const
{
	if (InErrorState())
	{
		return NAME_None;
	}
	
	return GetFragment()->Bit.GetMoodKey();
}

// ================================================================================================
// MOOD KEY MENU ENTRY WIDGET
// ------------------------------------------------------------------------------------------------

TSharedRef<SWidget> FPropertyCustomization_FlowYapFragment::CreateMoodKeyMenuEntryWidget(FName InIconName, bool bSelected, const FText& InLabel, FName InTextStyle)
{
	const UFlowYapProjectSettings* ProjectSettings = UFlowYapProjectSettings::Get();
		
	TSharedPtr<SHorizontalBox> HBox = SNew(SHorizontalBox);

	TSharedPtr<SImage> PortraitIconImage;
		
	FString IconPath = ProjectSettings->GetPortraitIconPath(InIconName);

	// TODO this is dumb, cache FSlateIcons or FSlateBrushes in the subsystem instead?
	UTexture2D* MoodKeyIcon = GEditor->GetEditorSubsystem<UFlowYapEditorSubsystem>()->GetMoodKeyIcon(InIconName);
	
	FSlateBrush Brush;
	Brush.ImageSize = FVector2D(16, 16);
	Brush.SetResourceObject(MoodKeyIcon);
	
	TSharedRef<FDeferredCleanupSlateBrush> MoodKeyBrush = FDeferredCleanupSlateBrush::CreateBrush(Brush);
	
	if (!InIconName.IsNone())
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
	.OnClicked(this, &FPropertyCustomization_FlowYapFragment::MoodKeyMenuEntry_OnClicked, InIconName)
	[
		SAssignNew(PortraitIconImage, SImage)
		.ColorAndOpacity(FSlateColor::UseForeground())
		.Image(TAttribute<const FSlateBrush*>::Create(TAttribute<const FSlateBrush*>::FGetter::CreateLambda([MoodKeyBrush](){return MoodKeyBrush->GetSlateBrush();})))
	];
}

FReply FPropertyCustomization_FlowYapFragment::MoodKeyMenuEntry_OnClicked(FName NewValue)
{
	if (InErrorState())
	{
		return FReply::Unhandled();
	}
	
	FFlowYapTransactions::BeginModify(LOCTEXT("NodeMoodKeyChanged", "Portrait Key Changed"), Dialogue.Get());

	GetFragment()->Bit.SetMoodKey(NewValue);

	FFlowYapTransactions::EndModify();

	return FReply::Handled();
}



// ================================================================================================
// TITLE TEXT WIDGET
// ------------------------------------------------------------------------------------------------

TSharedRef<SWidget> FPropertyCustomization_FlowYapFragment::CreateTitleTextWidget()
{
	return SAssignNew(TitleTextBox, SEditableTextBox)
	.Visibility(this, &FPropertyCustomization_FlowYapFragment::TitleText_Visibility)
	.Text(this, &FPropertyCustomization_FlowYapFragment::TitleText_Text)
	.OnTextCommitted(this, &FPropertyCustomization_FlowYapFragment::TitleText_OnTextCommitted)
	.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
	.Padding(2)
	.HintText(LOCTEXT("TitleText_Hint", "Enter optional title text"))
	.ToolTipText(LOCTEXT("TitleText_Tooltip", "Title text may be used to build player's dialogue selection list."));
}

EVisibility FPropertyCustomization_FlowYapFragment::TitleText_Visibility() const
{
	if (Dialogue.Get()->GetIsPlayerPrompt())
	{
		return EVisibility::Visible;
	}
	
	return UFlowYapProjectSettings::Get()->GetHideTitleTextOnNPCDialogueNodes() ? EVisibility::Hidden : EVisibility::Visible;
}

FText FPropertyCustomization_FlowYapFragment::TitleText_Text() const
{
	if (InErrorState())
	{
		return FText::GetEmpty();
	}
	
	return GetFragment()->Bit.GetTitleText();
}

void FPropertyCustomization_FlowYapFragment::TitleText_OnTextCommitted(const FText& CommittedText, ETextCommit::Type CommitType)
{
	if (InErrorState())
	{
		return;
	}
	
	FFlowYapTransactions::BeginModify(LOCTEXT("NodeTitleTextChanged", "Title Text Changed"), Dialogue.Get());
	
	if (CommitType == ETextCommit::OnEnter || CommitType == ETextCommit::OnUserMovedFocus)
	{
		GetFragment()->Bit.SetTitleText(CommittedText);
	}

	FFlowYapTransactions::EndModify();
}

// ================================================================================================
// FRAGMENT TAG WIDGET
// ------------------------------------------------------------------------------------------------

TSharedRef<SWidget> FPropertyCustomization_FlowYapFragment::CreateFragmentTagWidget()
{
	const FString FilterString = Dialogue.Get()->GetDialogueTag().ToString();

	return SNew(SGameplayTagComboFiltered)
	.Tag(this, &FPropertyCustomization_FlowYapFragment::FragmentTag_Tag)
	.Filter(FilterString)
	.OnTagChanged(this, &FPropertyCustomization_FlowYapFragment::FragmentTag_OnTagChanged);
}

FGameplayTag FPropertyCustomization_FlowYapFragment::FragmentTag_Tag() const
{
	if (InErrorState())
	{
		return FGameplayTag::EmptyTag;
	}
	
	return GetFragment()->FragmentTag;
}

void FPropertyCustomization_FlowYapFragment::FragmentTag_OnTagChanged(FGameplayTag GameplayTag)
{
	if (InErrorState())
	{
		return;
	}
	
	FFlowYapTransactions::BeginModify(LOCTEXT("Fragment", "Change Fragment Tag"), Dialogue.Get());

	GetFragment()->FragmentTag = GameplayTag;

	FFlowYapTransactions::EndModify();
}

// ================================================================================================
// BOTTOM ROW WIDGET (AUDIO ASSET, TIME SETTINGS)
// ------------------------------------------------------------------------------------------------

TSharedRef<SBox> FPropertyCustomization_FlowYapFragment::CreateBottomRowWidget()
{
	FSlateIcon ProjectSettingsIcon(FAppStyle::GetAppStyleSetName(), "ProjectSettings.TabIcon");

	double StartTime = FPlatformTime::Seconds();
	
	TSharedRef<SBox> Box = SNew(SBox)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.Padding(0, 2, 0, 0)
		[
			// TODO this takes 1.5 ms, optimize it?? 
			CreateAudioAssetWidget()
		]
		+ SHorizontalBox::Slot()
		.Padding(0, 0, 1, 0)
		.AutoWidth()
		[
			SNew(SSeparator)
			.SeparatorImage(FAppStyle::Get().GetBrush("Menu.Separator"))
			.Orientation(Orient_Vertical)
			.Thickness(1.0f)
			.ColorAndOpacity(FLinearColor::Gray)
		]
		// -------------------
		// TIME SETTINGS
		// -------------------
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.HAlign(HAlign_Right)
		.Padding(6,2,0,0)
		[
			SNew(SBox)
			[
				SNew(SHorizontalBox)
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
					.Style(&UFlowYapEditorSubsystem::GetCheckBoxStyles().ToggleButtonCheckBox_Green)
					.Padding(FMargin(4, 3))
					.CheckBoxContentUsesAutoWidth(true)
					.ToolTipText(LOCTEXT("UseProjectDefaultTimeSettings_Tooltip", "Use time settings from project settings"))
					.IsEnabled(true)
					.IsChecked(this, &FPropertyCustomization_FlowYapFragment::UseProjectDefaultTimeSettingsButton_IsChecked)
					.OnCheckStateChanged(this, &FPropertyCustomization_FlowYapFragment::UseProjectDefaultTimeSettingsButton_OnCheckStateChanged)
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
					.IsEnabled(this, &FPropertyCustomization_FlowYapFragment::UseManuallyEnteredTimeButton_IsEnabled)
					.IsChecked(this, &FPropertyCustomization_FlowYapFragment::UseManuallyEnteredTimeButton_IsChecked)
					.OnCheckStateChanged(this, &FPropertyCustomization_FlowYapFragment::UseManuallyEnteredTimeButton_OnCheckStateChanged)
					[
						SNew(SImage)
						.ColorAndOpacity(FSlateColor::UseForeground())
						.Image(FYapEditorStyle::Get().GetBrush("ImageBrush.Icon.Timer"))
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
					.IsEnabled(this, &FPropertyCustomization_FlowYapFragment::UseTextTimeButton_IsEnabled)
					.IsChecked(this, &FPropertyCustomization_FlowYapFragment::UseTextTimeButton_IsChecked)
					.OnCheckStateChanged(this, &FPropertyCustomization_FlowYapFragment::UseTextTimeButton_OnCheckStateChanged)
					[
						SNew(SBox)
						[
							SNew(SImage)
							.Image(FYapEditorStyle::Get().GetBrush("ImageBrush.Icon.TextTime"))
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
					.IsEnabled(this, &FPropertyCustomization_FlowYapFragment::UseAudioTimeButton_IsEnabled)
					.IsChecked(this, &FPropertyCustomization_FlowYapFragment::UseAudioTimeButton_IsChecked)
					.OnCheckStateChanged(this, &FPropertyCustomization_FlowYapFragment::UseAudioTimeButton_OnCheckStateChanged)
					.HAlign(HAlign_Center)
					[
						SNew(SImage)
						.Image(FYapEditorStyle::Get().GetBrush("ImageBrush.Icon.AudioTime"))
					]
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(3, 0, 1, 0)
				//.MaxWidth(73)
				[
					// =============================
					// TIME DISPLAY / MANUAL ENTRY FIELD
					// =============================
					SNew(SBox)
					.WidthOverride(58)
					.VAlign(VAlign_Fill)
					[
						SNew(SNumericEntryBox<double>)
						.IsEnabled(this, &FPropertyCustomization_FlowYapFragment::TimeEntryBox_IsEnabled)
						.Delta(0.1)
						.MinValue(0.0)
						.ToolTipText(LOCTEXT("FragmentTimeEntry_Tooltip", "Time this dialogue fragment will play for"))
						.Justification(ETextJustify::Center)
						.Value(this, &FPropertyCustomization_FlowYapFragment::TimeEntryBox_Value)
						.OnValueCommitted(this, &FPropertyCustomization_FlowYapFragment::TimeEntryBox_OnValueCommitted)
					]
				]
			]
		]
	];

	double EndTime = FPlatformTime::Seconds();

	//UE_LOG(LogTemp, Warning, TEXT("CreateBottomRowWidget took %f ms"), 1000 * (EndTime - StartTime));

	return Box;
}

// ---------------------
ECheckBoxState FPropertyCustomization_FlowYapFragment::UseProjectDefaultTimeSettingsButton_IsChecked() const
{
	if (InErrorState())
	{
		return ECheckBoxState::Undetermined;
	}
	
	return GetFragment()->Bit.GetUseProjectDefaultTimeSettings() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void FPropertyCustomization_FlowYapFragment::UseProjectDefaultTimeSettingsButton_OnCheckStateChanged(ECheckBoxState CheckBoxState)
{
	FFlowYapTransactions::BeginModify(LOCTEXT("NodeUseProjectDefaultTimeSettings", "Use Project Default Time Settings Changed"), Dialogue.Get());

	GetFragment()->Bit.SetUseProjectDefaultSettings(CheckBoxState == ECheckBoxState::Checked);
	
	FFlowYapTransactions::EndModify();
}

// ---------------------
bool FPropertyCustomization_FlowYapFragment::UseManuallyEnteredTimeButton_IsEnabled() const
{
	if (InErrorState())
	{
		return false;
	}
	
	return GetFragment()->Bit.GetUseProjectDefaultTimeSettings() ? false : true;
}

ECheckBoxState FPropertyCustomization_FlowYapFragment::UseManuallyEnteredTimeButton_IsChecked() const
{
	if (InErrorState())
	{
		return ECheckBoxState::Undetermined;
	}
	
	return GetFragment()->Bit.GetTimeMode() == EFlowYapTimeMode::ManualTime ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void FPropertyCustomization_FlowYapFragment::UseManuallyEnteredTimeButton_OnCheckStateChanged(ECheckBoxState CheckBoxState)
{
	FFlowYapTransactions::BeginModify(LOCTEXT("Fragment", "Fragment Time Mode Changed"), Dialogue.Get());

	if (CheckBoxState == ECheckBoxState::Checked)
	{
		GetFragment()->Bit.SetBitTimeMode(EFlowYapTimeMode::ManualTime);
	}

	FFlowYapTransactions::EndModify();
}

// ---------------------
bool FPropertyCustomization_FlowYapFragment::UseTextTimeButton_IsEnabled() const
{
	if (InErrorState())
	{
		return false;
	}
	
	return GetFragment()->Bit.GetUseProjectDefaultTimeSettings() ? false : true;
}

ECheckBoxState FPropertyCustomization_FlowYapFragment::UseTextTimeButton_IsChecked() const
{
	if (InErrorState())
	{
		return ECheckBoxState::Undetermined;
	}
	
	return GetFragment()->Bit.GetTimeMode() == EFlowYapTimeMode::TextTime ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void FPropertyCustomization_FlowYapFragment::UseTextTimeButton_OnCheckStateChanged(ECheckBoxState CheckBoxState)
{
	FFlowYapTransactions::BeginModify(LOCTEXT("Fragment", "Fragment Time Mode Changed"), Dialogue.Get());

	if (CheckBoxState == ECheckBoxState::Checked)
	{
		GetFragment()->Bit.SetBitTimeMode(EFlowYapTimeMode::TextTime);
	}
	
	FFlowYapTransactions::EndModify();
}

// ---------------------
bool FPropertyCustomization_FlowYapFragment::UseAudioTimeButton_IsEnabled() const
{
	if (InErrorState())
	{
		return false;
	}
	
	return !GetFragment()->Bit.GetUseProjectDefaultTimeSettings();
}

ECheckBoxState FPropertyCustomization_FlowYapFragment::UseAudioTimeButton_IsChecked() const
{
	if (InErrorState())
	{
		return ECheckBoxState::Undetermined;
	}
	
	return GetFragment()->Bit.GetTimeMode() == EFlowYapTimeMode::AudioTime ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void FPropertyCustomization_FlowYapFragment::UseAudioTimeButton_OnCheckStateChanged(ECheckBoxState CheckBoxState)
{
	FFlowYapTransactions::BeginModify(LOCTEXT("NodeTimedModeChanged", "Timed Mode Changed"), Dialogue.Get());

	if (CheckBoxState == ECheckBoxState::Checked)
	{
		GetFragment()->Bit.SetBitTimeMode(EFlowYapTimeMode::AudioTime);
	}
	
	FFlowYapTransactions::EndModify();
}

bool FPropertyCustomization_FlowYapFragment::TimeEntryBox_IsEnabled() const
{
	if (InErrorState())
	{
		return false;
	}
	
	return GetFragment()->Bit.GetTimeMode() == EFlowYapTimeMode::ManualTime;
}

TOptional<double> FPropertyCustomization_FlowYapFragment::TimeEntryBox_Value() const
{
	if (InErrorState())
	{
		return 0;
	}
	
	double Time = GetFragment()->Bit.GetTime();
	
	return (Time > 0) ? Time : TOptional<double>();
}

void FPropertyCustomization_FlowYapFragment::TimeEntryBox_OnValueCommitted(double NewValue, ETextCommit::Type CommitType)
{
	FFlowYapTransactions::BeginModify(LOCTEXT("NodeEnteredTimeChanged", "Entered Time Changed"), Dialogue.Get());

	if (CommitType == ETextCommit::OnEnter || CommitType == ETextCommit::OnUserMovedFocus)
	{
		GetFragment()->Bit.SetManualTime(NewValue);
	}
	else if (CommitType == ETextCommit::OnCleared)
	{
		GetFragment()->Bit.SetManualTime(0.0);
	}

	FFlowYapTransactions::EndModify();
}

// ================================================================================================
// AUDIO ASSET WIDGET
// ------------------------------------------------------------------------------------------------

TSharedRef<SWidget> FPropertyCustomization_FlowYapFragment::CreateAudioAssetWidget()
{
	double StartTime = FPlatformTime::Seconds();

	UClass* DialogueAssetClass = UFlowYapProjectSettings::Get()->GetDialogueAssetClass();

	if (!DialogueAssetClass)
	{
		DialogueAssetClass = UObject::StaticClass();
	} 

	TSharedRef<SWidget> AudioAssetProperty = SNew(SObjectPropertyEntryBox)
		.DisplayBrowse(true)
		.DisplayUseSelected(false)
		.DisplayThumbnail(false)
		.AllowedClass(DialogueAssetClass)
		.EnableContentPicker(true)
		.ObjectPath(this, &FPropertyCustomization_FlowYapFragment::AudioAsset_ObjectPath)
		.OnObjectChanged(this, &FPropertyCustomization_FlowYapFragment::AudioAsset_OnObjectChanged)
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
		.Visibility(this, &FPropertyCustomization_FlowYapFragment::AudioAssetErrorState_Visibility)
		.ColorAndOpacity(this, &FPropertyCustomization_FlowYapFragment::AudioAssetErrorState_ColorAndOpacity)
	];
	
	double EndTime = FPlatformTime::Seconds();

	//UE_LOG(LogTemp, Warning, TEXT("CreateAudioAssetWidget took %f ms"), 1000 * (EndTime - StartTime));

	return Widget;
}

FText FPropertyCustomization_FlowYapFragment::AudioAsset_ObjectPathText() const
{
	return FText::FromString(AudioAsset_ObjectPath());
}

FString FPropertyCustomization_FlowYapFragment::AudioAsset_ObjectPath() const
{
	if (InErrorState())
	{
		return "";
	}
	
	const TSoftObjectPtr<UObject> Asset = GetFragment()->Bit.GetDialogueAudioAsset<UObject>();

	if (!Asset) { return ""; }

	return Asset.ToString();
}

void FPropertyCustomization_FlowYapFragment::AudioAsset_OnObjectChanged(const FAssetData& InAssetData)
{
	FFlowYapTransactions::BeginModify(LOCTEXT("NodeAudioAssetChanged", "Audio Asset Changed"), Dialogue.Get());

	GetFragment()->Bit.SetDialogueAudioAsset(InAssetData.GetAsset());

	FFlowYapTransactions::EndModify();
}

EVisibility FPropertyCustomization_FlowYapFragment::AudioAssetErrorState_Visibility() const
{
	if (InErrorState())
	{
		return EVisibility::Collapsed;
	}
	
	if (AudioAssetErrorLevel() != EFlowYapErrorLevel::OK)
	{
		return EVisibility::HitTestInvisible;
	}
	
	return EVisibility::Hidden;
}

FSlateColor FPropertyCustomization_FlowYapFragment::AudioAssetErrorState_ColorAndOpacity() const
{	
	switch (AudioAssetErrorLevel())
	{
	case EFlowYapErrorLevel::OK:
		{
			return YapColor::Green;
		}
	case EFlowYapErrorLevel::Warning:
		{
			return YapColor::Orange;
		}
	case EFlowYapErrorLevel::Error:
		{
			return YapColor::Red;
		}
	}

	return YapColor::Black;
}

EFlowYapErrorLevel FPropertyCustomization_FlowYapFragment::AudioAssetErrorLevel() const
{
	if (InErrorState())
	{
		return EFlowYapErrorLevel::Error;
	}
	
	UClass* AssetClass = UFlowYapProjectSettings::Get()->GetDialogueAssetClass();

	static EFlowYapErrorLevel CachedErrorLevel = EFlowYapErrorLevel::OK;
	static double LastUpdateTime = 0;
	
	if (GWorld)
	{
		if (GWorld->GetRealTimeSeconds() - LastUpdateTime < 1.0)
		{
			return CachedErrorLevel;
		}		
	}
	
	const TSoftObjectPtr<UObject> Asset = GetFragment()->Bit.GetDialogueAudioAsset<UObject>();

	if (Asset)
	{
		if (!Asset->IsA(AssetClass))
		{
			CachedErrorLevel = EFlowYapErrorLevel::Error;
			return CachedErrorLevel;
		}
	}

	if (GetFragment()->Bit.GetTimeMode() == EFlowYapTimeMode::AudioTime && !GetFragment()->Bit.HasDialogueAudioAsset())
	{
		CachedErrorLevel = UFlowYapProjectSettings::Get()->GetMissingAudioErrorLevel();
		return CachedErrorLevel;
	}

	CachedErrorLevel = EFlowYapErrorLevel::OK;
	return CachedErrorLevel;
}

// ================================================================================================
// HELPER API
// ================================================================================================

FFlowYapFragment* FPropertyCustomization_FlowYapFragment::GetFragment() const
{
	FFlowYapFragment* FragmentPtr = nullptr;
	
	const FStructProperty* StructProp = CastField<FStructProperty>(FragmentHandle->GetProperty());
	if (StructProp != nullptr && StructProp->Struct == FFlowYapFragment::StaticStruct())
	{
		void* Data = nullptr;
		
		if (FragmentHandle->GetValueData(Data) == FPropertyAccess::Success && Data != nullptr)
		{
			FragmentPtr = reinterpret_cast<FFlowYapFragment*>(Data);
		}
	}

	return FragmentPtr;
}

// ================================================================================================
// OVERRIDES
// ================================================================================================

bool FPropertyCustomization_FlowYapFragment::InErrorState() const
{
	FFlowYapFragment* Fragment = GetFragment();

	if (Fragment == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("omg"));
	}
	
	return !Fragment || !Dialogue.IsValid();
}

FSlateColor FPropertyCustomization_FlowYapFragment::GetNodeTitleColor() const
{
	FLinearColor Color;

	if (Dialogue.Get()->GetDynamicTitleColor(Color))
	{
		return Color;
	}

	return FLinearColor::Black;
}

void FPropertyCustomization_FlowYapFragment::CustomizeChildren(TSharedRef<IPropertyHandle> InPropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	
}

#undef LOCTEXT_NAMESPACE