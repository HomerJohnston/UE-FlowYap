// Copyright Ghost Pepper Games, Inc. All Rights Reserved.

#include "Widgets/SFlowGraphNode_YapFragmentWidget.h"

#include "FlowYapColors.h"
#include "PropertyCustomizationHelpers.h"
#include "FlowYap/FlowYapProjectSettings.h"
#include "FlowYap/Nodes/FlowNode_YapDialogue.h"
#include "Slate/DeferredCleanupSlateBrush.h"
#include "FlowYapEditorSubsystem.h"
#include "FlowYapInputTracker.h"
#include "FlowYapTransactions.h"
#include "FlowYap/FlowYapCharacter.h"
#include "FlowYap/FlowYapLog.h"
#include "Widgets/SFlowGraphNode_YapDialogueWidget.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "Widgets/Layout/SSeparator.h"

#define LOCTEXT_NAMESPACE "FlowYap"

void SFlowGraphNode_YapFragmentWidget::Construct(const FArguments& InArgs, SFlowGraphNode_YapDialogueWidget* InOwner, FFlowYapFragment* InFragment)
{
	Owner = InOwner;

	Fragment = InFragment;
	
	DialogueAssetClass = GetDefault<UFlowYapProjectSettings>()->GetDialogueAssetClass();
	
	ChildSlot
	[
		CreateDialogueContentArea()
	];
}

EVisibility SFlowGraphNode_YapFragmentWidget::GetPortraitWidgetVisibility() const
{
	if (!Owner->GetIsSelected())
	{
		return EVisibility::Visible;
	}
	
	if (Owner->GetFocusedFragment() == this)
	{
		return EVisibility::Collapsed;
	}

	/*
	if (Owner->GetControlHooked())
	{
		return EVisibility::Collapsed;
	}
	*/

	return EVisibility::Visible;
}

EVisibility SFlowGraphNode_YapFragmentWidget::GetTitleTextEntryVisibility() const
{
	if (GetFlowNodeYapDialogue()->GetIsPlayerPrompt())
	{
		return EVisibility::Visible;
	}
	
	return GetDefault<UFlowYapProjectSettings>()->GetHideTitleTextOnNPCDialogueNodes() ? EVisibility::Collapsed : EVisibility::Visible;
}

FReply SFlowGraphNode_YapFragmentWidget::OnClickDialogueTextBox()
{
	UE_LOG(LogTemp, Warning, TEXT("Hello World"));
	return FReply::Unhandled();
}

