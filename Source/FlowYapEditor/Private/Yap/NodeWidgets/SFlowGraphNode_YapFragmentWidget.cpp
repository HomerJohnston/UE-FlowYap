// Copyright Ghost Pepper Games, Inc. All Rights Reserved.


#define LOCTEXT_NAMESPACE "FlowYap"
#include "Yap/NodeWidgets/SFlowGraphNode_YapFragmentWidget.h"

#include "PropertyCustomizationHelpers.h"
#include "Logging/StructuredLog.h"
#include "Slate/DeferredCleanupSlateBrush.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "Widgets/Input/SSlider.h"
#include "Widgets/Notifications/SProgressBar.h"
#include "Widgets/Text/SRichTextBlock.h"
#include "Yap/FlowYapCharacter.h"
#include "Yap/FlowYapColors.h"
#include "Yap/FlowYapEditorSubsystem.h"
#include "Yap/FlowYapFragment.h"
#include "Yap/FlowYapInputTracker.h"
#include "Yap/FlowYapProjectSettings.h"
#include "Yap/FlowYapSubsystem.h"
#include "Yap/FlowYapTransactions.h"
#include "Yap/FlowYapUtil.h"
#include "Yap/YapEditorStyle.h"
#include "Yap/Enums/FlowYapErrorLevel.h"
#include "Yap/Nodes/FlowNode_YapDialogue.h"
#include "Yap/NodeWidgets/SFlowGraphNode_YapDialogueWidget.h"
#include "Yap/SlateWidgets/SGameplayTagComboFiltered.h"
#include "Yap/FlowYapBitReplacement.h"
#include "Yap/FlowYapCondition.h"
#include "Yap/Helpers/FlowYapWidgetHelper.h"

constexpr int32 PAD1 = 2;
constexpr int32 PAD2 = 4;

void SFlowGraphNode_YapFragmentWidget::Construct(const FArguments& InArgs, SFlowGraphNode_YapDialogueWidget* InOwner, uint8 InFragmentIndex)
{
	Owner = InOwner;
	FragmentIndex = InFragmentIndex;
	
	ChildSlot
	[
		CreateFragmentWidget()
	];
}

EVisibility SFlowGraphNode_YapFragmentWidget::BarAboveDialogue_Visibility() const
{
	return (ConditionWidgets_Visibility() != EVisibility::Visible && FragmentTagPreview_Visibility() != EVisibility::Visible) ? EVisibility::Collapsed : EVisibility::Visible;
}

int32 SFlowGraphNode_YapFragmentWidget::GetFragmentActivationCount() const
{
	return GetFragment()->GetActivationCount();
}

int32 SFlowGraphNode_YapFragmentWidget::GetFragmentActivationLimit() const
{
	return GetFragment()->GetActivationLimit();
}

// ================================================================================================
// FRAGMENT WIDGET
// ------------------------------------------------------------------------------------------------

TSharedRef<SWidget> SFlowGraphNode_YapFragmentWidget::CreateFragmentWidget()
{
	return SNew(SBox)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.VAlign(VAlign_Bottom)
		.HAlign(HAlign_Center)
		.AutoWidth()
		.Padding(0, 0, 0, 20)
		[
			SNew(SBox).WidthOverride(32)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(SActivationCounterWidget)
				.ActivationCount(this, &SFlowGraphNode_YapFragmentWidget::GetFragmentActivationCount)
				.ActivationLimit(this, &SFlowGraphNode_YapFragmentWidget::GetFragmentActivationLimit)
				.FontHeight(10)
			]
		]
		+ SHorizontalBox::Slot()
		.HAlign(HAlign_Fill)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, 0)
			[
				SNew(SBox)
				.Visibility(this, &SFlowGraphNode_YapFragmentWidget::BarAboveDialogue_Visibility)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.HAlign(HAlign_Fill)
					.Padding(0, 0, 0, 4)
					[
						CreateConditionWidgets()
					]
					+ SHorizontalBox::Slot()
					.HAlign(HAlign_Right)
					.AutoWidth()
					.Padding(0, 0, 0, 4)
					[
						FFlowYapWidgetHelper::CreateTagPreviewWidget(this, &SFlowGraphNode_YapFragmentWidget::FragmentTagPreview_Text, &SFlowGraphNode_YapFragmentWidget::FragmentTagPreview_Visibility)
					]
				]
			]
			+ SVerticalBox::Slot()
			.Padding(0, 0, 0, 0)
			.AutoHeight()
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
						// TODO this function eats up almost 2ms... optimize it 
						CreatePortraitWidget()
					]
					+ SOverlay::Slot()
					.VAlign(VAlign_Bottom)
					.HAlign(HAlign_Right)
					.Padding(0, 0, -4, -4)
					[
						SNew(SButton)
						.ButtonColorAndOpacity(YapColor::Transparent)
						.ForegroundColor(YapColor::Green)
						.ContentPadding(0)
						.ButtonStyle(FAppStyle::Get(), "SimpleButton")
						.Visibility(this, &SFlowGraphNode_YapFragmentWidget::AudioButton_Visibility)
						//.ToolTip(INVTEXT("Test"))
						[
							SNew(SOverlay)
							+ SOverlay::Slot()
							[
								SNew(SImage)
								.Image(FCoreStyle::Get().GetBrush("Icons.FilledCircle"))
								.DesiredSizeOverride(FVector2D(24, 24))
								.ColorAndOpacity(YapColor::DarkBlueGray)
							]
							+ SOverlay::Slot()
							.Padding(4,4)
							[
								SNew(SImage)
								.DesiredSizeOverride(FVector2D(16, 16))
								.Image(FYapEditorStyle::Get().GetBrush("ImageBrush.Icon.Audio"))
							]
						]
					]
				]
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				.FillWidth(1.0f)
				.Padding(2, 0, 0, 0)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.Padding(0, 0, 0, 0)
					.VAlign(VAlign_Fill)
					.HAlign(HAlign_Fill)
					[
						SNew(SBox)
						.ToolTipText(this, &SFlowGraphNode_YapFragmentWidget::Dialogue_ToolTipText)
						[
						SNew(SOverlay)
						+ SOverlay::Slot()
						.HAlign(HAlign_Fill)
						.VAlign(VAlign_Fill)
						[
							CreateDialogueWidget()
						]
						+ SOverlay::Slot()
						.VAlign(VAlign_Bottom)
						.Padding(0, 0, 0, 2)
						[
							SNew(SBox)
							.HeightOverride(2)
							.ToolTipText(this, &SFlowGraphNode_YapFragmentWidget::FragmentTimePadding_ToolTipText)
							[
								SNew(SProgressBar)
								.BorderPadding(0)
								.Percent(this, &SFlowGraphNode_YapFragmentWidget::FragmentTimePadding_Percent)
								.Style(FYapEditorStyle::Get(), "ProgressBarStyle.FragmentTimePadding")
								.FillColorAndOpacity(this, &SFlowGraphNode_YapFragmentWidget::FragmentTimePadding_FillColorAndOpacity)
								.BarFillType(EProgressBarFillType::FillFromCenterHorizontal)
							]
						]
						]
						
					]
					+ SVerticalBox::Slot()
					.Padding(0, 4, 0, 0)
					.AutoHeight()
					[
						CreateTitleTextWidget()
					]
				]
			]
		]
	];
}

