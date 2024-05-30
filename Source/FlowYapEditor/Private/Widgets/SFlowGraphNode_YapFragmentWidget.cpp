// 2022 The Enby Witch

#include "Widgets/SFlowGraphNode_YapFragmentWidget.h"

#include "AkAudioEvent.h"
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

#define LOCTEXT_NAMESPACE "PGFlowEditor"

void SFlowGraphNode_YapFragmentWidget::Construct(const FArguments& InArgs, SFlowGraphNode_YapDialogueWidget* InOwner, FFlowYapFragment* InFragment)
{
	Owner = InOwner;
	FragmentID = InFragment->GetEditorID();

	ChildSlot
	[
		CreateDialogueContentArea()
	];
}

TSharedRef<SWidget> SFlowGraphNode_YapFragmentWidget::CreateDialogueContentArea()
{
	return SNew(SVerticalBox)
	+ SVerticalBox::Slot()
	.AutoHeight()
	[
		SNew(SHorizontalBox)
		// ===================
		// PORTRAIT IMAGE (LEFT SIDE)
		// ===================
		+ SHorizontalBox::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Top)
		.AutoWidth()
		.Padding(2.f, 5.f, 5.f, 2.f)
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			[
				CreatePortraitWidget()
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
		// DIALOGUE SETTINGS (RIGHT SIDE)
		// ===================
		+ SHorizontalBox::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Center)
		.FillWidth(1.0f)
		.Padding(5.f)
		[
			SNew(SVerticalBox)
			// ===================
			// TITLE TEXT
			// ===================
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.HAlign(HAlign_Fill)
				[
					SNew(SEditableTextBox)
					.Text(this, &SFlowGraphNode_YapFragmentWidget::GetTitleText)
					.OnTextCommitted(this, &SFlowGraphNode_YapFragmentWidget::HandleTitleTextCommitted)
					.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
					.MinDesiredWidth(270)
					.HintText(INVTEXT("Enter optional title text"))
					.ToolTipText(LOCTEXT("Test", "Test Test TODO"))
					//.ToolTip(INVTEXT("Title text is only used for selectable dialogue.\nWhen supplied, the game's UI will display selectable repsonses using their title text representation.\nWhen not supplied, the game's UI will display the full dialogue below for selectable responses."))
				]
			]
			// ===================
			// SPACER
			// ===================
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SSpacer)
				.Size(4.f)
			]
			// ===================
			// AUDIO ASSET SELECTOR
			// ===================
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SObjectPropertyEntryBox)
				.DisplayBrowse(true)
				.DisplayThumbnail(true)
				.AllowedClass(UAkAudioEvent::StaticClass())
				.EnableContentPicker(true)
				.ObjectPath(this, &SFlowGraphNode_YapFragmentWidget::GetSelectedDialogueAudioAssetPath)
				.OnObjectChanged(this, &SFlowGraphNode_YapFragmentWidget::HandleDialogueAudioAssetChanged)
				.ToolTipText(LOCTEXT("Test", "Test Test TODO"))
			]
			// ===================
			// SPACER
			// ===================
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SSpacer)
				.Size(4.f)
			]
			// ===================
			// ADDITIONAL OPTIONS
			// ===================
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				CreateFlowOptionsWidget()
			]
		]
	]
	+ SVerticalBox::Slot()
	.AutoHeight()
	[
		SNew(SBox)
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Top)
		.MaxDesiredHeight(this, &SFlowGraphNode_YapFragmentWidget::GetMaxDialogueEditableTextWidgetHeight)
		.MaxDesiredWidth(this, &SFlowGraphNode_YapFragmentWidget::GetMaxDialogueEditableTextWidgetWidth)
		.Padding(4.f)
		[
			SAssignNew(DialogueBox, SMultiLineEditableTextBox)
			.Text(this, &SFlowGraphNode_YapFragmentWidget::GetDialogueText)
			.OnTextCommitted(this, &SFlowGraphNode_YapFragmentWidget::HandleDialogueTextCommitted)
			.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
			.HintText(INVTEXT("Enter full dialogue"))
		]
	];
}