TSharedRef<SWidget> SFlowGraphNode_YapFragmentWidget::CreateDialogueContentArea()
{	
	return SNew(SBox)
	.WidthOverride(this, &SFlowGraphNode_YapFragmentWidget::GetDialogueWidgetWidthAdjustment)
	[
		SNew(SVerticalBox)
		// ===================
		// TOP (PORTRAIT | DIALOGUE)
		// ===================
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SBox)
			.MinDesiredHeight(85)
			[
				SNew(SHorizontalBox)
				// ===================
				// DIALOGUE (LEFT SIDE)
				// ===================
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				.FillWidth(1.0f)
				.Padding(0.f, 5.0f, 0.0f, 5.0f)
				[
					SNew(SBox)
					.MaxDesiredHeight(this, &SFlowGraphNode_YapFragmentWidget::GetMaxDialogueEditableTextWidgetHeight)
					.Padding(0, 1, 0, 0)
					[
						SAssignNew(DialogueBox, SMultiLineEditableTextBox)
						.Text(this, &SFlowGraphNode_YapFragmentWidget::GetDialogueText)
						.ModiferKeyForNewLine(EModifierKey::Shift)
						.OnTextCommitted(this, &SFlowGraphNode_YapFragmentWidget::HandleDialogueTextCommitted)
						.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
						.HintText(LOCTEXT("DialogueText_Hint", "Enter dialogue text"))
						.ToolTipText(LOCTEXT("DialogueText_Tooltip", "To be displayed during speaking"))
						.Margin(FMargin(0,0,0,0))
						.Padding(3)
						.BackgroundColor(this, &SFlowGraphNode_YapFragmentWidget::GetDialogueTextBackgroundColor)
						.ForegroundColor(this, &SFlowGraphNode_YapFragmentWidget::GetDialogueTextColor)
					]
				]
				// ===================
				// PORTRAIT IMAGE (RIGHT SIDE)
				// ===================
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Top)
				.AutoWidth()
				.Padding(2, 5, 1, 5)
				[
					SNew(SOverlay)
					.Visibility(this, &SFlowGraphNode_YapFragmentWidget::GetPortraitWidgetVisibility)
					+ SOverlay::Slot()
					.Padding(2, 1, 0, 0)
					[
						CreatePortraitWidget()
					]
					+ SOverlay::Slot()
					.VAlign(VAlign_Bottom)
					.HAlign(HAlign_Right)
					.Padding(0, 0, 2, 2)
					[
						CreatePortraitKeySelector()
					]	
				]
			]
		]
		// ===================
		// BOTTOM (TITLE TEXT ABOVE, AUDIO | SETTINGS BELOW)
		// ===================
		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Fill)
		.Padding(0, 0, 0, 4)
		[
			SNew(SBox)
			.WidthOverride(400)
			//.HeightOverride(52)
			[
				SNew(SVerticalBox)
				.Visibility(this, &SFlowGraphNode_YapFragmentWidget::DisplayAllLowerFragmentControls)
				// ===================
				// TITLE TEXT
				// ===================
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Fill)
				.Padding(0, 0, 0, 2)
				[
					SAssignNew(TitleTextBox, SEditableTextBox)
					.Visibility(this, &SFlowGraphNode_YapFragmentWidget::GetTitleTextEntryVisibility)
					.Text(this, &SFlowGraphNode_YapFragmentWidget::GetTitleText)
					.OnTextCommitted(this, &SFlowGraphNode_YapFragmentWidget::HandleTitleTextCommitted)
					.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
					.Padding(2)
					.HintText(LOCTEXT("TitleText_Hint", "Enter optional title text"))
					.ToolTipText(LOCTEXT("TitleText_Tooltip", "Title text may be used to build player's dialogue selection list."))
				]
				// ===================
				// AUDIO ASSET SELECTOR
				// ===================
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0, 2, 0, 2)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.Padding(0, 2, 0, 0)
					[
						SNew(SOverlay)
						+ SOverlay::Slot()
						.HAlign(HAlign_Fill)
						.VAlign(VAlign_Fill)
						[
							SAssignNew(AudioAssetProperty, SObjectPropertyEntryBox)
							.DisplayBrowse(true)
							.DisplayUseSelected(false)
							.DisplayThumbnail(false)
							.AllowedClass(DialogueAssetClass)
							.EnableContentPicker(true)
							.ObjectPath(this, &SFlowGraphNode_YapFragmentWidget::GetSelectedDialogueAudioAssetPath)
							.OnObjectChanged(this, &SFlowGraphNode_YapFragmentWidget::HandleDialogueAudioAssetChanged)
							.ToolTipText(LOCTEXT("DialogueAudioAsset_Tooltip", "Select an audio asset."))
						]
						+ SOverlay::Slot()
						.HAlign(HAlign_Fill)
						.VAlign(VAlign_Fill)
						[
							SNew(SImage)
							.Visibility(this, &SFlowGraphNode_YapFragmentWidget::GetUseTimeFromAudioButtonErrorState)
							.Image(FAppStyle::GetBrush("MarqueeSelection"))
							.ColorAndOpacity(FlowYapColor::Orange)
						]
						+ SOverlay::Slot()
						.HAlign(HAlign_Fill)
						.VAlign(VAlign_Fill)
						[
							SNew(SImage)
							.Visibility(this, &SFlowGraphNode_YapFragmentWidget::GetSelectedDialogueAudioAssetIsValid)
							.Image(FAppStyle::GetBrush("MarqueeSelection"))
							.ColorAndOpacity(FLinearColor::Red)
						]
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
					// ===================
					// ADDITIONAL OPTIONS
					// ===================
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.HAlign(HAlign_Right)
					.Padding(6,2,0,0)
					[
						CreateTimeSettingsWidget()
					]
				]
			]
		]
	];
}

FOptionalSize SFlowGraphNode_YapFragmentWidget::GetDialogueWidgetWidthAdjustment() const
{
	return 360 + GetDefault<UFlowYapProjectSettings>()->GetDialogueWidthAdjustment();
}