FOptionalSize SFlowGraphNode_YapFragmentWidget::Fragment_WidthOverride() const
{
	return 360 + UFlowYapProjectSettings::Get()->GetDialogueWidthAdjustment();
}

EVisibility SFlowGraphNode_YapFragmentWidget::FragmentBottomSection_Visibility() const
{
	// Always show if there are errors!
	if (AudioAssetErrorLevel() != EFlowYapErrorLevel::OK)
	{
		return EVisibility::Visible;
	}
	
	if (FragmentFocused())
	{
		return EVisibility::Visible;
	}
	
	if (!Owner->GetIsSelected())
	{
		return EVisibility::Collapsed;
	}

	if (Owner->GetShiftHooked())
	{
		return EVisibility::Visible;
	}

	return EVisibility::Collapsed;
}

// ================================================================================================
// DIALOGUE WIDGET
// ------------------------------------------------------------------------------------------------

TSharedRef<SWidget> SFlowGraphNode_YapFragmentWidget::CreateDialogueWidget()
{
	return SNew(SBorder)
	.BorderImage(FYapEditorStyle::Get().GetBrush("ImageBrush.Box.SolidWhite.DeburredCorners"))
	.BorderBackgroundColor(YapColor::DeepGray)
	.ColorAndOpacity(YapColor::White)
	.Padding(0)
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
		[
			SNew(SMultiLineEditableTextBox)
			.Padding(FMargin(4, 2))
			.Style(FYapEditorStyle::Get(), "EditableTextBox.Dialogue")
			.Text(this, &SFlowGraphNode_YapFragmentWidget::Dialogue_Text)
			.OnTextCommitted(this, &SFlowGraphNode_YapFragmentWidget::Dialogue_OnTextCommitted)
			.RevertTextOnEscape(true)
			.ModiferKeyForNewLine(EModifierKey::Shift)
			.HintText(INVTEXT("<None>"))
		]
	];
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
	return GetFragment()->Bit.GetDialogueText();
}

void SFlowGraphNode_YapFragmentWidget::Dialogue_OnTextCommitted(const FText& CommittedText, ETextCommit::Type CommitType)
{
	FFlowYapTransactions::BeginModify(LOCTEXT("NodeDialogueTextChanged", "Dialogue Text Changed"), GetFlowYapDialogueNode());

	if (CommitType == ETextCommit::OnEnter)
	{
		GetFragment()->Bit.SetDialogueText(CommittedText);
	}

	FFlowYapTransactions::EndModify();
}

FText SFlowGraphNode_YapFragmentWidget::Dialogue_ToolTipText() const
{
	return LOCTEXT("DialogueText_Tooltip", "Spoken dialogue");
}

FSlateColor SFlowGraphNode_YapFragmentWidget::Dialogue_BackgroundColor() const
{
	if (GetFragment()->GetBitReplaced())
	{
		return YapColor::Yellow;
	}
	
	return GetFlowYapDialogueNode()->GetIsPlayerPrompt() ? YapColor::White : YapColor::Noir;
}

FSlateColor SFlowGraphNode_YapFragmentWidget::Dialogue_ForegroundColor() const
{
	FLinearColor Color = YapColor::Error;
	
	Color = GetFlowYapDialogueNode()->GetIsPlayerPrompt() ? YapColor::White : YapColor::LightGray;
	
	if (GEditor->PlayWorld)
	{
		if (GetFlowYapDialogueNode()->GetRunningFragmentIndex() == GetFragment()->IndexInDialogue)
		{
			Color = YapColor::White;
		}
		
		Color = (GetFragment()->IsActivationLimitMet()) ? YapColor::DarkRed : YapColor::LightGray;
	}

	if (GetFragment()->GetBitReplaced())
	{
		Color *= YapColor::LightBlue;
	}

	return Color;
}

