#pragma once

#include "CoreMinimal.h"
#include "GraphNodes/FlowGraphNode_YapDialogue.h"
#include "Graph/Widgets/SFlowGraphNode.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/SUserWidget.h"
#include "Widgets/Input/SSegmentedControl.h"

class SObjectPropertyEntryBox;
class SMultiLineEditableText;
class SFlowGraphNode_YapDialogueWidget;
class UFlowNode_YapDialogue;
class UFlowGraphNode_YapDialogue;

class SMultiLineEditableTextBox;

enum class EFlowYapTimeMode : uint8;

class SFlowGraphNode_YapFragmentWidget : public SCompoundWidget
{
public:
	SLATE_USER_ARGS(SFlowGraphNode_YapFragmentWidget){}
	SLATE_END_ARGS()
	
	void Construct(const FArguments& InArgs, SFlowGraphNode_YapDialogueWidget* InOwner, FFlowYapFragment* InFragment);

	// STATE
protected:
	SFlowGraphNode_YapDialogueWidget* Owner = nullptr; // TODO TSharedPtr safer?

	FFlowYapFragment* Fragment = nullptr;

	TSharedPtr<SMultiLineEditableTextBox> DialogueBox;

	TSharedPtr<SObjectPropertyEntryBox> AudioAssetProperty;

	TSharedPtr<SEditableTextBox> TitleTextBox;
	
	SVerticalBox DialogueEntries;
	
	bool bCursorContained = false;
	bool bShiftPressed = false;
	bool bShiftCaptured = false;

	bool bControlPressed = false;
	
protected:
	// ----------------------------------------------

	FText DialogueText_ToolTipText() const;
	
	TSharedRef<SWidget> CreateDialogueContentArea();
	
	FOptionalSize Fragment_WidthOverride() const;
	
	FOptionalSize DialogueText_MaxDesiredHeight() const;

	FText DialogueText_Text() const;
	
	void DialogueText_OnTextCommitted(const FText& CommittedText, ETextCommit::Type CommitType);
	
	FSlateColor DialogueText_ForegroundColor() const;
	
	FSlateColor DialogueText_BackgroundColor() const;

	EVisibility FragmentLowerControls_Visibility() const;
	
	EVisibility TitleText_Visibility() const;
	
	FText TitleText_Text() const;

	void TitleText_OnTextCommitted(const FText& CommittedText, ETextCommit::Type CommitType);
	
	EVisibility DialogueAudioAssetWarningState_Visibility() const;
	bool DialogueAudioAssetInWarningState() const; // I've used separate functions for the actual conditions here because I call these in multiple places, and EVisibility is a struct which is annoying to compare against
	
	EVisibility DialogueAudioAssetErrorState_Visibility() const;
	bool DialogueAudioAssetInErrorState() const;
	// ----------------------------------------------
	
	TSharedRef<SBox> CreatePortraitWidget();

	EVisibility PortraitImage_Visibility() const;


	FReply OnClickDialogueTextBox();
	


	

	const FSlateBrush* PortraitImage_Image() const;

	FSlateColor GetNodeTitleColor() const;
	
	EVisibility MissingPortraitWarning_Visibility() const;
	
	TSharedRef<SBox> CreatePortraitKeySelector();
	
	TSharedRef<SWidget> CreatePortraitKeyMenuEntry(FName InIconName = FName(), bool bSelected = false, const FText& InLabel = FText::GetEmpty(), FName InTextStyle = TEXT("ButtonText"));

	const FSlateBrush* GetPortraitKeyBrush() const;

	TSharedRef<SBox> CreateTimeSettingsWidget();
	
	bool UseManuallyEnteredTimeButton_IsEnabled() const;
	
	ECheckBoxState UseManuallyEnteredTimeButton_IsChecked() const;
	
	void UseManuallyEnteredTimeButton_OnCheckStateChanged(ECheckBoxState CheckBoxState);
	
	bool UseTextTimeButton_IsEnabled() const;
	
	ECheckBoxState UseTextTimeButton_IsChecked() const;
	
	void UseTextTimeButton_OnCheckStateChanged(ECheckBoxState CheckBoxState);
	
	// Fragment settings
protected:

	
	FString DialogueAudioAsset_ObjectPath() const;
	void DialogueAudioAsset_OnObjectChanged(const FAssetData& InAssetData);

	FName GetPortraitKey() const;
	FReply HandlePortraitKeyChanged(FName NewValue);

	void UseAudioTimeButton_OnCheckStateChanged(ECheckBoxState CheckBoxState);

	// Time Settings
protected:
	bool IsManualTimeEntryEnabled() const;
	TOptional<double> TimeEntryBox_Value() const;
	void TimeEntryBox_OnValueCommitted(double NewValue, ETextCommit::Type CommitType);

	bool GetEnabled_UseTextTimeButton() const;

	ECheckBoxState InterruptibleButton_IsChecked() const;
	void InterruptibleButton_OnCheckStateChanged(ECheckBoxState CheckBoxState);

	ECheckBoxState UseProjectDefaultTimeSettingsButton_IsChecked() const;
	void UseProjectDefaultTimeSettingsButton_OnCheckStateChanged(ECheckBoxState CheckBoxState);

	bool GetEnabled_TimeEntryBox() const;


	bool InterruptibleButton_IsEnabled() const; 

	bool UseAudioTimeButton_IsEnabled() const;

	ECheckBoxState UseAudioTimeButton_IsChecked() const;
	

protected:
	
	UFlowNode_YapDialogue* GetFlowNodeYapDialogue() const;
	
	void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;
};