FSlateColor SFlowGraphNode_YapFragmentWidget::GetDialogueTextColor() const
{
	return GetFlowNodeYapDialogue()->GetIsPlayerPrompt() ? FlowYapColor::White : FlowYapColor::LightGray;
}

FSlateColor SFlowGraphNode_YapFragmentWidget::GetDialogueTextBackgroundColor() const
{
	return GetFlowNodeYapDialogue()->GetIsPlayerPrompt() ? FlowYapColor::White : FlowYapColor::Noir;
}

TSharedRef<SBox> SFlowGraphNode_YapFragmentWidget::CreatePortraitWidget()
{
	TSharedPtr<SBox> PortraitPreview;
	
	SAssignNew(PortraitPreview, SBox)
	.WidthOverride(74)
	.HeightOverride(74)
	[
		SNew(SBorder)
		//.BorderBackgroundColor(this, &SFlowGraphNode_YapFragmentWidget::GetNodeTitleColor) // doesn't do shit
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		.Padding(2.0f)
		.BorderImage(FAppStyle::Get().GetBrush("PropertyEditor.AssetThumbnailBorder"))
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Center)
			[
				SNew(SImage)
				.Image(this, &SFlowGraphNode_YapFragmentWidget::GetPortraitBrush)
			]		
			+ SOverlay::Slot()
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Center)
			[
				SNew(STextBlock)
				.RenderTransformPivot(FVector2D(0.5, 0.5))
				.RenderTransform(FSlateRenderTransform(FQuat2D(FMath::DegreesToRadians(-30.0f))))
				.Visibility(this, &SFlowGraphNode_YapFragmentWidget::GetVisibilityForMissingPortraitText)
				.Text(LOCTEXT("FragmentCharacterMissing", "Missing"))
			]
		]
	];

	return PortraitPreview.ToSharedRef();
}

const FSlateBrush* SFlowGraphNode_YapFragmentWidget::GetPortraitBrush() const
{
	return GetFlowNodeYapDialogue()->GetSpeakerPortraitBrush(GetPortraitKey());
}

FSlateColor SFlowGraphNode_YapFragmentWidget::GetNodeTitleColor() const
{
	FLinearColor Color;

	if (GetFlowNodeYapDialogue()->GetDynamicTitleColor(Color))
	{
		return Color;
	}

	return FLinearColor::Black;
}

EVisibility SFlowGraphNode_YapFragmentWidget::GetVisibilityForMissingPortraitText() const
{
	FSlateBrush* Brush = GetFlowNodeYapDialogue()->GetSpeakerPortraitBrush(GetPortraitKey());

	if (Brush)
	{
		return (Brush->GetResourceObject()) ? EVisibility::Hidden : EVisibility::Visible;
	}
	
	return EVisibility::Visible;
}

TSharedRef<SBox> SFlowGraphNode_YapFragmentWidget::CreatePortraitKeySelector()
{
	TSharedPtr<SBox> Box;
	FMenuBuilder MenuBuilder(true, nullptr);
	FName SelectedPortraitKey = GetPortraitKey();

	for (const FName& PortraitKey : GetDefault<UFlowYapProjectSettings>()->GetPortraitKeys())
	{
		if (PortraitKey == NAME_None)
		{
			UE_LOG(FlowYap, Warning, TEXT("Warning: Portrait keys contains a 'NONE' entry. Clean this up!"));
			continue;
		}
		
		bool bSelected = PortraitKey == SelectedPortraitKey;
		MenuBuilder.AddWidget(CreatePortraitKeyMenuEntry(PortraitKey, bSelected), FText::GetEmpty());
	}

	TSharedPtr<SImage> PortraitIconImage;
	
	FString IconPath = GetDefault<UFlowYapProjectSettings>()->GetPortraitIconPath(GetPortraitKey());

	SAssignNew(Box, SBox)
	[
		SNew(SComboButton)
		.HasDownArrow(false)
		.ContentPadding(FMargin(0.f, 0.f))
		.MenuPlacement(MenuPlacement_CenteredBelowAnchor)
		.ButtonColorAndOpacity(FSlateColor(FLinearColor(0.f, 0.f, 0.f, 0.75f)))
		.HAlign(HAlign_Center)
		.ButtonStyle(FAppStyle::Get(), "SimpleButton")
		.ButtonContent()
		[
			SNew(SBox)
			.Padding(4, 4)
			[
				SAssignNew(PortraitIconImage, SImage)
				.ColorAndOpacity(FSlateColor::UseForeground())
				.Image(this, &SFlowGraphNode_YapFragmentWidget::GetPortraitKeyBrush)
			]
		]
		.MenuContent()
		[
			MenuBuilder.MakeWidget()
		]
	];
	
	return Box.ToSharedRef();
}


