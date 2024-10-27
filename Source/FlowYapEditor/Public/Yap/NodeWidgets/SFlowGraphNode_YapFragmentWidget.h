#pragma once

#include "CoreMinimal.h"
#include "EditorUndoClient.h"

class UFlowYapCondition;
struct FFlowYapFragment;
class SObjectPropertyEntryBox;
class SMultiLineEditableText;
class SFlowGraphNode_YapDialogueWidget;
class UFlowNode_YapDialogue;
class UFlowGraphNode_YapDialogue;
struct FFlowYapBitReplacement;

class SMultiLineEditableTextBox;

struct FGameplayTag;

enum class EFlowYapErrorLevel : uint8;

enum class EYapFragmentControlsDirection : uint8
{
	Up,
	Down,
};

class SFlowGraphNode_YapFragmentWidget : public SCompoundWidget
{
	// ------------------------------------------
	// STATE
protected:
	SFlowGraphNode_YapDialogueWidget* Owner = nullptr;

	TSharedPtr<SEditableTextBox> TitleTextBox;

	TSharedPtr<SOverlay> PortraitWidget;

	TSharedPtr<SWidget> Test;
	
	bool bCursorContained = false;
	bool MoodKeySelectorMenuOpen = false;

	uint8 FragmentIndex = 0;

	bool bCtrlPressed = false;

	uint64 LastBitReplacementCacheFrame = 0;
	FFlowYapBitReplacement* CachedBitReplacement = nullptr;

	bool bRunning;

	bool bShowSettings = false;
	bool bTitleTextExpanded = false;

	TSharedPtr<SBox> CentreBox;
	TSharedPtr<SOverlay> FragmentWidgetOverlay;
	TSharedPtr<SWidget> MoveFragmentControls = nullptr;
	TSharedPtr<SWidget> CentreDialogueWidget;
	TSharedPtr<SWidget> CentreSettingsWidget;
	TSharedPtr<SWidget> CreateCentreDialogueWidget();
	TSharedPtr<SWidget> CreateCentreSettingsWidget();

	bool bDialogueExpanded = false;
	TSharedPtr<SWidget> ExpandedDialogueWidget;
	TSharedPtr<SWidget> ExpandedTitleTextWidget;
	TSharedPtr<SOverlay> FragmentOverlay;

	TSharedPtr<SButton> TitleTextEditButtonWidget;
	TSharedPtr<SButton> DialogueEditButtonWidget;
	// ------------------------------------------
	// CONSTRUCTION
public:
	SLATE_USER_ARGS(SFlowGraphNode_YapFragmentWidget){}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, SFlowGraphNode_YapDialogueWidget* InOwner, uint8 InFragmentIndex); // non-virtual override
	bool IsBeingEdited();

	// ------------------------------------------
	// WIDGETS