EVisibility SFlowGraphNode_YapFragmentWidget::DialogueBackground_Visibility() const
{
	return GetFragment()->GetBitReplaced() ? EVisibility::Visible : EVisibility::Collapsed;
}

FSlateColor SFlowGraphNode_YapFragmentWidget::Dialogue_BorderBackgroundColor() const
{
	return YapColor::LightYellow_SuperGlass;
}

// ================================================================================================
// LOCAL ACTIVATION LIMITER WIDGET
// ------------------------------------------------------------------------------------------------

TSharedRef<SBox> SFlowGraphNode_YapFragmentWidget::CreateActivationLimiterWidget()
{
	TSharedRef<SVerticalBox> ActivationLimiter_VerticalBox = SNew(SVerticalBox);
	
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
			.Visibility(this, &SFlowGraphNode_YapFragmentWidget::ActivationDot_Visibility)
			[
				SNew(SButton)
				.ButtonStyle(FYapEditorStyle::Get(), "ButtonStyle.ActivationLimit")
				.ContentPadding(0)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				.OnClicked(this, &SFlowGraphNode_YapFragmentWidget::ActivationDot_OnClicked)
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
						.ColorAndOpacity(this, &SFlowGraphNode_YapFragmentWidget::ActivationDot_ColorAndOpacity)
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
			.Text(this, &SFlowGraphNode_YapFragmentWidget::ActivationLimiter_Text)
			.ColorAndOpacity(YapColor::White)
			//.TextStyle(&NormalText)
			.Justification(ETextJustify::Center)
		];
	}

	return SNew(SBox)
	.Visibility(this, &SFlowGraphNode_YapFragmentWidget::ActivationLimiter_Visibility)
	[
		ActivationLimiter_VerticalBox
	];
}

EVisibility SFlowGraphNode_YapFragmentWidget::ActivationLimiter_Visibility() const
{
	const FFlowYapFragment* Fragment = GetFragment();
	
	if (GEditor->PlayWorld)
	{
		return (Fragment->ActivationLimit > 0) ? EVisibility::Visible : EVisibility::Collapsed;		
	}
	else
	{
		if (IsHovered() || Fragment->ActivationLimit > 0)
		{
			return EVisibility::Visible;
		}
	}

	return EVisibility::Collapsed;
}

FText SFlowGraphNode_YapFragmentWidget::ActivationLimiter_Text() const
{
	return FText::Join
	(
		FText::FromString("/"),
		FText::AsNumber(GetFragment()->GetActivationCount()),
		FText::AsNumber(GetFragment()->GetActivationLimit())
	);
}

EVisibility SFlowGraphNode_YapFragmentWidget::ActivationDot_Visibility() const
{
	if (IsHovered() || GetFragment()->GetActivationLimit() > 0)
	{
		return EVisibility::Visible;
	}

	return EVisibility::Collapsed;
}

