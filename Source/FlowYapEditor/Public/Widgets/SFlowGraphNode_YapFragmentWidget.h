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

enum class EFlowYapErrorLevel : uint8;

class SFlowGraphNode_YapFragmentWidget : public SCompoundWidget
{
	// ------------------------------------------
	// STATE
protected:
	SFlowGraphNode_YapDialogueWidget* Owner = nullptr;

	TSharedPtr<SMultiLineEditableTextBox> DialogueBox;
	TSharedPtr<SEditableTextBox> TitleTextBox;
	TSharedPtr<SObjectPropertyEntryBox> AudioAssetProperty;

	static FButtonStyle MoveFragmentButtonStyle;
	static bool bStylesInitialized;
	
	bool bCursorContained = false;
	bool MoodKeySelectorMenuOpen = false;

	uint8 FragmentIndex;
	
	// ------------------------------------------
	// CONSTRUCTION
public:
	SLATE_USER_ARGS(SFlowGraphNode_YapFragmentWidget){}
	SLATE_END_ARGS()
	void Construct(const FArguments& InArgs, SFlowGraphNode_YapDialogueWidget* InOwner, uint8 InFragmentIndex); // non-virtual override

	// ------------------------------------------
	// WIDGETS
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
	// HELPERS
protected:
	UFlowNode_YapDialogue* GetFlowYapDialogueNode() const;

	FFlowYapFragment* GetFragment() const;

	// ------------------------------------------
	// OVERRIDES
public:
	FSlateColor GetNodeTitleColor() const; // non-virtual override

	void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;
};