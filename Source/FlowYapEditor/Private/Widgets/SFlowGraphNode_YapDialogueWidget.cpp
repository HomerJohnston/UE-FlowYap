#include "Widgets/SFlowGraphNode_YapDialogueWidget.h"

#include "FlowYapEditorSubsystem.h"
#include "FlowYap/Nodes/FlowNode_YapDialogue.h"
#include "Slate/DeferredCleanupSlateBrush.h"
#include "Widgets/Input/SNumericEntryBox.h"


void SFlowGraphNode_YapDialogueWidget::Construct(const FArguments& InArgs, UFlowGraphNode* InNode)
{
	SFlowGraphNode_YapSpeechBaseWidget::Construct(InArgs, InNode);
	
	FlowNode_YapDialogue = Cast<UFlowNode_YapDialogue>(FlowGraphNode_YapSpeechBase->GetFlowNode());

	UpdateGraphNode();
}

TSharedRef<SBox> SFlowGraphNode_YapDialogueWidget::GetAdditionalOptionsWidget()
{	
	TSharedPtr<SBox> Box;
	
	UTexture2D* TimedIcon = GEditor->GetEditorSubsystem<UFlowYapEditorSubsystem>()->GetDialogueTimerIco();
	
	FSlateBrush TimedBrush;
	TimedBrush.ImageSize = FVector2D(16, 16);
	TimedBrush.SetResourceObject(TimedIcon);
		
	TSharedRef<FDeferredCleanupSlateBrush> TimedIconBrush = FDeferredCleanupSlateBrush::CreateBrush(TimedBrush);

	UTexture2D* UserInterruptibleIcon = GEditor->GetEditorSubsystem<UFlowYapEditorSubsystem>()->GetDialogueUserInterruptIco();
	
	FSlateBrush InterruptibleBrush;
	InterruptibleBrush.ImageSize = FVector2D(16, 16);
	InterruptibleBrush.SetResourceObject(UserInterruptibleIcon);
	
	TSharedRef<FDeferredCleanupSlateBrush> InterruptibleIconBrush = FDeferredCleanupSlateBrush::CreateBrush(InterruptibleBrush);

	SAssignNew(Box, SBox)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SSpacer)
			.Size(1)
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SCheckBox)
			.Style( &FAppStyle::Get().GetWidgetStyle<FCheckBoxStyle>("ToggleButtonCheckBox"))
			.Padding(FMargin(4, 3))
			.CheckBoxContentUsesAutoWidth(true)
			.IsChecked(this, &SFlowGraphNode_YapDialogueWidget::GetUserInterruptibleEnabled)
			.OnCheckStateChanged(this, &SFlowGraphNode_YapDialogueWidget::HandleInterruptibleChanged)
			[
				SNew(SBox)
				[
					SNew(SImage)
					.ColorAndOpacity(FSlateColor::UseForeground())
					.Image(TAttribute<const FSlateBrush*>::Create(
					TAttribute<const FSlateBrush*>::FGetter::CreateLambda([InterruptibleIconBrush](){return InterruptibleIconBrush->GetSlateBrush();})))
				]
			]
		]
		+ SHorizontalBox::Slot()
		.FillWidth(1.0)
		[
			SNew(SSpacer)
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SCheckBox)
			.Style(&FAppStyle::Get().GetWidgetStyle<FCheckBoxStyle>("ToggleButtonCheckBox"))
			.Padding(FMargin(4, 2))
			.CheckBoxContentUsesAutoWidth(true)
			.IsChecked(this, &SFlowGraphNode_YapDialogueWidget::GetTimedEnabled)
			.OnCheckStateChanged(this, &SFlowGraphNode_YapDialogueWidget::HandleTimedChanged)
			[
				SNew(SImage)
				.ColorAndOpacity(FSlateColor::UseForeground())
				.Image(TAttribute<const FSlateBrush*>::Create(
				TAttribute<const FSlateBrush*>::FGetter::CreateLambda([TimedIconBrush](){return TimedIconBrush->GetSlateBrush();})))
			]
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SSpacer)
			.Size(1)
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SNumericEntryBox<double>)
			.IsEnabled(this, &SFlowGraphNode_YapDialogueWidget::GetTimeEntryEnabled)
			.Delta(0.1)
			.MinValue(0.0)
			.MaxSliderValue(30.0)
			.MinDesiredValueWidth(40)
			.Value(this, &SFlowGraphNode_YapDialogueWidget::GetTime)
			.Justification(ETextJustify::Center)
			.OnValueCommitted(this, &SFlowGraphNode_YapDialogueWidget::HandleTimeChanged)
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SSpacer)
			.Size(1)
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SCheckBox)
			.IsEnabled(this, &SFlowGraphNode_YapDialogueWidget::GetUseAudioLengthEnabled)
			.IsChecked(this, &SFlowGraphNode_YapDialogueWidget::GetUseAudioLength)
			.OnCheckStateChanged(this, &SFlowGraphNode_YapDialogueWidget::HandleUseAudioLengthChanged)
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.IsEnabled(this, &SFlowGraphNode_YapDialogueWidget::GetUseAudioLengthEnabled)
			.Text(INVTEXT("Use Audio Length"))
		]
	];

	return Box.ToSharedRef(); 
}

