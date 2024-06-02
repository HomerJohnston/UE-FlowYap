// 2022 The Enby Witch

#include "Widgets/SFlowGraphNode_YapFragmentWidget.h"

#include "EditorStyleSet.h"
#include "FlowYapColors.h"
#include "GraphNodes/FlowGraphNode_YapDialogue.h"
#include "PropertyCustomizationHelpers.h"
#include "FlowYap/FlowYapProjectSettings.h"
#include "FlowYap/Nodes/FlowNode_YapDialogue.h"
#include "Slate/DeferredCleanupSlateBrush.h"
#include "FlowYapEditorSubsystem.h"
#include "FlowYapTransactions.h"
#include "FlowYap/FlowYapLog.h"
#include "Widgets/SFlowGraphNode_YapDialogueWidget.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "FlowYap/FlowYapFlowModes.h"
#include "Widgets/Layout/SSeparator.h"

#define LOCTEXT_NAMESPACE "PGFlowEditor"

void SFlowGraphNode_YapFragmentWidget::Construct(const FArguments& InArgs, SFlowGraphNode_YapDialogueWidget* InOwner, FFlowYapFragment* InFragment)
{
	Owner = InOwner;
	FragmentID = InFragment->GetEditorID();

	DialogueAssetClass = GetDefault<UFlowYapProjectSettings>()->GetDialogueAssetClass();
	
	ChildSlot
	[
		CreateDialogueContentArea()
	];
}