TSharedRef<SWidget> SFlowGraphNode_YapFragmentWidget::CreatePortraitKeyMenuEntry(FName InIconName, bool bSelected, const FText& InLabel, FName InTextStyle)
{
	const UFlowYapProjectSettings* ProjectSettings = GetDefault<UFlowYapProjectSettings>();
		
	TSharedPtr<SHorizontalBox> HBox = SNew(SHorizontalBox);

	TSharedPtr<SImage> PortraitIconImage;
		
	FString IconPath = ProjectSettings->GetPortraitIconPath(InIconName);

	// TODO this is kind of uggers, can I maybe store FSlateIcons in the subsystem instead?
	UTexture2D* PortraitKeyIcon = GEditor->GetEditorSubsystem<UFlowYapEditorSubsystem>()->GetPortraitKeyIcon(InIconName);
	
	FSlateBrush Brush;
	Brush.ImageSize = FVector2D(16, 16);
	Brush.SetResourceObject(PortraitKeyIcon);
		
	TSharedRef<FDeferredCleanupSlateBrush> PortraitKeyBrush = FDeferredCleanupSlateBrush::CreateBrush(Brush);
	
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
			.Image(TAttribute<const FSlateBrush*>::Create(TAttribute<const FSlateBrush*>::FGetter::CreateLambda([PortraitKeyBrush](){return PortraitKeyBrush->GetSlateBrush();})))
		];
	}
	
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
	.OnClicked(this, &SFlowGraphNode_YapFragmentWidget::HandlePortraitKeyChanged, InIconName)
	[
		SAssignNew(PortraitIconImage, SImage)
		.ColorAndOpacity(FSlateColor::UseForeground())
		.Image(TAttribute<const FSlateBrush*>::Create(TAttribute<const FSlateBrush*>::FGetter::CreateLambda([PortraitKeyBrush](){return PortraitKeyBrush->GetSlateBrush();})))
	];
}

const FSlateBrush* SFlowGraphNode_YapFragmentWidget::GetPortraitKeyBrush() const
{
	return GEditor->GetEditorSubsystem<UFlowYapEditorSubsystem>()->GetPortraitKeyBrush(GetPortraitKey());
}

TSharedRef<SBox> SFlowGraphNode_YapFragmentWidget::CreateTimeSettingsWidget()
{
	TSharedPtr<SBox> Box;

	FSlateIcon ProjectSettingsIcon(FAppStyle::GetAppStyleSetName(), "ProjectSettings.TabIcon");
	const FSlateBrush* ProjectSettingsIconBrush = ProjectSettingsIcon.GetIcon();
	
	TSharedRef<FDeferredCleanupSlateBrush> UseProjectDefaultsBrush = FDeferredCleanupSlateBrush::CreateBrush(*ProjectSettingsIconBrush);
	
	SAssignNew(Box, SBox)
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
				.Image(TAttribute<const FSlateBrush*>::Create(TAttribute<const FSlateBrush*>::FGetter::CreateLambda([UseProjectDefaultsBrush](){return UseProjectDefaultsBrush->GetSlateBrush();})))
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
				.Image(GEditor->GetEditorSubsystem<UFlowYapEditorSubsystem>()->GetTimerBrush())
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
					.Image(GEditor->GetEditorSubsystem<UFlowYapEditorSubsystem>()->GetTextTimeBrush())
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
				.Image(GEditor->GetEditorSubsystem<UFlowYapEditorSubsystem>()->GetAudioTimeBrush())
			]
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Fill)
		.Padding(1, 0, 1, 0)
		.MaxWidth(51)
		[
			// =============================
			// TIME DISPLAY / MANUAL ENTRY FIELD
			// =============================
			SNew(SBox)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Fill)
			[
				SNew(SNumericEntryBox<double>)
				.IsEnabled(this, &SFlowGraphNode_YapFragmentWidget::GetEnabled_TimeEntryBox)
				.Delta(0.1)
				.MinValue(0.0)
				.MinDesiredValueWidth(51) /* 8 pixels per side for border + 7 pixels per number... allow for 5 numbers */
				.ToolTipText(LOCTEXT("FragmentTimeEntry_Tooltip", "Time this dialogue fragment will play for"))
				.Justification(ETextJustify::Center)
				.Value(this, &SFlowGraphNode_YapFragmentWidget::TimeEntryBox_Value)
				.OnValueCommitted(this, &SFlowGraphNode_YapFragmentWidget::TimeEntryBox_OnValueCommitted)
			]
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(1, 0, 0, 0)
		[
			// =============================
			// INTERRUPTIBLE BUTTON
			// =============================
			SNew(SCheckBox)
			.Style( &FAppStyle::Get().GetWidgetStyle<FCheckBoxStyle>("ToggleButtonCheckBox"))
			.Padding(FMargin(4, 3))
			.CheckBoxContentUsesAutoWidth(true)
			.ToolTipText(LOCTEXT("FragmentInterruptible_Tooltip", "Can the player interrupt (fast forward) this?"))
			.IsEnabled(this, &SFlowGraphNode_YapFragmentWidget::InterruptibleButton_IsEnabled)
			.IsChecked(this, &SFlowGraphNode_YapFragmentWidget::InterruptibleButton_IsChecked)
			.OnCheckStateChanged(this, &SFlowGraphNode_YapFragmentWidget::InterruptibleButton_OnCheckStateChanged)
			[
				SNew(SBox)
				[
					SNew(SImage)
					.Image(GEditor->GetEditorSubsystem<UFlowYapEditorSubsystem>()->GetUserInterruptBrush())
				]
			]
		]
	];

	return Box.ToSharedRef(); 
}

