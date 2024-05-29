#pragma once

#include "CoreMinimal.h"
#include "GraphNodes/FlowGraphNode_YapDialogue.h"
#include "Graph/Widgets/SFlowGraphNode.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/SUserWidget.h"
#include "Widgets/Input/SSegmentedControl.h"

class SFlowGraphNode_YapDialogueWidget;
class UFlowNode_YapDialogue;
class UFlowGraphNode_YapDialogue;

class SMultiLineEditableTextBox;


class SFlowGraphNode_YapFragmentWidget : public SCompoundWidget
{
public:
	SLATE_USER_ARGS(SFlowGraphNode_YapFragmentWidget){}
	SLATE_END_ARGS()
	
	void Construct(const FArguments& InArgs, SFlowGraphNode_YapDialogueWidget* InOwner, FFlowYapFragment* InFragment);

protected:
	// TODO: Is this safe?
	SFlowGraphNode_YapDialogueWidget* Owner = nullptr;

	int64 FragmentID = -1;
	
protected:
	TSharedRef<SBox> GetAdditionalOptionsWidget();
	
protected:
	TSharedRef<SWidget> CreateDialogueContentArea();

	TSharedRef<SBox> GetPortraitWidget();
	
	TSharedRef<SBox> GetPortraitKeySelectorWidget();
	
	TSharedRef<SWidget> CreatePortraitKeyButton(FName InIconName = FName(), bool bSelected = false, const FText& InLabel = FText::GetEmpty(), FName InTextStyle = TEXT("ButtonText"));

	FOptionalSize GetMaxDialogueWidgetHeight() const;
	
	FOptionalSize GetMaxDialogueWidgetWidth() const;
		
public:
	FText GetTitleText() const;

	FText GetDialogueText() const;

	FString GetSelectedDialogueAudioAssetPath() const;

	FName GetPortraitKey() const;

	const FSlateBrush* GetPortraitBrush() const;

	const FSlateBrush* GetPortraitKeyBrush() const;
	
	EVisibility GetVisibilityForMissingPortraitText() const;

	UFlowNode_YapDialogue* GetFlowNodeYapDialogue() const;

	FFlowYapFragment& GetFragment() const;
	
protected:
	void HandleTitleTextCommitted(const FText& CommittedText, ETextCommit::Type CommitType);
	
	void HandleDialogueTextCommitted(const FText& CommittedText, ETextCommit::Type CommitType);

	void HandleDialogueAudioAssetChanged(const FAssetData& InAssetData);

	FReply HandlePortraitKeyChanged(FName NewValue);

protected:
	
	TSharedPtr<SMultiLineEditableTextBox> DialogueBox;

	SVerticalBox DialogueEntries;
	
protected:
	ECheckBoxState GetUserInterruptibleEnabled() const;

	bool GetTimeEntryEnabled() const;

	bool GetUseAutoTimeEnabled() const;

	bool GetUseAudioLengthEnabled() const;

	ECheckBoxState GetTimed() const;
	
	TOptional<double> GetTimeManualValue() const;
	
	ECheckBoxState GetUseAutoTime() const;

	ECheckBoxState GetUseAudioLength() const;

protected:
	void HandleInterruptibleChanged(ECheckBoxState CheckBoxState);

	void HandleTimedChanged(ECheckBoxState CheckBoxState);

	void HandleTimeChanged(double NewValue, ETextCommit::Type CommitType);

	void HandleUseAutoTimeChanged(ECheckBoxState CheckBoxState);

	void HandleUseAudioLengthChanged(ECheckBoxState CheckBoxState);

	FCheckBoxStyle Style;
};