EVisibility SFlowGraphNode_YapFragmentWidget::GetPortraitWidgetVisibility() const
{
	if (DialogueBox->HasKeyboardFocus())
	{
		return EVisibility::Collapsed;
	}

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

FReply SFlowGraphNode_YapFragmentWidget::OnClickPortrait()
{
	GetFlowNodeYapDialogue()->ToggleIsPlayerPrompt();

	return FReply::Handled();
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
			.MinDesiredHeight(84)
			[
				SNew(SHorizontalBox)
				// ===================
				// PORTRAIT IMAGE (LEFT SIDE)
				// ===================
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Top)
				.AutoWidth()
				.Padding(2.f, 5.f, 5.f, 5.f)
				[
					SNew(SOverlay)
					.Visibility(this, &SFlowGraphNode_YapFragmentWidget::GetPortraitWidgetVisibility)
					+ SOverlay::Slot()
					[
						SNew(SButton)
						.ButtonStyle(FAppStyle::Get(), "SimpleButton")
						.ContentPadding(0)
						.OnClicked(this, &SFlowGraphNode_YapFragmentWidget::OnClickPortrait)
						[
							CreatePortraitWidget()
						]
					]
					+ SOverlay::Slot()
					.HAlign(HAlign_Right)
					.VAlign(VAlign_Bottom)
					.Padding(FMargin(0, 0, 2, 2))
					[
						CreatePortraitKeySelector()
					]
				]
				// ===================
				// DIALOGUE (RIGHT SIDE)
				// ===================
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				.FillWidth(1.0f)
				.Padding(0.f, 5.0f, 0.0f, 5.0f)
				[
					SNew(SBox)
					.MaxDesiredHeight(this, &SFlowGraphNode_YapFragmentWidget::GetMaxDialogueEditableTextWidgetHeight)
					[
						SAssignNew(DialogueBox, SMultiLineEditableTextBox)
						//.AutoWrapText(true)
						.Text(this, &SFlowGraphNode_YapFragmentWidget::GetDialogueText)
						.ModiferKeyForNewLine(EModifierKey::Shift)
						.HScrollBarPadding(FMargin(0))
						.OnTextCommitted(this, &SFlowGraphNode_YapFragmentWidget::HandleDialogueTextCommitted)
						.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
						.HintText(INVTEXT("Enter full dialogue"))
						.BackgroundColor(this, &SFlowGraphNode_YapFragmentWidget::GetDialogueTextBackgroundColor)
						.ForegroundColor(this, &SFlowGraphNode_YapFragmentWidget::GetDialogueTextColor)
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
			.HeightOverride(52)
			[
				SNew(SVerticalBox)
				// ===================
				// TITLE TEXT
				// ===================
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Fill)
				.Padding(0, 0, 0, 2)
				[
					SNew(SEditableTextBox)
					.Visibility(this, &SFlowGraphNode_YapFragmentWidget::GetTitleTextEntryVisibility)
					.Text(this, &SFlowGraphNode_YapFragmentWidget::GetTitleText)
					.OnTextCommitted(this, &SFlowGraphNode_YapFragmentWidget::HandleTitleTextCommitted)
					.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
					//.MinDesiredWidth(270)
					.HintText(INVTEXT("Enter optional title text"))
					.ToolTipText(LOCTEXT("Test", "Test Test TODO"))
					//.ToolTip(INVTEXT("Title text is only used for selectable dialogue.\nWhen supplied, the game's UI will display selectable repsonses using their title text representation.\nWhen not supplied, the game's UI will display the full dialogue below for selectable responses."))
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
							SNew(SObjectPropertyEntryBox)
							.DisplayBrowse(true)
							.DisplayUseSelected(false)
							.DisplayThumbnail(false)
							.AllowedClass(DialogueAssetClass)
							.EnableContentPicker(true)
							.ObjectPath(this, &SFlowGraphNode_YapFragmentWidget::GetSelectedDialogueAudioAssetPath)
							.OnObjectChanged(this, &SFlowGraphNode_YapFragmentWidget::HandleDialogueAudioAssetChanged)
							.ToolTipText(LOCTEXT("Test", "Test Test TODO"))
						]
						+ SOverlay::Slot()
						.Padding(36,0,0,0)
						.HAlign(HAlign_Left)
						.VAlign(VAlign_Center)
						[
							SNew(SImage)
							.Visibility(this, &SFlowGraphNode_YapFragmentWidget::GetUseTimeFromAudioButtonErrorState)
							.Image(FAppStyle::GetBrush("Icons.Warning"))
							.ColorAndOpacity(FlowYapColors::Orange)
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
	return GetFlowNodeYapDialogue()->GetIsPlayerPrompt() ? FLinearColor(0.8, 0.82, 1.0, 1.0) : FLinearColor(0.60, 0.60, 0.60, 1.0);
}

FSlateColor SFlowGraphNode_YapFragmentWidget::GetDialogueTextBackgroundColor() const
{
	return GetFlowNodeYapDialogue()->GetIsPlayerPrompt() ? FLinearColor::White : FLinearColor::Black;
}

TSharedRef<SBox> SFlowGraphNode_YapFragmentWidget::CreatePortraitWidget()
{
	TSharedPtr<SBox> PortraitPreview;
	
	SAssignNew(PortraitPreview, SBox)
	.WidthOverride(74)
	.HeightOverride(74)
	[
		SNew(SBorder)
		//.BorderBackgroundColor(this, &SFlowGraphNode_YapFragmentWidget::GetNodeTitleColor)
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
				.Text(INVTEXT("MISSING"))
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
	
	FMenuBuilder MenuBuilder(true, NULL);
	
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
		.Padding(0.f)
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

	ToggleButtonCheckBox_Orange = FAppStyle::Get().GetWidgetStyle<FCheckBoxStyle>("ToggleButtonCheckBox");
	ToggleButtonCheckBox_Orange.CheckedImage.TintColor = FLinearColor(FlowYapColors::Orange);
	ToggleButtonCheckBox_Orange.CheckedHoveredImage.TintColor = FLinearColor(FlowYapColors::OrangeHovered);
	ToggleButtonCheckBox_Orange.CheckedPressedImage.TintColor = FLinearColor(FlowYapColors::OrangePressed);

	UseAudioTimeButtonStyle = FAppStyle::Get().GetWidgetStyle<FCheckBoxStyle>("ToggleButtonCheckBox");

	if (!IsUseTimeFromAudioEnabled())
	{
		UseAudioTimeButtonStyle.CheckedImage.TintColor = FLinearColor(FlowYapColors::Red);
		UseAudioTimeButtonStyle.CheckedHoveredImage.TintColor = FLinearColor(FlowYapColors::RedHovered);
		UseAudioTimeButtonStyle.CheckedPressedImage.TintColor = FLinearColor(FlowYapColors::RedPressed);
	}
	
	SAssignNew(Box, SBox)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		.Padding(0, 0, 1, 0)
		[
			SNew(SCheckBox)
			.Style(&ToggleButtonCheckBox_Orange)
			.Padding(FMargin(4, 3))
			.CheckBoxContentUsesAutoWidth(true)
			.ToolTipText(INVTEXT("Test Test TODO"))
			.IsChecked(this, &SFlowGraphNode_YapFragmentWidget::GetUseProjectDefaultTimeSettingsChecked)
			.OnCheckStateChanged(this, &SFlowGraphNode_YapFragmentWidget::HandleUseProjectDefaultTimeSettingsChanged)
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
			SNew(SCheckBox)
			.Style(&FAppStyle::Get().GetWidgetStyle<FCheckBoxStyle>("ToggleButtonCheckBox"))
			.Padding(FMargin(4, 3))
			.CheckBoxContentUsesAutoWidth(true)
			.IsEnabled(this, &SFlowGraphNode_YapFragmentWidget::IsUseEnteredTimeEnabled)
			.IsChecked(this, &SFlowGraphNode_YapFragmentWidget::GetIsTimedMode, EFlowYapTimedMode::UseEnteredTime)
			.OnCheckStateChanged(this, &SFlowGraphNode_YapFragmentWidget::HandleTimedModeChanged, EFlowYapTimedMode::UseEnteredTime)
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
			SNew(SCheckBox)
			.Style(&FAppStyle::Get().GetWidgetStyle<FCheckBoxStyle>("ToggleButtonCheckBox"))
			.Padding(FMargin(4, 3))
			.CheckBoxContentUsesAutoWidth(true)
			.IsEnabled(this, &SFlowGraphNode_YapFragmentWidget::IsUseTimeFromTextEnabled)
			.IsChecked(this, &SFlowGraphNode_YapFragmentWidget::GetIsTimedMode, EFlowYapTimedMode::AutomaticFromText)
			.OnCheckStateChanged(this, &SFlowGraphNode_YapFragmentWidget::HandleTimedModeChanged, EFlowYapTimedMode::AutomaticFromText)
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
			SNew(SOverlay)
			+ SOverlay::Slot()
			[
				SNew(SCheckBox)
				//.Style(&UseAudioTimeButtonStyle)
				.Style(&FAppStyle::Get().GetWidgetStyle<FCheckBoxStyle>("ToggleButtonCheckBox"))
				.Padding(FMargin(4, 3))
				.CheckBoxContentUsesAutoWidth(true)
				.IsEnabled(this, &SFlowGraphNode_YapFragmentWidget::IsUseTimeFromAudioEnabled)
				.IsChecked(this, &SFlowGraphNode_YapFragmentWidget::GetUseTimeFromAudioChecked)
				.OnCheckStateChanged(this, &SFlowGraphNode_YapFragmentWidget::HandleTimedModeChanged, EFlowYapTimedMode::AutomaticFromAudio)
				.HAlign(HAlign_Center)
				[
					SNew(SImage)
					.Image(GEditor->GetEditorSubsystem<UFlowYapEditorSubsystem>()->GetAudioTimeBrush())
				]
			]
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Fill)
		.Padding(1, 0, 1, 0)
		.MaxWidth(51)
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Fill)
			[
				SNew(SBox)
				[
					SNew(SNumericEntryBox<double>)
					.IsEnabled(this, &SFlowGraphNode_YapFragmentWidget::GetTimeEntryEnabled)
					.Delta(0.1)
					.MinValue(0.0)
					/* 8 pixels per side for border + 7 pixels per number... allow for 5 numbers */
					.MinDesiredValueWidth(51)
					.Value(this, &SFlowGraphNode_YapFragmentWidget::GetEnteredTime)
					.Justification(ETextJustify::Center)
					.OnValueCommitted(this, &SFlowGraphNode_YapFragmentWidget::HandleEnteredTimeChanged)
				]
			]
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(1, 0, 2, 0)
		[
			SNew(SCheckBox)
			.Style( &FAppStyle::Get().GetWidgetStyle<FCheckBoxStyle>("ToggleButtonCheckBox"))
			.Padding(FMargin(4, 3))
			.CheckBoxContentUsesAutoWidth(true)
			.IsEnabled(this, &SFlowGraphNode_YapFragmentWidget::GetUserInterruptibleButtonEnabled)
			.IsChecked(this, &SFlowGraphNode_YapFragmentWidget::GetUserInterruptibleChecked)
			.OnCheckStateChanged(this, &SFlowGraphNode_YapFragmentWidget::HandleUserInterruptibleChanged)
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

/*

FOptionalSize SFlowGraphNode_YapFragmentWidget::GetMinDialogueEditableTextWidgetWidth() const
{
	return 300;
}
*/

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

/*
FOptionalSize SFlowGraphNode_YapFragmentWidget::GetMaxDialogueEditableTextWidgetWidth() const
{
	if (!DialogueBox.Get())
	{
		return 300;
	}
	
	if (DialogueBox->HasKeyboardFocus())
	{
		// TODO It feels weird to have the graph morphing around. Don't permit any stretching. Can I draw a whole new multiline widget over top of everything somehow eventually?
		return 300 + 74 + 5;
	}

	return 300;
}

FOptionalSize SFlowGraphNode_YapFragmentWidget::GetMinDialogueEditableTextWidgetHeight() const
{
}
*/

// -----------------------------------------------------------------------------------------------
FText SFlowGraphNode_YapFragmentWidget::GetTitleText() const
{
	return GetFragment().GetTitleText();
}

void SFlowGraphNode_YapFragmentWidget::HandleTitleTextCommitted(const FText& CommittedText, ETextCommit::Type CommitType)
{
	FFlowYapTransactions::BeginModify(INVTEXT("Title Text Changed"), GetFlowNodeYapDialogue());
	
	if (CommitType == ETextCommit::OnEnter || CommitType == ETextCommit::OnUserMovedFocus)
	{
		GetFragment().SetTitleText(CommittedText);
	}

	FFlowYapTransactions::EndModify();
}

// -----------------------------------------------------------------------------------------------
FText SFlowGraphNode_YapFragmentWidget::GetDialogueText() const
{
	return GetFragment().GetDialogueText();
}

void SFlowGraphNode_YapFragmentWidget::HandleDialogueTextCommitted(const FText& CommittedText, ETextCommit::Type CommitType)
{
	FFlowYapTransactions::BeginModify(INVTEXT("Dialogue Text Changed"), GetFlowNodeYapDialogue());

	if (CommitType == ETextCommit::OnEnter || CommitType == ETextCommit::OnUserMovedFocus)
	{
		GetFragment().SetDialogueText(CommittedText);
	}

	FFlowYapTransactions::EndModify();
}

 // -----------------------------------------------------------------------------------------------
FString SFlowGraphNode_YapFragmentWidget::GetSelectedDialogueAudioAssetPath() const
{
	const UObject* Asset = GetFragment().GetDialogueAsset();

	if (!Asset) { return ""; }

	return FSoftObjectPath(Asset).GetAssetPathString();
}

void SFlowGraphNode_YapFragmentWidget::HandleDialogueAudioAssetChanged(const FAssetData& InAssetData)
{
	FFlowYapTransactions::BeginModify(INVTEXT("Audio Asset Changed"), GetFlowNodeYapDialogue());

	GetFragment().SetDialogueAudio(InAssetData.GetAsset());

	FFlowYapTransactions::EndModify();
}

// -----------------------------------------------------------------------------------------------
FName SFlowGraphNode_YapFragmentWidget::GetPortraitKey() const
{
	return GetFragment().GetPortraitKey();
}

FReply SFlowGraphNode_YapFragmentWidget::HandlePortraitKeyChanged(FName NewValue)
{
	FFlowYapTransactions::BeginModify(INVTEXT("Portrait Key Changed"), GetFlowNodeYapDialogue());

	GetFragment().SetPortraitKey(NewValue);

	FFlowYapTransactions::EndModify();

	return FReply::Handled();
}

bool SFlowGraphNode_YapFragmentWidget::GetIsNotTimedMode(EFlowYapTimedMode TimedMode) const
{
	return GetFragment().GetTimedMode() != TimedMode;
}

// -----------------------------------------------------------------------------------------------
ECheckBoxState SFlowGraphNode_YapFragmentWidget::GetIsTimedMode(EFlowYapTimedMode QueriedMode) const
{
	EFlowYapTimedMode TimedMode = GetFragment().GetUsesProjectDefaultTimeSettings() ? GetFragment().GetRuntimeTimedMode() : GetFragment().GetTimedMode();
		
	return TimedMode == QueriedMode ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SFlowGraphNode_YapFragmentWidget::HandleTimedModeChanged(ECheckBoxState CheckBoxState, EFlowYapTimedMode Mode)
{
	FFlowYapTransactions::BeginModify(INVTEXT("Timed Mode Changed"), GetFlowNodeYapDialogue());

	if (CheckBoxState == ECheckBoxState::Checked)
	{
		GetFragment().SetTimedMode(Mode);
	}
	else
	{
		GetFragment().UnsetTimedMode();
	}
	
	FFlowYapTransactions::EndModify();
}


// -----------------------------------------------------------------------------------------------
ECheckBoxState SFlowGraphNode_YapFragmentWidget::GetUserInterruptibleChecked() const
{
	if (GetFragment().GetUsesProjectDefaultTimeSettings())
	{
		const FFlowYapFragmentTimeSettings& TimeSettings = GetDefault<UFlowYapProjectSettings>()->GetDefaultTimeSettings();

		return TimeSettings.bUserInterruptible ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
	}

	if (!GetUserInterruptibleButtonEnabled())
	{
		return ECheckBoxState::Unchecked;
	}
	
	return GetFragment().GetTimeSettings().bUserInterruptible ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SFlowGraphNode_YapFragmentWidget::HandleUserInterruptibleChanged(ECheckBoxState CheckBoxState)
{
	FFlowYapTransactions::BeginModify(INVTEXT("Interruptible Changed"), GetFlowNodeYapDialogue());
	
	GetFragment().SetUserInterruptible(CheckBoxState == ECheckBoxState::Checked ? true : false);

	FFlowYapTransactions::EndModify();
}

ECheckBoxState SFlowGraphNode_YapFragmentWidget::GetUseProjectDefaultTimeSettingsChecked() const
{
	bool bUsesProjectDefaultTimeSettings = GetFragment().GetUsesProjectDefaultTimeSettings();
	
	return bUsesProjectDefaultTimeSettings ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SFlowGraphNode_YapFragmentWidget::HandleUseProjectDefaultTimeSettingsChanged(ECheckBoxState CheckBoxState)
{
	FFlowYapTransactions::BeginModify(INVTEXT("Use Project Default Time Settings Changed"), GetFlowNodeYapDialogue());

	GetFragment().SetUseProjectDefaultTimeSettings(CheckBoxState == ECheckBoxState::Checked ? true : false);
	
	FFlowYapTransactions::EndModify();
}

// -----------------------------------------------------------------------------------------------
bool SFlowGraphNode_YapFragmentWidget::GetTimeEntryEnabled() const
{
	if (GetFragment().GetUsesProjectDefaultTimeSettings())
	{
		return false;
	}
	
	if (GetFragment().GetTimedMode() != EFlowYapTimedMode::UseEnteredTime)
	{
		return false;
	}
	
	return true;
}

bool SFlowGraphNode_YapFragmentWidget::GetUserInterruptibleButtonEnabled() const
{
	if (GetFragment().GetUsesProjectDefaultTimeSettings())
	{
		return false;
	}

	if (GetFragment().GetTimedMode() == EFlowYapTimedMode::None)
	{
		return false;
	}

	return true;
}

bool SFlowGraphNode_YapFragmentWidget::IsUseTimeFromAudioEnabled() const
{
	if (GetFragment().GetUsesProjectDefaultTimeSettings())
	{
		return false;
	}

	return true;
}

ECheckBoxState SFlowGraphNode_YapFragmentWidget::GetUseTimeFromAudioChecked() const
{
	if (GetFragment().GetUsesProjectDefaultTimeSettings())
	{
		return GetFragment().GetRuntimeTimedMode() == EFlowYapTimedMode::AutomaticFromAudio ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
	}
	
	const FFlowYapFragmentTimeSettings& TimeSettingsRef = GetFragment().GetRuntimeTimeSettings();

	return TimeSettingsRef.TimedMode == EFlowYapTimedMode::AutomaticFromAudio ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

// -----------------------------------------------------------------------------------------------
bool SFlowGraphNode_YapFragmentWidget::IsUseEnteredTimeEnabled() const
{
	if (GetFragment().GetUsesProjectDefaultTimeSettings())
	{
		return false;
	}

	return true;
}

TOptional<double> SFlowGraphNode_YapFragmentWidget::GetEnteredTime() const
{
	TOptional<double> Value;

	const FFlowYapFragmentTimeSettings& TimeSettingsRef = GetFragment().GetRuntimeTimeSettings();

	if (TimeSettingsRef.TimedMode == EFlowYapTimedMode::None)
	{
		Value.Reset();
	}
	else
	{
		Value = GetFragment().GetCalculatedTimedValue();
	}

	return Value;
}

void SFlowGraphNode_YapFragmentWidget::HandleEnteredTimeChanged(double NewValue, ETextCommit::Type CommitType)
{
	FFlowYapTransactions::BeginModify(INVTEXT("Time Value Changed"), GetFlowNodeYapDialogue());

	FFlowYapFragment& Fragment = GetFragment();
	
	if (CommitType == ETextCommit::OnEnter || CommitType == ETextCommit::OnUserMovedFocus)
	{
		Fragment.SetEnteredTimeValue(NewValue);
	}

	if (CommitType == ETextCommit::OnCleared)
	{
		Fragment.SetEnteredTimeValue(0.0);
	}

	FFlowYapTransactions::EndModify();
}

bool SFlowGraphNode_YapFragmentWidget::IsUseTimeFromTextEnabled() const
{
	if (GetFragment().GetUsesProjectDefaultTimeSettings())
	{
		return false;
	}

	return true;
}

EVisibility SFlowGraphNode_YapFragmentWidget::GetUseTimeFromAudioButtonErrorState() const
{
	const FFlowYapFragmentTimeSettings& TimeSettingsRef = GetFragment().GetRuntimeTimeSettings();

	if (TimeSettingsRef.TimedMode == EFlowYapTimedMode::AutomaticFromAudio && !GetFragment().HasDialogueAudioAsset())
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

	const UObject* Asset = GetFragment().GetDialogueAsset();

	if (Asset)
	{
		return (Asset->IsA(AssetClass)) ? EVisibility::Collapsed : EVisibility::HitTestInvisible;
	}
	else
	{
		return EVisibility::Collapsed;
	}
}

// -----------------------------------------------------------------------------------------------
UFlowNode_YapDialogue* SFlowGraphNode_YapFragmentWidget::GetFlowNodeYapDialogue() const
{
	return Owner->GetFlowYapDialogueNode();
}

FFlowYapFragment& SFlowGraphNode_YapFragmentWidget::GetFragment() const
{
	return GetFlowNodeYapDialogue()->GetFragment(FragmentID);
}
#undef LOCTEXT_NAMESPACE