bool SFlowGraphNode_YapDialogueWidget::GetTimeEntryEnabled() const
{
	return FlowNode_YapDialogue->GetTimed() && (!FlowNode_YapDialogue->GetUseAudioAssetLength() || !FlowNode_YapDialogue->GetDialogueAudio());
}

ECheckBoxState SFlowGraphNode_YapDialogueWidget::GetTimedEnabled() const
{
	return FlowNode_YapDialogue->GetTimed() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

bool SFlowGraphNode_YapDialogueWidget::GetUseAudioLengthEnabled() const
{
	return FlowNode_YapDialogue->GetTimed() && FlowNode_YapDialogue->GetDialogueAudio();
}

ECheckBoxState SFlowGraphNode_YapDialogueWidget::GetUseAudioLength() const
{
	return (FlowNode_YapDialogue->GetUseAudioAssetLength() && FlowNode_YapDialogue->GetDialogueAudio()) ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

ECheckBoxState SFlowGraphNode_YapDialogueWidget::GetUserInterruptibleEnabled() const
{
	return FlowNode_YapDialogue->GetUserInterruptible() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

TOptional<double> SFlowGraphNode_YapDialogueWidget::GetTime() const
{
	TOptional<double> Value;

	if (FlowNode_YapDialogue->GetTimed() && !FlowNode_YapDialogue->GetUseAudioAssetLength())
	{
		Value = FlowNode_YapDialogue->GetTime();
	}
	else
	{
		Value.Reset();
	}
	
	return Value;
}

void SFlowGraphNode_YapDialogueWidget::HandleTimedChanged(ECheckBoxState CheckBoxState)
{
	FlowNode_YapDialogue->SetTimed(CheckBoxState == ECheckBoxState::Checked ? true : false);
}

void SFlowGraphNode_YapDialogueWidget::HandleInterruptibleChanged(ECheckBoxState CheckBoxState)
{
	FlowNode_YapDialogue->SetUserInterruptible(CheckBoxState == ECheckBoxState::Checked ? true : false);
}

void SFlowGraphNode_YapDialogueWidget::HandleTimeChanged(double NewValue, ETextCommit::Type CommitType)
{
	if (CommitType == ETextCommit::OnEnter || CommitType == ETextCommit::OnUserMovedFocus)
	{
		FlowNode_YapDialogue->SetTime(NewValue);
	}

	if (CommitType == ETextCommit::OnCleared)
	{
		FlowNode_YapDialogue->SetTime(0.0);
	}
}

void SFlowGraphNode_YapDialogueWidget::HandleUseAudioLengthChanged(ECheckBoxState CheckBoxState)
{
	FlowNode_YapDialogue->SetUseAudioAssetLength(CheckBoxState == ECheckBoxState::Checked ? true : false);
}

