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

enum class EFlowYapTimedMode : uint8;

class SFlowGraphNode_YapFragmentWidget : public SCompoundWidget
{
public:
	SLATE_USER_ARGS(SFlowGraphNode_YapFragmentWidget){}
	SLATE_END_ARGS()
	
	void Construct(const FArguments& InArgs, SFlowGraphNode_YapDialogueWidget* InOwner, FFlowYapFragment* InFragment);

	// STATE
protected:
	SFlowGraphNode_YapDialogueWidget* Owner = nullptr; // TODO: Is this safe?

	int64 FragmentID = -1;

	TSharedPtr<SMultiLineEditableTextBox> DialogueBox;

	TSharedPtr<SObjectPropertyEntryBox> AudioAssetProperty;

	TSharedPtr<SEditableTextBox> TitleTextBox;
	
	SVerticalBox DialogueEntries;
	
	UClass* DialogueAssetClass = nullptr;

	bool bCursorContained = false;

	bool bShiftPressed = false;
	
	bool bShiftCaptured = false;

	
protected:
	EVisibility GetPortraitWidgetVisibility() const;

	EVisibility GetTitleTextEntryVisibility() const;

	FReply OnClickPortrait();
	
	TSharedRef<SWidget> CreateDialogueContentArea();

	FOptionalSize GetDialogueWidgetWidthAdjustment() const;

	FSlateColor GetDialogueTextColor() const;
	
	FSlateColor GetDialogueTextBackgroundColor() const;
	
	TSharedRef<SBox> CreatePortraitWidget();

	const FSlateBrush* GetPortraitBrush() const;

	FSlateColor GetNodeTitleColor() const;
	
	EVisibility GetVisibilityForMissingPortraitText() const;
	
	TSharedRef<SBox> CreatePortraitKeySelector();
	
	TSharedRef<SWidget> CreatePortraitKeyMenuEntry(FName InIconName = FName(), bool bSelected = false, const FText& InLabel = FText::GetEmpty(), FName InTextStyle = TEXT("ButtonText"));

	const FSlateBrush* GetPortraitKeyBrush() const;

	TSharedRef<SBox> CreateTimeSettingsWidget();
	
	FOptionalSize GetMaxDialogueEditableTextWidgetHeight() const;
	
	// Fragment settings
protected:
	FText GetTitleText() const;
	void HandleTitleTextCommitted(const FText& CommittedText, ETextCommit::Type CommitType);

	FText GetDialogueText() const;
	void HandleDialogueTextCommitted(const FText& CommittedText, ETextCommit::Type CommitType);
	
	FString GetSelectedDialogueAudioAssetPath() const;
	void HandleDialogueAudioAssetChanged(const FAssetData& InAssetData);

	FName GetPortraitKey() const;
	FReply HandlePortraitKeyChanged(FName NewValue);

	bool GetIsNotTimedMode(EFlowYapTimedMode TimedMode) const;
	ECheckBoxState GetIsTimedMode(EFlowYapTimedMode QueriedMode) const;
	void HandleTimedModeChanged(ECheckBoxState CheckBoxState, EFlowYapTimedMode FlowYapTimedMode);

	// Time Settings
protected:
	bool IsUseEnteredTimeEnabled() const;
	TOptional<double> GetEnteredTime() const;
	void HandleEnteredTimeChanged(double NewValue, ETextCommit::Type CommitType);

	bool IsUseTimeFromTextEnabled() const;

	ECheckBoxState GetUserInterruptibleChecked() const;
	void HandleUserInterruptibleChanged(ECheckBoxState CheckBoxState);

	ECheckBoxState GetUseProjectDefaultTimeSettingsChecked() const;
	void HandleUseProjectDefaultTimeSettingsChanged(ECheckBoxState CheckBoxState);

	bool GetTimeEntryEnabled() const;


	bool GetUserInterruptibleButtonEnabled() const; 

	bool IsUseTimeFromAudioEnabled() const;

	ECheckBoxState GetUseTimeFromAudioChecked() const;
	
	EVisibility GetUseTimeFromAudioButtonErrorState() const;

	EVisibility GetSelectedDialogueAudioAssetIsValid() const;

	EVisibility DisplayAllLowerFragmentControls() const;
protected:
	
	UFlowNode_YapDialogue* GetFlowNodeYapDialogue() const;
	
	FFlowYapFragment& GetFragment() const;

	FFlowYapFragment& GetFragmentMutable();

	void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;
};