FSlateColor SFlowGraphNode_YapFragmentWidget::ActivationDot_ColorAndOpacity() const
{
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

FReply SFlowGraphNode_YapFragmentWidget::ActivationDot_OnClicked()
{
	FFlowYapFragment* Fragment = GetFragment();
	
	FFlowYapTransactions::BeginModify(LOCTEXT("Dialogue", "Change activation limit"), GetFlowYapDialogueNode());
	
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

FText SFlowGraphNode_YapFragmentWidget::FragmentTagPreview_Text() const
{
	// Pass tag from the properties

	FString Filter = GetFlowYapDialogueNode()->GetDialogueTag().ToString();

	FText Text = FText::FromString(FlowYapUtil::GetFilteredSubTag(Filter, GetFragment()->FragmentTag));

	if (Text.IsEmptyOrWhitespace())
	{
		return INVTEXT("");
	}
	else
	{
		return Text;
	}
}

EVisibility SFlowGraphNode_YapFragmentWidget::FragmentTagPreview_Visibility() const
{
	return GetFragment()->FragmentTag.IsValid() ? EVisibility::Visible : EVisibility::Collapsed;
}

TSharedRef<SWidget> SFlowGraphNode_YapFragmentWidget::CreateConditionWidgets() const
{
	TSharedRef<SScrollBox> Box = SNew(SScrollBox)
	.Visibility(this, &SFlowGraphNode_YapFragmentWidget::ConditionWidgets_Visibility)
	.ScrollBarVisibility(EVisibility::Collapsed)
	.ConsumeMouseWheel(EConsumeMouseWheel::Always)
	.AllowOverscroll(EAllowOverscroll::No)
	.AnimateWheelScrolling(true)
	.Orientation(Orient_Horizontal);

	for (const UFlowYapCondition* Condition : GetFragment()->GetConditions())
	{
		Box->AddSlot()
		.Padding(0, 0, 4, 0)
		[
			FFlowYapWidgetHelper::CreateConditionWidget(Condition)
		];	
	}
	
	return Box;
}

TSharedRef<SWidget> SFlowGraphNode_YapFragmentWidget::CreateConditionWidget(const UFlowYapCondition* Condition) const
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

EVisibility SFlowGraphNode_YapFragmentWidget::ConditionWidgets_Visibility() const
{
	return (GetFragment()->GetConditions().Num() > 0) ? EVisibility::Visible : EVisibility::Hidden;
}

// ================================================================================================
// FRAGMENT TIME PADDING WIDGET
// ------------------------------------------------------------------------------------------------

/*
TSharedRef<SWidget> SFlowGraphNode_YapFragmentWidget::CreateFragmentTimePaddingWidget()
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
				.Percent(this, &SFlowGraphNode_YapFragmentWidget::FragmentTimePadding_Percent)
				.Style(FYapEditorStyle::Get(), "ProgressBarStyle.FragmentTimePadding")
				.FillColorAndOpacity(this, &SFlowGraphNode_YapFragmentWidget::FragmentTimePadding_FillColorAndOpacity)
			]
		]
		+ SOverlay::Slot()
		.Padding(0, -2)
		[
			SNew(SSlider)
			.Visibility(this, &SFlowGraphNode_YapFragmentWidget::FragmentTimePaddingSlider_Visibility)
			.Value(this, &SFlowGraphNode_YapFragmentWidget::FragmentTimePadding_Value)
			.OnValueChanged(this, &SFlowGraphNode_YapFragmentWidget::FragmentTimePadding_OnValueChanged)
			.Style(FYapEditorStyle::Get(), "SliderStyle.FragmentTimePadding")
			.SliderHandleColor(YapColor::Gray)
			.ToolTipText(this, &SFlowGraphNode_YapFragmentWidget::FragmentTimePadding_ToolTipText)
		]
	];
}
*/

TOptional<float> SFlowGraphNode_YapFragmentWidget::FragmentTimePadding_Percent() const
{
	const float MaxPaddedSetting =  UFlowYapProjectSettings::Get()->GetFragmentPaddingSliderMax();
	const float FragmentPadding = GetFragment()->GetPaddingToNextFragment();

	if (GEditor->PlayWorld)
	{
		const TOptional<uint8>& RunningIndex = GetFlowYapDialogueNode()->RunningFragmentIndex;

		if (!RunningIndex.IsSet() || RunningIndex.GetValue() < GetFragment()->IndexInDialogue)
		{
			return FragmentPadding / MaxPaddedSetting;
		}

		if (RunningIndex == GetFragment()->IndexInDialogue)
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

float SFlowGraphNode_YapFragmentWidget::FragmentTimePadding_Value() const
{
	const float MaxPaddedSetting =  UFlowYapProjectSettings::Get()->GetFragmentPaddingSliderMax();

	return GetFragment()->GetPaddingToNextFragment() / MaxPaddedSetting;
}

EVisibility SFlowGraphNode_YapFragmentWidget::FragmentTimePaddingSlider_Visibility() const
{
	if (GEditor->PlayWorld)
	{
		return EVisibility::Hidden;
	}

	return EVisibility::Visible;
}

void SFlowGraphNode_YapFragmentWidget::FragmentTimePadding_OnValueChanged(float X)
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

FSlateColor SFlowGraphNode_YapFragmentWidget::FragmentTimePadding_FillColorAndOpacity() const
{
	const float MaxPaddedSetting =  UFlowYapProjectSettings::Get()->GetFragmentPaddingSliderMax();

	if (GetFragment()->GetPaddingToNextFragment() > MaxPaddedSetting)
	{
		return YapColor::Blue_Trans;
	}
	
	if (GEditor->PlayWorld)
	{
		const TOptional<uint8>& RunningIndex = GetFlowYapDialogueNode()->RunningFragmentIndex;

		if (!RunningIndex.IsSet() || RunningIndex.GetValue() < GetFragment()->IndexInDialogue)
		{
		}

		if (RunningIndex == GetFragment()->IndexInDialogue)
		{
			return YapColor::White_Trans;
		}

		return YapColor::DarkGray_Trans;
	}

	return GetFragment()->GetCommonPaddingSetting().IsSet() ? YapColor::DimGray_Trans : YapColor::LightBlue_Trans;
}

FText SFlowGraphNode_YapFragmentWidget::FragmentTimePadding_ToolTipText() const
{
	return FText::Format(LOCTEXT("Fragment", "Delay: {0}"), GetFragment()->GetPaddingToNextFragment());
}

FSlateColor SFlowGraphNode_YapFragmentWidget::PortraitImage_BorderBackgroundColor() const
{
	return GetFlowYapDialogueNode()->GetIsPlayerPrompt() ? YapColor::LightGray_Trans : YapColor::Gray_Glass;
}

// ================================================================================================
// PORTRAIT WIDGET
// ------------------------------------------------------------------------------------------------

TSharedRef<SWidget> SFlowGraphNode_YapFragmentWidget::CreatePortraitWidget()
{
	return SNew(SOverlay)
	.Visibility(this, &SFlowGraphNode_YapFragmentWidget::PortraitImage_Visibility)
	+ SOverlay::Slot()
	.Padding(0, 0, 0, 0)
	[
		SNew(SBox)
		.WidthOverride(68)
		.HeightOverride(68)
		[
			SNew(SBorder)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			.BorderImage(FYapEditorStyle::Get().GetBrush("ImageBrush.Border.DeburredSquare"))
			.BorderBackgroundColor(this, &SFlowGraphNode_YapFragmentWidget::PortraitImage_BorderBackgroundColor)
		]
	]
	+ SOverlay::Slot()
	[
		SNew(SOverlay)
		+ SOverlay::Slot()
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Center)
		[
			SNew(SImage)
			.DesiredSizeOverride(FVector2D(64, 64))
			.Image(this, &SFlowGraphNode_YapFragmentWidget::PortraitImage_Image)
		]
		+ SOverlay::Slot()
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Center)
		[
			SNew(STextBlock)
			.Visibility(this, &SFlowGraphNode_YapFragmentWidget::MissingPortraitWarning_Visibility)
			.Text(LOCTEXT("FragmentCharacterMissing", "?"))
			.Justification(ETextJustify::Center)
		]
	];
}

EVisibility SFlowGraphNode_YapFragmentWidget::PortraitImage_Visibility() const
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

const FSlateBrush* SFlowGraphNode_YapFragmentWidget::PortraitImage_Image() const
{
	return GetFragment()->GetBit().GetSpeakerPortraitBrush();
}

EVisibility SFlowGraphNode_YapFragmentWidget::MissingPortraitWarning_Visibility() const
{
	const FSlateBrush* Brush = GetFragment()->GetBit().GetSpeakerPortraitBrush();

	if (Brush)
	{
		return (Brush->GetResourceObject()) ? EVisibility::Hidden : EVisibility::Visible;
	}
	
	return EVisibility::Visible;
}

EVisibility SFlowGraphNode_YapFragmentWidget::CharacterSelect_Visibility() const
{
	return IsHovered() ? EVisibility::Visible : EVisibility::Collapsed;
}

FString SFlowGraphNode_YapFragmentWidget::CharacterSelect_ObjectPath() const
{
	const TSoftObjectPtr<UFlowYapCharacter> Asset = GetFragment()->Bit.GetCharacterAsset();

	if (Asset.IsPending())
	{
		Asset.LoadSynchronous();
	}
	
	if (!Asset) { return ""; }

	return Asset.ToString();
}

void SFlowGraphNode_YapFragmentWidget::CharacterSelect_OnObjectChanged(const FAssetData& InAssetData)
{

	UObject* Asset = InAssetData.GetAsset();

	UFlowYapCharacter* Character = Cast<UFlowYapCharacter>(Asset);

	if (Character)
	{
		FFlowYapTransactions::BeginModify(LOCTEXT("NodeCharacterChanged", "Character Changed"), GetFlowYapDialogueNode());

		GetFragment()->Bit.SetCharacter(Character);

		FFlowYapTransactions::EndModify();
	}

}

// ================================================================================================
// MOOD KEY SELECTOR WIDGET
// ------------------------------------------------------------------------------------------------

TSharedRef<SBox> SFlowGraphNode_YapFragmentWidget::CreateMoodKeySelectorWidget()
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
	.Visibility(this, &SFlowGraphNode_YapFragmentWidget::MoodKeySelector_Visibility)
	[
		SNew(SComboButton)
		.HasDownArrow(false)
		.ContentPadding(FMargin(0.f, 0.f))
		.MenuPlacement(MenuPlacement_CenteredBelowAnchor)
		.ButtonColorAndOpacity(FSlateColor(FLinearColor(0.f, 0.f, 0.f, 0.75f)))
		.HAlign(HAlign_Center)
		.ButtonStyle(FAppStyle::Get(), "SimpleButton")
		.OnMenuOpenChanged(this, &SFlowGraphNode_YapFragmentWidget::MoodKeySelector_OnMenuOpenChanged)
		.ToolTipText(LOCTEXT("Fragment", "Choose a mood key"))
		.ButtonContent()
		[
			SNew(SBox)
			.Padding(4, 4)
			[
				SAssignNew(PortraitIconImage, SImage)
				.ColorAndOpacity(FSlateColor::UseForeground())
				.Image(this, &SFlowGraphNode_YapFragmentWidget::MoodKeyBrush_GetBrush)
			]
		]
		.MenuContent()
		[
			MenuBuilder.MakeWidget()
		]
	];
	
	return Box.ToSharedRef();
}

EVisibility SFlowGraphNode_YapFragmentWidget::MoodKeySelector_Visibility() const
{
	return IsHovered() || MoodKeySelectorMenuOpen ? EVisibility::Visible : EVisibility::Collapsed;
}

void SFlowGraphNode_YapFragmentWidget::MoodKeySelector_OnMenuOpenChanged(bool bMenuOpen)
{
	MoodKeySelectorMenuOpen = bMenuOpen;
}

const FSlateBrush* SFlowGraphNode_YapFragmentWidget::MoodKeyBrush_GetBrush() const
{
	return GEditor->GetEditorSubsystem<UFlowYapEditorSubsystem>()->GetMoodKeyBrush(GetCurrentMoodKey());
}

FName SFlowGraphNode_YapFragmentWidget::GetCurrentMoodKey() const
{
	return GetFragment()->Bit.GetMoodKey();
}

// ================================================================================================
// MOOD KEY MENU ENTRY WIDGET
// ------------------------------------------------------------------------------------------------

TSharedRef<SWidget> SFlowGraphNode_YapFragmentWidget::CreateMoodKeyMenuEntryWidget(FName InIconName, bool bSelected, const FText& InLabel, FName InTextStyle)
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
	.OnClicked(this, &SFlowGraphNode_YapFragmentWidget::MoodKeyMenuEntry_OnClicked, InIconName)
	[
		SAssignNew(PortraitIconImage, SImage)
		.ColorAndOpacity(FSlateColor::UseForeground())
		.Image(TAttribute<const FSlateBrush*>::Create(TAttribute<const FSlateBrush*>::FGetter::CreateLambda([MoodKeyBrush](){return MoodKeyBrush->GetSlateBrush();})))
	];
}