TSharedRef<SBox> SFlowGraphNode_YapFragmentWidget::CreatePortraitWidget()
{
	TSharedPtr<SBox> PortraitPreview;
	
	SAssignNew(PortraitPreview, SBox)
	.WidthOverride(100)
	.HeightOverride(100)
	[
		SNew(SBorder)
		.ForegroundColor(FLinearColor::White)
		//.BorderBackgroundColor(this, &SGraphNode::GetNodeTitleColor)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		.Padding(2)
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			[
				SNew(SImage)
				.Image(this, &SFlowGraphNode_YapFragmentWidget::GetPortraitBrush)
			]
			+ SOverlay::Slot()
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

TSharedRef<SBox> SFlowGraphNode_YapFragmentWidget::CreateFlowOptionsWidget()
{
	TSharedPtr<SBox> Box;

	FSlateIcon ProjectSettingsIcon(FAppStyle::GetAppStyleSetName(), "ProjectSettings.TabIcon");
	const FSlateBrush* ProjectSettingsIconBrush = ProjectSettingsIcon.GetIcon();
	
	TSharedRef<FDeferredCleanupSlateBrush> UseProjectDefaultsBrush = FDeferredCleanupSlateBrush::CreateBrush(*ProjectSettingsIconBrush);

	UseProjectDefaultsButtonStyle = FAppStyle::Get().GetWidgetStyle<FCheckBoxStyle>("ToggleButtonCheckBox");
	UseProjectDefaultsButtonStyle.CheckedImage.TintColor = FLinearColor(FlowYapColors::Orange);
	UseProjectDefaultsButtonStyle.CheckedHoveredImage.TintColor = FLinearColor(FlowYapColors::OrangeHovered);
	UseProjectDefaultsButtonStyle.CheckedPressedImage.TintColor = FLinearColor(FlowYapColors::OrangePressed);

	SAssignNew(Box, SBox)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.FillWidth(1.0)
		[
			SNew(SSpacer)
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(2, 0)
		[
			SNew(SCheckBox)
			.Style(&UseProjectDefaultsButtonStyle)
			.Padding(FMargin(4, 3))
			.CheckBoxContentUsesAutoWidth(true)
			.ToolTipText(INVTEXT("Test Test TODO"))
			.IsChecked(this, &SFlowGraphNode_YapFragmentWidget::GetIsTimedMode, EFlowYapTimedMode::UseProjectDefaults)
			.OnCheckStateChanged(this, &SFlowGraphNode_YapFragmentWidget::HandleTimedModeChanged, EFlowYapTimedMode::UseProjectDefaults)
			.Content()
			[
				SNew(SImage)
				.ColorAndOpacity(FSlateColor::UseForeground())
				.Image(TAttribute<const FSlateBrush*>::Create(TAttribute<const FSlateBrush*>::FGetter::CreateLambda([UseProjectDefaultsBrush](){return UseProjectDefaultsBrush->GetSlateBrush();})))
			]
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(2, 0)
		[
			SNew(SCheckBox)
			.Style(&FAppStyle::Get().GetWidgetStyle<FCheckBoxStyle>("ToggleButtonCheckBox"))
			.Padding(FMargin(4, 3))
			.CheckBoxContentUsesAutoWidth(true)
			.IsEnabled(this, &SFlowGraphNode_YapFragmentWidget::GetIsNotTimedMode, EFlowYapTimedMode::UseProjectDefaults)
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
		.Padding(2, 0)
		[
			SNew(SCheckBox)
			.Style(&FAppStyle::Get().GetWidgetStyle<FCheckBoxStyle>("ToggleButtonCheckBox"))
			.Padding(FMargin(4, 3))
			.CheckBoxContentUsesAutoWidth(true)
			.IsEnabled(this, &SFlowGraphNode_YapFragmentWidget::GetIsNotTimedMode, EFlowYapTimedMode::UseProjectDefaults)
			.IsChecked(this, &SFlowGraphNode_YapFragmentWidget::GetIsTimedMode, EFlowYapTimedMode::AutomaticFromText)
			.OnCheckStateChanged(this, &SFlowGraphNode_YapFragmentWidget::HandleTimedModeChanged, EFlowYapTimedMode::AutomaticFromText)
			[
				SNew(SBox)
				[
					SNew(SImage)
					.ColorAndOpacity(FSlateColor::UseForeground())
					.Image(GEditor->GetEditorSubsystem<UFlowYapEditorSubsystem>()->GetTextTimeBrush())
				]
			]
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(2, 0)
		[
			SNew(SCheckBox)
			.Style(&FAppStyle::Get().GetWidgetStyle<FCheckBoxStyle>("ToggleButtonCheckBox"))
			.Padding(FMargin(4, 3))
			.CheckBoxContentUsesAutoWidth(true)
			.IsEnabled(this, &SFlowGraphNode_YapFragmentWidget::GetAutoFromAudioButtonEnabled)
			.IsChecked(this, &SFlowGraphNode_YapFragmentWidget::GetAutoFromAudioButtonIsChecked)
			.OnCheckStateChanged(this, &SFlowGraphNode_YapFragmentWidget::HandleTimedModeChanged, EFlowYapTimedMode::AutomaticFromAudio)
			[
				SNew(SBox)
				[
					SNew(SImage)
					.ColorAndOpacity(FSlateColor::UseForeground())
					.Image(GEditor->GetEditorSubsystem<UFlowYapEditorSubsystem>()->GetAudioTimeBrush())
				]
			]
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Fill)
		.Padding(2, 0)
		[
			SNew(SBox)
			[
			SNew(SNumericEntryBox<double>)
			.IsEnabled(this, &SFlowGraphNode_YapFragmentWidget::GetTimeEntryEnabled)
			.Delta(0.1)
			.MinValue(0.0)
			.MinDesiredValueWidth(26)
			.Value(this, &SFlowGraphNode_YapFragmentWidget::GetEnteredTime)
			.Justification(ETextJustify::Center)
			.OnValueCommitted(this, &SFlowGraphNode_YapFragmentWidget::HandleEnteredTimeChanged)
			]
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(2, 0)
		[
			SNew(SCheckBox)
			.Style( &FAppStyle::Get().GetWidgetStyle<FCheckBoxStyle>("ToggleButtonCheckBox"))
			.Padding(FMargin(4, 3))
			.CheckBoxContentUsesAutoWidth(true)
			.IsEnabled(this, &SFlowGraphNode_YapFragmentWidget::GetUserInterruptibleButtonEnabled)
			.IsChecked(this, &SFlowGraphNode_YapFragmentWidget::GetUserInterruptible)
			.OnCheckStateChanged(this, &SFlowGraphNode_YapFragmentWidget::HandleUserInterruptibleChanged)
			[
				SNew(SBox)
				[
					SNew(SImage)
					.ColorAndOpacity(FSlateColor::UseForeground())
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

	int16 UnfocusedLines = 6;
	int16 FocusedLines = 8;
	
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

FOptionalSize SFlowGraphNode_YapFragmentWidget::GetMaxDialogueEditableTextWidgetWidth() const
{
	if (!DialogueBox.Get())
	{
		return 421;
	}
	
	if (DialogueBox->HasKeyboardFocus())
	{
		// TODO It feels weird to have the graph morphing around. Don't permit any stretching. Can I draw a whole new multiline widget over top of everything somehow eventually?
		return 421;
	}

	return 421;
}

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
	const UAkAudioEvent* Asset = GetFragment().GetDialogueAudio();

	if (!Asset) { return ""; }

	return FSoftObjectPath(Asset).GetAssetPathString();
}

void SFlowGraphNode_YapFragmentWidget::HandleDialogueAudioAssetChanged(const FAssetData& InAssetData)
{
	FFlowYapTransactions::BeginModify(INVTEXT("Audio Asset Changed"), GetFlowNodeYapDialogue());

	GetFragment().SetDialogueAudio(Cast<UAkAudioEvent>(InAssetData.GetAsset()));

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
	EFlowYapTimedMode TimedMode = GetFragment().GetTimedMode();
	
	if (TimedMode == EFlowYapTimedMode::UseProjectDefaults && QueriedMode != EFlowYapTimedMode::UseProjectDefaults)
	{
		return GetDefault<UFlowYapProjectSettings>()->GetSharedSettings().TimedMode == QueriedMode ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
	}
	
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
		GetFragment().UnsetTimedMode(Mode);
	}
	
	FFlowYapTransactions::EndModify();
}

// -----------------------------------------------------------------------------------------------
ECheckBoxState SFlowGraphNode_YapFragmentWidget::GetUserInterruptible() const
{
	if (GetFragment().GetTimedMode() == EFlowYapTimedMode::UseProjectDefaults)
	{
		const FFlowYapFragmentSharedSettings& Settings = GetDefault<UFlowYapProjectSettings>()->GetSharedSettings();

		if (Settings.TimedMode == EFlowYapTimedMode::None)
		{
			return ECheckBoxState::Unchecked;
		}
		
		return GetDefault<UFlowYapProjectSettings>()->GetSharedSettings().bUserInterruptible ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
	}

	if (GetFragment().GetTimedMode() == EFlowYapTimedMode::None)
	{
		return ECheckBoxState::Unchecked;	
	}
	
	return GetFragment().GetUserInterruptible() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SFlowGraphNode_YapFragmentWidget::HandleUserInterruptibleChanged(ECheckBoxState CheckBoxState)
{
	FFlowYapTransactions::BeginModify(INVTEXT("Interruptible Changed"), GetFlowNodeYapDialogue());
	
	GetFragment().SetUserInterruptible(CheckBoxState == ECheckBoxState::Checked ? true : false);

	FFlowYapTransactions::EndModify();
}

// -----------------------------------------------------------------------------------------------
bool SFlowGraphNode_YapFragmentWidget::GetTimeEntryEnabled() const
{	
	if (GetFragment().GetTimedMode() != EFlowYapTimedMode::UseEnteredTime)
	{
		return false;
	}
	
	return true;
}

bool SFlowGraphNode_YapFragmentWidget::GetUserInterruptibleButtonEnabled() const
{
	EFlowYapTimedMode TimedMode = GetFragment().GetTimedMode();

	if (TimedMode == EFlowYapTimedMode::UseProjectDefaults || TimedMode == EFlowYapTimedMode::None)
	{
		return false;
	}
	
	return true;
}

bool SFlowGraphNode_YapFragmentWidget::GetAutoFromAudioButtonEnabled() const
{
	if (GetIsTimedMode(EFlowYapTimedMode::UseProjectDefaults) == ECheckBoxState::Checked)
	{
		return false;
	}

	if (!GetFragment().HasDialogueAudioAsset())
	{
		return false;
	}

	return true;
}

ECheckBoxState SFlowGraphNode_YapFragmentWidget::GetAutoFromAudioButtonIsChecked() const
{
	EFlowYapTimedMode TimedMode = GetFragment().GetTimedMode();

	if (TimedMode == EFlowYapTimedMode::UseProjectDefaults)
	{
		TimedMode = GetDefault<UFlowYapProjectSettings>()->GetSharedSettings().TimedMode;
	}
	
	if (TimedMode == EFlowYapTimedMode::AutomaticFromAudio)
	{
		return GetFragment().HasDialogueAudioAsset() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
	}

	return ECheckBoxState::Unchecked;
}

// -----------------------------------------------------------------------------------------------
TOptional<double> SFlowGraphNode_YapFragmentWidget::GetEnteredTime() const
{
	TOptional<double> Value;

	if (GetFragment().GetTimedMode() == EFlowYapTimedMode::UseProjectDefaults)
	{
		const FFlowYapFragmentSharedSettings& Settings = GetDefault<UFlowYapProjectSettings>()->GetSharedSettings();

		if (Settings.TimedMode == EFlowYapTimedMode::UseEnteredTime)
		{
			Value = Settings.EnteredTime;
		}
		else
		{
			Value.Reset();
		}
	}
	else if (GetFragment().GetTimedMode() == EFlowYapTimedMode::UseEnteredTime)
	{
		Value = GetFragment().GetTimeEnteredValue();
	}
	else
	{
		Value.Reset();
	}

	// TODO: display auto time from text/audio!
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
