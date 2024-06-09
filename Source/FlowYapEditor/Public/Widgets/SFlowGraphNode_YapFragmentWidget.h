#pragma once

#include "CoreMinimal.h"
#include "GraphNodes/FlowGraphNode_YapDialogue.h"
#include "Widgets/SCompoundWidget.h"

class SObjectPropertyEntryBox;
class SMultiLineEditableText;
class SFlowGraphNode_YapDialogueWidget;
class UFlowNode_YapDialogue;
class UFlowGraphNode_YapDialogue;

class SMultiLineEditableTextBox;

enum class EFlowYapTimeMode : uint8;
enum class EFlowYapErrorLevel : uint8;

class SFlowGraphNode_YapFragmentWidget : public SCompoundWidget
{
public:
	SLATE_USER_ARGS(SFlowGraphNode_YapFragmentWidget){}
	SLATE_END_ARGS()
	
	// STATE
protected:
	SFlowGraphNode_YapDialogueWidget* Owner = nullptr; // TODO TSharedPtr safer?

	FFlowYapFragment* Fragment = nullptr;

	TSharedPtr<SMultiLineEditableTextBox> DialogueBox;

	TSharedPtr<SEditableTextBox> TitleTextBox;
	
	TSharedPtr<SObjectPropertyEntryBox> AudioAssetProperty;
	
	SVerticalBox DialogueEntries;

	// TODO static?
	FButtonStyle MoveFragmentButtonStyle;
	
	FButtonStyle DeleteFragmentButtonStyle;
	
	bool bCursorContained = false;
	bool bShiftPressed = false;
	bool bShiftCaptured = false;

	bool bControlPressed = false;

	bool MoodKeySelectorMenuOpen = false;

public:
	void Construct(const FArguments& InArgs, SFlowGraphNode_YapDialogueWidget* InOwner, FFlowYapFragment* InFragment); // non-virtual override

protected:
	// ------------------------------------------
	TSharedRef<SWidget> CreateFragmentWidget();

	FOptionalSize		Fragment_WidthOverride() const;
	EVisibility			FragmentBottomSection_Visibility() const;
	

	// ------------------------------------------
	TSharedRef<SBox>	CreateDialogueWidget();
	
	FOptionalSize		Dialogue_MaxDesiredHeight() const;
	FText				Dialogue_Text() const;
	void				Dialogue_OnTextCommitted(const FText& CommittedText, ETextCommit::Type CommitType);
	FText				Dialogue_ToolTipText() const;
	FSlateColor			Dialogue_BackgroundColor() const;
	FSlateColor			Dialogue_ForegroundColor() const;

	// ------------------------------------------
	TSharedRef<SBox>	CreatePortraitWidget();

	EVisibility			PortraitImage_Visibility() const;
	const FSlateBrush*	PortraitImage_Image() const;
	EVisibility			MissingPortraitWarning_Visibility() const;

	// ------------------------------------------
	TSharedRef<SBox>	CreateMoodKeySelectorWidget();

	EVisibility			MoodKeySelector_Visibility() const;
	void				MoodKeySelector_OnMenuOpenChanged(bool bMenuOpen);
	const FSlateBrush*	MoodKeyBrush_GetBrush() const;
	FName				GetCurrentMoodKey() const;

	// ------------------------------------------
	TSharedRef<SWidget> CreateMoodKeyMenuEntryWidget(FName InIconName = FName(), bool bSelected = false, const FText& InLabel = FText::GetEmpty(), FName InTextStyle = TEXT("ButtonText"));

	FReply				MoodKeyMenuEntry_OnClicked(FName NewValue);

	// ------------------------------------------
	TSharedRef<SBox>	CreateFragmentControlsWidget();

	EVisibility			FragmentControls_Visibility() const;
	EVisibility			MoveFragmentUpButton_Visibility() const;
	FReply				MoveFragmentUpButton_OnClicked();
	EVisibility			DeleteFragmentButton_Visibility() const;
	FReply				DeleteFragmentButton_OnClicked();
	EVisibility			MoveFragmentDownButton_Visibility() const;
	FReply				MoveFragmentDownButton_OnClicked();

	// ------------------------------------------
	TSharedRef<SWidget> CreateTitleTextWidget();

	EVisibility			TitleText_Visibility() const;
	FText				TitleText_Text() const;
	void				TitleText_OnTextCommitted(const FText& CommittedText, ETextCommit::Type CommitType);

	// ------------------------------------------
	TSharedRef<SBox>	CreateBottomRowWidget();

	ECheckBoxState		UseProjectDefaultTimeSettingsButton_IsChecked() const;
	void				UseProjectDefaultTimeSettingsButton_OnCheckStateChanged(ECheckBoxState CheckBoxState);
	
	bool				UseManuallyEnteredTimeButton_IsEnabled() const;
	ECheckBoxState		UseManuallyEnteredTimeButton_IsChecked() const;
	void				UseManuallyEnteredTimeButton_OnCheckStateChanged(ECheckBoxState CheckBoxState);

	bool				UseTextTimeButton_IsEnabled() const;
	ECheckBoxState		UseTextTimeButton_IsChecked() const;
	void				UseTextTimeButton_OnCheckStateChanged(ECheckBoxState CheckBoxState);
	
	bool				UseAudioTimeButton_IsEnabled() const;
	ECheckBoxState		UseAudioTimeButton_IsChecked() const;
	void				UseAudioTimeButton_OnCheckStateChanged(ECheckBoxState CheckBoxState);

	bool				TimeEntryBox_IsEnabled() const;
	TOptional<double>	TimeEntryBox_Value() const;
	void				TimeEntryBox_OnValueCommitted(double NewValue, ETextCommit::Type CommitType);

	// ------------------------------------------
	TSharedRef<SWidget> CreateAudioAssetWidget();

	FString				AudioAsset_ObjectPath() const;
	void				AudioAsset_OnObjectChanged(const FAssetData& InAssetData);
	EVisibility			AudioAssetErrorState_Visibility() const;
	FSlateColor			AudioAssetErrorState_ColorAndOpacity() const;
	EFlowYapErrorLevel	AudioAssetErrorLevel() const;

	// ------------------------------------------
protected:
	UFlowNode_YapDialogue* GetFlowYapDialogueNode() const;

public:
	void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;
	FSlateColor GetNodeTitleColor() const; // non-virtual override
};