FOptionalSize SFlowGraphNode_YapFragmentWidget::GetMaxDialogueEditableTextWidgetHeight() const
{
	int16 Deadspace = 15;
	int16 LineHeight = 15;

	int16 UnfocusedLines = 4;
	int16 FocusedLines = 9;
	
	if (!DialogueBox.Get())
	{
		return Deadspace + UnfocusedLines * LineHeight;
	}
	
	if (DialogueBox->HasKeyboardFocus())
	{
		return Deadspace + FocusedLines * LineHeight;
	}

	return Deadspace + UnfocusedLines * LineHeight;
}

// -----------------------------------------------------------------------------------------------
FText SFlowGraphNode_YapFragmentWidget::GetTitleText() const
{
	return Fragment->Bit.GetTitleText();
}

void SFlowGraphNode_YapFragmentWidget::HandleTitleTextCommitted(const FText& CommittedText, ETextCommit::Type CommitType)
{
	FFlowYapTransactions::BeginModify(LOCTEXT("NodeTitleTextChanged", "Title Text Changed"), GetFlowNodeYapDialogue());
	
	if (CommitType == ETextCommit::OnEnter || CommitType == ETextCommit::OnUserMovedFocus)
	{
		Fragment->Bit.SetTitleText(CommittedText);
	}

	FFlowYapTransactions::EndModify();
}

// -----------------------------------------------------------------------------------------------
FText SFlowGraphNode_YapFragmentWidget::GetDialogueText() const
{
	return Fragment->Bit.GetDialogueText();
}

void SFlowGraphNode_YapFragmentWidget::HandleDialogueTextCommitted(const FText& CommittedText, ETextCommit::Type CommitType)
{
	FFlowYapTransactions::BeginModify(LOCTEXT("NodeDialogueTextChanged", "Dialogue Text Changed"), GetFlowNodeYapDialogue());

	if (CommitType == ETextCommit::OnEnter || CommitType == ETextCommit::OnUserMovedFocus)
	{
		Fragment->Bit.SetDialogueText(CommittedText);
	}

	FFlowYapTransactions::EndModify();
}

 // -----------------------------------------------------------------------------------------------
FString SFlowGraphNode_YapFragmentWidget::GetSelectedDialogueAudioAssetPath() const
{
	const TSoftObjectPtr<UObject> Asset = Fragment->Bit.GetDialogueAudioAsset<UObject>();

	if (!Asset) { return ""; }

	return Asset.ToString();
}