protected:
	EVisibility Visibility_UpperFragmentBar() const;
	int32 GetFragmentActivationCount() const;
	int32 GetFragmentActivationLimit() const;
	EVisibility Visibility_FragmentControlsWidget() const;
	EVisibility Visibility_FragmentShiftWidget(EYapFragmentControlsDirection YapFragmentControlsDirection) const;
	FReply OnClicked_FragmentShift(EYapFragmentControlsDirection YapFragmentControlsDirection);
	FReply OnClicked_FragmentDelete();
	TSharedRef<SWidget> CreateFragmentControlsWidget();
	TSharedRef<SWidget> CreateAudioPreviewWidget();
	bool Enabled_AudioPreviewButton() const;
	TSharedRef<SWidget> CreateAudioPreviewWidget(TAttribute<EVisibility> Attribute);

	TSharedRef<SWidget> CreateFragmentHighlightWidget();
	void OnActivationLimitChanged(const FText& Text, ETextCommit::Type Arg);
	EVisibility Visibility_FragmentRowNormalControls() const;
	TSharedRef<SWidget> CreateUpperFragmentBar();
	// ------------------------------------------
	TSharedRef<SWidget> CreateFragmentWidget();

	FReply OnClicked_DialogueExpandButton();
	EVisibility Visibility_DialogueEdit() const;
	TSharedRef<SWidget> CreateTextEditButtonWidget(TAttribute<EVisibility> InVisibility);
	// ------------------------------------------
	TSharedRef<SWidget>	CreateDialogueWidget();

	FVector2D			DialogueScrollBar_Thickness() const;
	FOptionalSize		Dialogue_MaxDesiredHeight() const;
	FText				Dialogue_Text() const;
	void				OnTextCommitted_Dialogue(const FText& CommittedText, ETextCommit::Type CommitType);
	FText				ToolTipText_Dialogue() const;
	FSlateColor			BackgroundColor_Dialogue() const;
	FSlateColor			ForegroundColor_Dialogue() const;

	EVisibility			Visibility_DialogueBackground() const;
	FSlateColor			BorderBackgroundColor_Dialogue() const;
		
	// ------------------------------------------

	FText				FragmentTagPreview_Text() const;
	EVisibility			Visibility_FragmentTagPreview() const;

	// ---------------------------------------------------
	TSharedRef<SWidget> CreateConditionWidgets() const;

	EVisibility			Visibility_ConditionWidgets() const;

	// ---------------------------------------------------
	TOptional<float>	FragmentTimePadding_Percent() const;
	float				Value_FragmentTimePadding() const;
	EVisibility			FragmentTimePaddingSlider_Visibility() const;
	void				OnValueChanged_FragmentTimePadding(float X);
	FSlateColor			FragmentTimePadding_FillColorAndOpacity() const;
	FText				FragmentTimePadding_ToolTipText() const;

	FSlateColor PortraitImage_BorderBackgroundColor() const;
	FText PortraitWidget_ToolTipText() const;
	FReply OnClicked_PortraitWidget();
	
	// ------------------------------------------
	TSharedRef<SOverlay>	CreatePortraitWidget();

	EVisibility			Visibility_PortraitImage() const;
	const FSlateBrush*	Image_PortraitImage() const;
	EVisibility			Visibility_MissingPortraitWarning() const;
	EVisibility			Visibility_CharacterSelect() const;
	FString				ObjectPath_CharacterSelect() const;
	void				OnObjectChanged_CharacterSelect(const FAssetData& InAssetData);
	
	// ------------------------------------------
	TSharedRef<SBox>	CreateMoodKeySelectorWidget();

	EVisibility			Visibility_MoodKeySelector() const;
	void				OnMenuOpenChanged_MoodKeySelector(bool bMenuOpen);
	const FSlateBrush*	Image_MoodKeySelector() const;
	FGameplayTag		GetCurrentMoodKey() const;

	// ------------------------------------------
	TSharedRef<SWidget> CreateMoodKeyMenuEntryWidget(FGameplayTag InIconName, bool bSelected = false, const FText& InLabel = FText::GetEmpty(), FName InTextStyle = TEXT("ButtonText"));

	FReply				OnClicked_MoodKeyMenuEntry(FGameplayTag NewValue);

	FText TitleText_ToolTipText() const;
	EVisibility Visibility_TitleTextEdit() const;
	FReply OnClicked_TitleTextExpandButton();
	// ------------------------------------------
	TSharedRef<SWidget> CreateTitleTextWidget();

	EVisibility			TitleText_Visibility() const;
	FText				TitleText_Text() const;
	void				TitleText_OnTextCommitted(const FText& CommittedText, ETextCommit::Type CommitType);

	// ------------------------------------------
	TSharedRef<SWidget>	CreateFragmentTagWidget();
	
	FGameplayTag		Value_FragmentTag() const;
	void				OnTagChanged_FragmentTag(FGameplayTag GameplayTag);

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

	FText				AudioAsset_ObjectPathText() const;
	FString				AudioAsset_ObjectPath() const;
	void				AudioAsset_OnObjectChanged(const FAssetData& InAssetData);
	EVisibility			AudioAssetErrorState_Visibility() const;
	FSlateColor			AudioAssetErrorState_ColorAndOpacity() const;
	EFlowYapErrorLevel	AudioAssetErrorLevel() const;

	EVisibility			AudioButton_Visibility() const;

	// ------------------------------------------
	// HELPERS
protected:
	UFlowNode_YapDialogue* GetFlowYapDialogueNode() const;

	FFlowYapFragment& GetFragment() const;

	bool FragmentFocused() const;

	TSharedRef<SWidget> CreateWrappedTextBlock(FText (SFlowGraphNode_YapFragmentWidget::*TextDelegate)() const, FString TextStyle) const;
	
	EVisibility			RowHighlight_Visibility() const;
	FSlateColor			RowHighlight_BorderBackgroundColor() const;

	void SetNodeSelected();
	// ------------------------------------------
	// OVERRIDES
public:
	FSlateColor GetNodeTitleColor() const; // non-virtual override

	void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;
	
	TSharedRef<SVerticalBox>	CreateRightFragmentPane();

	TSharedPtr<SVerticalBox>	PinContainer;

	TSharedPtr<SVerticalBox> GetPinContainer() { return PinContainer; }
	
	EVisibility			Visibility_EnableOnStartPinButton() const;
	EVisibility			Visibility_EnableOnEndPinButton() const;
	
	FReply				OnClicked_EnableOnStartPinButton();
	FReply				OnClicked_EnableOnEndPinButton();

	bool GetNodeSelected() const;
};