FReply SFlowGraphNode_YapFragmentWidget::MoodKeyMenuEntry_OnClicked(FName NewValue)
{
	FFlowYapTransactions::BeginModify(LOCTEXT("NodeMoodKeyChanged", "Portrait Key Changed"), GetFlowYapDialogueNode());

	GetFragment()->Bit.SetMoodKey(NewValue);

	FFlowYapTransactions::EndModify();

	return FReply::Handled();
}



// ================================================================================================
// TITLE TEXT WIDGET
// ------------------------------------------------------------------------------------------------

TSharedRef<SWidget> SFlowGraphNode_YapFragmentWidget::CreateTitleTextWidget()
{
	return SNew(SBox)
	.Visibility(this, &SFlowGraphNode_YapFragmentWidget::TitleText_Visibility)
	.ToolTipText(LOCTEXT("TitleText_Tooltip", "Title text"))
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
		[
			SNew(SMultiLineEditableTextBox)
			.Padding(FMargin(4, 2))
			.Style(FYapEditorStyle::Get(), "EditableTextBox.TitleText")
			.Text(this, &SFlowGraphNode_YapFragmentWidget::TitleText_Text)
			.OnTextCommitted(this, &SFlowGraphNode_YapFragmentWidget::TitleText_OnTextCommitted)
			.RevertTextOnEscape(true)
			.ModiferKeyForNewLine(EModifierKey::Shift)
			.HintText(INVTEXT("<None>"))
		]
		//CreateWrappedTextBlock(&SFlowGraphNode_YapFragmentWidget::TitleText_Text, "Text.TitleText")
	];
}