void SFlowGraphNode_YapFragmentWidget::HandleDialogueAudioAssetChanged(const FAssetData& InAssetData)
{
	FFlowYapTransactions::BeginModify(LOCTEXT("NodeAudioAssetChanged", "Audio Asset Changed"), GetFlowNodeYapDialogue());

	Fragment->Bit.SetDialogueAudioAsset(InAssetData.GetAsset());

	FFlowYapTransactions::EndModify();
}

// -----------------------------------------------------------------------------------------------
FName SFlowGraphNode_YapFragmentWidget::GetPortraitKey() const
{
	return Fragment->Bit.GetPortraitKey();
}

FReply SFlowGraphNode_YapFragmentWidget::HandlePortraitKeyChanged(FName NewValue)
{
	FFlowYapTransactions::BeginModify(LOCTEXT("NodePortraitKeyChanged", "Portrait Key Changed"), GetFlowNodeYapDialogue());

	Fragment->Bit.SetPortraitKey(NewValue);

	FFlowYapTransactions::EndModify();

	return FReply::Handled();
}

// -----------------------------------------------------------------------------------------------


void SFlowGraphNode_YapFragmentWidget::UseAudioTimeButton_OnCheckStateChanged(ECheckBoxState CheckBoxState, EFlowYapTimeMode Mode)
{
	FFlowYapTransactions::BeginModify(LOCTEXT("NodeTimedModeChanged", "Timed Mode Changed"), GetFlowNodeYapDialogue());

	if (CheckBoxState == ECheckBoxState::Checked)
	{
		Fragment->Bit.SetBitTimeMode(Mode);
	}
	
	FFlowYapTransactions::EndModify();
}