EVisibility SFlowGraphNode_YapFragmentWidget::TitleText_Visibility() const
{
	if (GetFlowYapDialogueNode()->GetIsPlayerPrompt())
	{
		return EVisibility::Visible;
	}
	
	return UFlowYapProjectSettings::Get()->GetHideTitleTextOnNPCDialogueNodes() ? EVisibility::Collapsed : EVisibility::Visible;
}

FText SFlowGraphNode_YapFragmentWidget::TitleText_Text() const
{
	return GetFragment()->Bit.GetTitleText();
}

void SFlowGraphNode_YapFragmentWidget::TitleText_OnTextCommitted(const FText& CommittedText, ETextCommit::Type CommitType)
{
	FFlowYapTransactions::BeginModify(LOCTEXT("NodeTitleTextChanged", "Title Text Changed"), GetFlowYapDialogueNode());
	
	if (CommitType == ETextCommit::OnEnter || CommitType == ETextCommit::OnUserMovedFocus)
	{
		GetFragment()->Bit.SetTitleText(CommittedText);
	}

	FFlowYapTransactions::EndModify();
}

// ================================================================================================
// FRAGMENT TAG WIDGET
// ------------------------------------------------------------------------------------------------

TSharedRef<SWidget> SFlowGraphNode_YapFragmentWidget::CreateFragmentTagWidget()
{
	const FString FilterString = GetFlowYapDialogueNode()->GetDialogueTag().ToString();

	return SNew(SGameplayTagComboFiltered)
	.Tag(this, &SFlowGraphNode_YapFragmentWidget::FragmentTag_Tag)
	.Filter(FilterString)
	.OnTagChanged(this, &SFlowGraphNode_YapFragmentWidget::FragmentTag_OnTagChanged);
}

FGameplayTag SFlowGraphNode_YapFragmentWidget::FragmentTag_Tag() const
{
	return GetFragment()->FragmentTag;
}

void SFlowGraphNode_YapFragmentWidget::FragmentTag_OnTagChanged(FGameplayTag GameplayTag)
{
	FFlowYapTransactions::BeginModify(LOCTEXT("Fragment", "Change Fragment Tag"), GetFlowYapDialogueNode());

	GetFragment()->FragmentTag = GameplayTag;

	FFlowYapTransactions::EndModify();
}

// ================================================================================================
// BOTTOM ROW WIDGET (AUDIO ASSET, TIME SETTINGS)
// ------------------------------------------------------------------------------------------------

TSharedRef<SBox> SFlowGraphNode_YapFragmentWidget::CreateBottomRowWidget()
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
					.IsChecked(this, &SFlowGraphNode_YapFragmentWidget::UseProjectDefaultTimeSettingsButton_IsChecked)
					.OnCheckStateChanged(this, &SFlowGraphNode_YapFragmentWidget::UseProjectDefaultTimeSettingsButton_OnCheckStateChanged)
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
					.IsEnabled(this, &SFlowGraphNode_YapFragmentWidget::UseManuallyEnteredTimeButton_IsEnabled)
					.IsChecked(this, &SFlowGraphNode_YapFragmentWidget::UseManuallyEnteredTimeButton_IsChecked)
					.OnCheckStateChanged(this, &SFlowGraphNode_YapFragmentWidget::UseManuallyEnteredTimeButton_OnCheckStateChanged)
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
					.IsEnabled(this, &SFlowGraphNode_YapFragmentWidget::UseTextTimeButton_IsEnabled)
					.IsChecked(this, &SFlowGraphNode_YapFragmentWidget::UseTextTimeButton_IsChecked)
					.OnCheckStateChanged(this, &SFlowGraphNode_YapFragmentWidget::UseTextTimeButton_OnCheckStateChanged)
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
					.IsEnabled(this, &SFlowGraphNode_YapFragmentWidget::UseAudioTimeButton_IsEnabled)
					.IsChecked(this, &SFlowGraphNode_YapFragmentWidget::UseAudioTimeButton_IsChecked)
					.OnCheckStateChanged(this, &SFlowGraphNode_YapFragmentWidget::UseAudioTimeButton_OnCheckStateChanged)
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
						.IsEnabled(this, &SFlowGraphNode_YapFragmentWidget::TimeEntryBox_IsEnabled)
						.Delta(0.1)
						.MinValue(0.0)
						.ToolTipText(LOCTEXT("FragmentTimeEntry_Tooltip", "Time this dialogue fragment will play for"))
						.Justification(ETextJustify::Center)
						.Value(this, &SFlowGraphNode_YapFragmentWidget::TimeEntryBox_Value)
						.OnValueCommitted(this, &SFlowGraphNode_YapFragmentWidget::TimeEntryBox_OnValueCommitted)
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
ECheckBoxState SFlowGraphNode_YapFragmentWidget::UseProjectDefaultTimeSettingsButton_IsChecked() const
{
	return GetFragment()->Bit.GetUseProjectDefaultTimeSettings() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SFlowGraphNode_YapFragmentWidget::UseProjectDefaultTimeSettingsButton_OnCheckStateChanged(ECheckBoxState CheckBoxState)
{
	FFlowYapTransactions::BeginModify(LOCTEXT("NodeUseProjectDefaultTimeSettings", "Use Project Default Time Settings Changed"), GetFlowYapDialogueNode());

	GetFragment()->Bit.SetUseProjectDefaultSettings(CheckBoxState == ECheckBoxState::Checked);
	
	FFlowYapTransactions::EndModify();
}

// ---------------------
bool SFlowGraphNode_YapFragmentWidget::UseManuallyEnteredTimeButton_IsEnabled() const
{
	return GetFragment()->Bit.GetUseProjectDefaultTimeSettings() ? false : true;
}

ECheckBoxState SFlowGraphNode_YapFragmentWidget::UseManuallyEnteredTimeButton_IsChecked() const
{
	return GetFragment()->Bit.GetTimeMode() == EFlowYapTimeMode::ManualTime ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SFlowGraphNode_YapFragmentWidget::UseManuallyEnteredTimeButton_OnCheckStateChanged(ECheckBoxState CheckBoxState)
{
	FFlowYapTransactions::BeginModify(LOCTEXT("Fragment", "Fragment Time Mode Changed"), GetFlowYapDialogueNode());

	if (CheckBoxState == ECheckBoxState::Checked)
	{
		GetFragment()->Bit.SetBitTimeMode(EFlowYapTimeMode::ManualTime);
	}

	FFlowYapTransactions::EndModify();
}

// ---------------------
bool SFlowGraphNode_YapFragmentWidget::UseTextTimeButton_IsEnabled() const
{
	return GetFragment()->Bit.GetUseProjectDefaultTimeSettings() ? false : true;
}

ECheckBoxState SFlowGraphNode_YapFragmentWidget::UseTextTimeButton_IsChecked() const
{
	return GetFragment()->Bit.GetTimeMode() == EFlowYapTimeMode::TextTime ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SFlowGraphNode_YapFragmentWidget::UseTextTimeButton_OnCheckStateChanged(ECheckBoxState CheckBoxState)
{
	FFlowYapTransactions::BeginModify(LOCTEXT("Fragment", "Fragment Time Mode Changed"), GetFlowYapDialogueNode());

	if (CheckBoxState == ECheckBoxState::Checked)
	{
		GetFragment()->Bit.SetBitTimeMode(EFlowYapTimeMode::TextTime);
	}
	
	FFlowYapTransactions::EndModify();
}

// ---------------------
bool SFlowGraphNode_YapFragmentWidget::UseAudioTimeButton_IsEnabled() const
{
	return !GetFragment()->Bit.GetUseProjectDefaultTimeSettings();
}

ECheckBoxState SFlowGraphNode_YapFragmentWidget::UseAudioTimeButton_IsChecked() const
{
	return GetFragment()->Bit.GetTimeMode() == EFlowYapTimeMode::AudioTime ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SFlowGraphNode_YapFragmentWidget::UseAudioTimeButton_OnCheckStateChanged(ECheckBoxState CheckBoxState)
{
	FFlowYapTransactions::BeginModify(LOCTEXT("NodeTimedModeChanged", "Timed Mode Changed"), GetFlowYapDialogueNode());

	if (CheckBoxState == ECheckBoxState::Checked)
	{
		GetFragment()->Bit.SetBitTimeMode(EFlowYapTimeMode::AudioTime);
	}
	
	FFlowYapTransactions::EndModify();
}

bool SFlowGraphNode_YapFragmentWidget::TimeEntryBox_IsEnabled() const
{
	return GetFragment()->Bit.GetTimeMode() == EFlowYapTimeMode::ManualTime;
}

TOptional<double> SFlowGraphNode_YapFragmentWidget::TimeEntryBox_Value() const
{
	double Time = GetFragment()->Bit.GetTime();
	
	return (Time > 0) ? Time : TOptional<double>();
}

void SFlowGraphNode_YapFragmentWidget::TimeEntryBox_OnValueCommitted(double NewValue, ETextCommit::Type CommitType)
{
	FFlowYapTransactions::BeginModify(LOCTEXT("NodeEnteredTimeChanged", "Entered Time Changed"), GetFlowYapDialogueNode());

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

TSharedRef<SWidget> SFlowGraphNode_YapFragmentWidget::CreateAudioAssetWidget()
{
	double StartTime = FPlatformTime::Seconds();

#ifdef FLOWYAP_DISPLAY_DIALOGUEASSETPICKER
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
		.ObjectPath(this, &SFlowGraphNode_YapFragmentWidget::AudioAsset_ObjectPath)
		.OnObjectChanged(this, &SFlowGraphNode_YapFragmentWidget::AudioAsset_OnObjectChanged)
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
		.Visibility(this, &SFlowGraphNode_YapFragmentWidget::AudioAssetErrorState_Visibility)
		.ColorAndOpacity(this, &SFlowGraphNode_YapFragmentWidget::AudioAssetErrorState_ColorAndOpacity)
	];
	#else
	TSharedRef<SWidget> Widget = SNew(STextBlock)
	.Text(this, &SFlowGraphNode_YapFragmentWidget::AudioAsset_ObjectPathText);
	#endif
	
	double EndTime = FPlatformTime::Seconds();

	//UE_LOG(LogTemp, Warning, TEXT("CreateAudioAssetWidget took %f ms"), 1000 * (EndTime - StartTime));

	return Widget;
}

FText SFlowGraphNode_YapFragmentWidget::AudioAsset_ObjectPathText() const
{
	return FText::FromString(AudioAsset_ObjectPath());
}

FString SFlowGraphNode_YapFragmentWidget::AudioAsset_ObjectPath() const
{
	const TSoftObjectPtr<UObject> Asset = GetFragment()->Bit.GetDialogueAudioAsset<UObject>();

	if (!Asset) { return ""; }

	return Asset.ToString();
}

void SFlowGraphNode_YapFragmentWidget::AudioAsset_OnObjectChanged(const FAssetData& InAssetData)
{
	FFlowYapTransactions::BeginModify(LOCTEXT("NodeAudioAssetChanged", "Audio Asset Changed"), GetFlowYapDialogueNode());

	GetFragment()->Bit.SetDialogueAudioAsset(InAssetData.GetAsset());

	FFlowYapTransactions::EndModify();
}

EVisibility SFlowGraphNode_YapFragmentWidget::AudioAssetErrorState_Visibility() const
{
	if (AudioAssetErrorLevel() != EFlowYapErrorLevel::OK)
	{
		return EVisibility::HitTestInvisible;
	}
	
	return EVisibility::Hidden;
}

FSlateColor SFlowGraphNode_YapFragmentWidget::AudioAssetErrorState_ColorAndOpacity() const
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

EFlowYapErrorLevel SFlowGraphNode_YapFragmentWidget::AudioAssetErrorLevel() const
{
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

EVisibility SFlowGraphNode_YapFragmentWidget::AudioButton_Visibility() const
{
	return GetFragment()->GetBit().HasDialogueAudioAsset() ? EVisibility::Visible : EVisibility::Collapsed;
}

// ================================================================================================
// HELPER API
// ================================================================================================

UFlowNode_YapDialogue* SFlowGraphNode_YapFragmentWidget::GetFlowYapDialogueNode() const
{
	return Owner->GetFlowYapDialogueNodeMutable();
}

FFlowYapFragment* SFlowGraphNode_YapFragmentWidget::GetFragment() const
{
	return GetFlowYapDialogueNode()->GetFragmentByIndexMutable(FragmentIndex);
}

bool SFlowGraphNode_YapFragmentWidget::FragmentFocused() const
{
	uint8 FocusedFragmentIndex;
	return (Owner->GetFocusedFragmentIndex(FocusedFragmentIndex) && FocusedFragmentIndex == FragmentIndex);
}

TSharedRef<SWidget> SFlowGraphNode_YapFragmentWidget::CreateWrappedTextBlock(FText(SFlowGraphNode_YapFragmentWidget::*TextDelegate)() const, FString TextStyle) const
{
	TSharedRef<SScrollBar> HScrollBar = SNew(SScrollBar)
	.Orientation(Orient_Horizontal)
	.Thickness(this, &SFlowGraphNode_YapFragmentWidget::DialogueScrollBar_Thickness)
	.Padding(-1);
	
	TSharedRef<SScrollBar> VScrollBar = SNew(SScrollBar)
	.Orientation(Orient_Vertical)
	.Thickness(this, &SFlowGraphNode_YapFragmentWidget::DialogueScrollBar_Thickness)
	.Padding(-1);
	
	return SNew(SBorder)
	.BorderImage(FYapEditorStyle::Get().GetBrush("ImageBrush.Box.SolidWhite.DeburredCorners"))
	.BorderBackgroundColor(YapColor::DeepGray)
	.Padding(4, 2, 4, 2)
	.ColorAndOpacity(YapColor::White)//this, &SFlowGraphNode_YapFragmentWidget::FragmentTagPreview_ColorAndOpacity)
	[
		SNew(STextBlock)
		.Text(this, TextDelegate)
		.TextStyle( FYapEditorStyle::Get(), *TextStyle )
		.WrappingPolicy(ETextWrappingPolicy::DefaultWrapping)
		.AutoWrapText(false)
		//.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
		//.HScrollBar(HScrollBar)
		//.VScrollBar(VScrollBar)
		//.AlwaysShowScrollbars(true)

		/*
		SNew(SMultiLineEditableText)
		.IsEnabled(false)
		.Text(this, wtf)
		.TextStyle(FYapEditorStyle::Get(), *TextStyle)
		*/
	];
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
	/*
	bCtrlPressed = GEditor->GetEditorSubsystem<UFlowYapEditorSubsystem>()->GetInputTracker()->GetControlPressed();

	if (DialogueBox->HasKeyboardFocus() || TitleTextBox->HasKeyboardFocus())
	{
		Owner->SetFocusedFragmentIndex(GetFragment()->IndexInDialogue);
	}
	else if (Owner->GetFocusedFragmentIndex() == GetFragment()->IndexInDialogue)
	{
		Owner->ClearTypingFocus();
	}
	*/
}

#undef LOCTEXT_NAMESPACE