// -----------------------------------------------------------------------------------------------
ECheckBoxState SFlowGraphNode_YapFragmentWidget::InterruptibleButton_IsChecked() const
{
	if (Fragment->Bit.GetUseProjectDefaultTimeSettings())
	{
		return GetDefault<UFlowYapProjectSettings>()->GetDefaultInterruptibleSetting() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
	}
	
	return Fragment->Bit.GetInterruptible() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SFlowGraphNode_YapFragmentWidget::InterruptibleButton_OnCheckStateChanged(ECheckBoxState CheckBoxState)
{
	FFlowYapTransactions::BeginModify(LOCTEXT("NodeInterruptibleChanged", "Interruptible Changed"), GetFlowNodeYapDialogue());
	
	Fragment->Bit.SetBitInterruptible(CheckBoxState == ECheckBoxState::Checked ? true : false);

	FFlowYapTransactions::EndModify();
}

ECheckBoxState SFlowGraphNode_YapFragmentWidget::UseProjectDefaultTimeSettingsButton_IsChecked() const
{
	return Fragment->Bit.GetUseProjectDefaultTimeSettings() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SFlowGraphNode_YapFragmentWidget::UseProjectDefaultTimeSettingsButton_OnCheckStateChanged(ECheckBoxState CheckBoxState)
{
	FFlowYapTransactions::BeginModify(LOCTEXT("NodeUseProjectDefaultTimeSettings", "Use Project Default Time Settings Changed"), GetFlowNodeYapDialogue());

	Fragment->SetUseProjectDefaultTimeSettings(CheckBoxState == ECheckBoxState::Checked ? true : false);
	
	FFlowYapTransactions::EndModify();
}

// -----------------------------------------------------------------------------------------------
bool SFlowGraphNode_YapFragmentWidget::GetEnabled_TimeEntryBox() const
{
	if (Fragment->GetUseProjectDefaultTimeSettings())
	{
		return false;
	}
	
	if (Fragment->GetTimeMode() != EFlowYapTimeMode::ManualTime)
	{
		return false;
	}
	
	return true;
}

bool SFlowGraphNode_YapFragmentWidget::InterruptibleButton_IsEnabled() const
{
	if (Fragment->GetUseProjectDefaultTimeSettings())
	{
		return false;
	}

	if (Fragment->GetTimeMode() == EFlowYapTimeMode::None)
	{
		return false;
	}

	return true;
}

bool SFlowGraphNode_YapFragmentWidget::UseAudioTimeButton_IsEnabled() const
{
	if (Fragment->GetUseProjectDefaultTimeSettings())
	{
		return false;
	}

	return true;
}

ECheckBoxState SFlowGraphNode_YapFragmentWidget::UseAudioTimeButton_IsChecked() const
{
	if (Fragment->GetUseProjectDefaultTimeSettings())
	{
		return Fragment->GetTimeMode() == EFlowYapTimeMode::AudioLength ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
	}
	
	const FFlowYapFragmentTimeSettings& TimeSettingsRef = Fragment->GetTimeSettings();

	return TimeSettingsRef.TimedMode == EFlowYapTimeMode::AudioLength ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

// -----------------------------------------------------------------------------------------------
bool SFlowGraphNode_YapFragmentWidget::IsManualTimeEntryEnabled() const
{
	EFlowYapTimeMode TimeMode = Fragment->Bit.GetTimeMode();
	
	return TimeMode == EFlowYapTimeMode::ManualTime;
}

TOptional<double> SFlowGraphNode_YapFragmentWidget::TimeEntryBox_Value() const
{
	return Fragment->Bit.GetTime();
}

void SFlowGraphNode_YapFragmentWidget::TimeEntryBox_OnValueCommitted(double NewValue, ETextCommit::Type CommitType)
{
	FFlowYapTransactions::BeginModify(LOCTEXT("NodeEnteredTimeChanged", "Entered Time Changed"), GetFlowNodeYapDialogue());

	if (CommitType == ETextCommit::OnEnter || CommitType == ETextCommit::OnUserMovedFocus)
	{
		Fragment->Bit.SetManualTime(NewValue);
	}

	if (CommitType == ETextCommit::OnCleared)
	{
		Fragment->SetEnteredTimeValue(0.0);
	}

	FFlowYapTransactions::EndModify();
}

bool SFlowGraphNode_YapFragmentWidget::GetEnabled_UseTextTimeButton() const
{
	if (Fragment->GetUseProjectDefaultTimeSettings())
	{
		return false;
	}

	return true;
}

EVisibility SFlowGraphNode_YapFragmentWidget::GetUseTimeFromAudioButtonErrorState() const
{
	const FFlowYapFragmentTimeSettings& TimeSettingsRef = Fragment->GetTimeSettings();

	if (TimeSettingsRef.TimedMode == EFlowYapTimeMode::AudioLength && !Fragment->HasDialogueAudioAsset())
	{
		return EVisibility::HitTestInvisible;
	}
	else
	{
		return EVisibility::Hidden;
	}
}

EVisibility SFlowGraphNode_YapFragmentWidget::GetSelectedDialogueAudioAssetIsValid() const
{
	UClass* AssetClass = GetDefault<UFlowYapProjectSettings>()->GetDialogueAssetClass();

	const UObject* Asset = Fragment->GetDialogueAudioAsset();

	if (Asset)
	{
		return (Asset->IsA(AssetClass)) ? EVisibility::Collapsed : EVisibility::HitTestInvisible;
	}
	else
	{
		return EVisibility::Collapsed;
	}
}

EVisibility SFlowGraphNode_YapFragmentWidget::DisplayAllLowerFragmentControls() const
{
	if (!Owner->GetIsSelected())
	{
		return EVisibility::Collapsed;
	}
	
	if (Owner->GetFocusedFragment() == this)
	{
		return EVisibility::Visible;
	}

	if (Owner->GetControlHooked())
	{
		return EVisibility::Visible;
	}

	return EVisibility::Collapsed;
}

// -----------------------------------------------------------------------------------------------
UFlowNode_YapDialogue* SFlowGraphNode_YapFragmentWidget::GetFlowNodeYapDialogue() const
{
	return Owner->GetFlowYapDialogueNode();
}

void SFlowGraphNode_YapFragmentWidget::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	const FGeometry& OwnerGeo = Owner->GetTickSpaceGeometry();
	FVector2D OwnerSize = OwnerGeo.GetAbsoluteSize();
	
	FVector2D UL = OwnerGeo.GetAbsolutePosition() - FVector2D(50, 50);
	FVector2D LR = UL + OwnerSize + FVector2D(50, 500);
	
	bShiftPressed = GEditor->GetEditorSubsystem<UFlowYapEditorSubsystem>()->GetInputTracker()->GetShiftPressed();

	if (DialogueBox->HasKeyboardFocus() || TitleTextBox->HasKeyboardFocus())
	{
		Owner->SetFocusedFragment(this);
	}
	else
	{
		Owner->ClearFocusedFragment(this);
	}
}
#undef LOCTEXT_NAMESPACE
