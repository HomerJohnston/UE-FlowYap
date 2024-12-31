#pragma once

#include "CoreMinimal.h"
#include "EditorUndoClient.h"

class SYapConditionsScrollBox;
struct FFlowPin;
class UYapCondition;
struct FYapFragment;
class SObjectPropertyEntryBox;
class SMultiLineEditableText;
class SFlowGraphNode_YapDialogueWidget;
class UFlowNode_YapDialogue;
class UFlowGraphNode_YapDialogue;
struct FYapBitReplacement;

class SMultiLineEditableTextBox;

struct FGameplayTag;

enum class EYapErrorLevel : uint8;

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

	bool bCursorContained = false;
	bool MoodKeySelectorMenuOpen = false;

	uint8 FragmentIndex = 0;

	bool bCtrlPressed = false;

	float Opacity = 0;

	double InitTime = -1;
	
	uint64 LastBitReplacementCacheFrame = 0;
	FYapBitReplacement* CachedBitReplacement = nullptr;

	bool bShowSettings = false;
	bool bTitleTextExpanded = false;
	bool bShowMaturitySettings = false;
	static bool bShowMaturitySettingsGlobal;

	bool GetShowMaturitySettings() const
	{
		return bShowMaturitySettings || bShowMaturitySettingsGlobal;
	}
	
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
	TSharedPtr<SYapConditionsScrollBox> ConditionsScrollBox;

public:
	TSharedPtr<SYapConditionsScrollBox> GetConditionsScrollBox() { return ConditionsScrollBox; }
	
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
	void OnTextCommitted_FragmentActivationLimit(const FText& Text, ETextCommit::Type Arg);
	EVisibility Visibility_FragmentRowNormalControls() const;

	TSharedRef<SWidget> CreateUpperFragmentBar();
	EVisibility Visibility_FragmentTagWidget() const;
	
	ECheckBoxState IsChecked_SkippableToggle() const;
	FSlateColor ColorAndOpacity_SkippableToggleIcon() const;
	EVisibility Visibility_SkippableToggleIconOff() const;
	void OnCheckStateChanged_SkippableToggle(ECheckBoxState CheckBoxState);

	ECheckBoxState		IsChecked_MaturitySettings() const;
	void				OnCheckStateChanged_MaturitySettings(ECheckBoxState CheckBoxState);
	FSlateColor			ColorAndOpacity_MaturitySettingsCheckBox() const;

	// ------------------------------------------
	TSharedRef<SWidget> CreateFragmentWidget();

	FReply OnClicked_DialogueExpandButton();
	EVisibility Visibility_DialogueEdit() const;
	TSharedRef<SWidget> CreateTextEditButtonWidget(TAttribute<EVisibility> InVisibility);
	// ------------------------------------------
	TSharedRef<SWidget>	CreateDialogueWidget();

	FVector2D			DialogueScrollBar_Thickness() const;
	FOptionalSize		Dialogue_MaxDesiredHeight() const;
	FText				Text_Dialogue() const;
	void				OnTextCommitted_Dialogue(const FText& CommittedText, ETextCommit::Type CommitType);
	void				OnTextCommitted_DialogueSafe(const FText& CommittedText, ETextCommit::Type CommitType);
	FText				ToolTipText_Dialogue() const;
	FSlateColor			BackgroundColor_Dialogue() const;
	FLinearColor			ForegroundColor_Dialogue() const;

	FSlateColor ColorAndOpacity_DialogueText() const;
	EVisibility			Visibility_DialogueBackground() const;
	FSlateColor			BorderBackgroundColor_Dialogue() const;
		
	// ------------------------------------------

	FText				FragmentTagPreview_Text() const;

	// ---------------------------------------------------
	EVisibility			Visibility_ConditionWidgets() const;

	void				OnConditionsUpdated();
	
	// ---------------------------------------------------
	TOptional<float>	FragmentTimePadding_Percent() const;
	float				Value_FragmentTimePadding() const;
	EVisibility			FragmentTimePaddingSlider_Visibility() const;
	void				OnValueChanged_FragmentTimePadding(float X);
	FSlateColor			FillColorAndOpacity_FragmentTimePadding() const;
	FText				ToolTipText_FragmentTimePadding() const;

	FSlateColor BorderBackgroundColor_PortraitImage() const;
	FText ToolTipText_PortraitWidget() const;
	FReply OnClicked_PortraitWidget();

	FText Text_PortraitWidget() const;
	bool OnAreAssetsAcceptableForDrop_PortraitWidget(TArrayView<FAssetData> AssetDatas) const;
	void OnAssetsDropped_PortraitWidget(const FDragDropEvent& DragDropEvent, TArrayView<FAssetData> AssetDatas);
	// ------------------------------------------
	TSharedRef<SOverlay>	CreatePortraitWidget();

	EVisibility			Visibility_PortraitImage() const;
	const FSlateBrush*	Image_PortraitImage() const;
	EVisibility			Visibility_MissingPortraitWarning() const;
	EVisibility			Visibility_CharacterSelect() const;
	FString				ObjectPath_CharacterSelect() const;
	void				OnObjectChanged_CharacterSelect(const FAssetData& InAssetData);

	FText ToolTipText_MoodKeySelector() const;
	// ------------------------------------------
	TSharedRef<SBox>	CreateMoodKeySelectorWidget();

	EVisibility			Visibility_MoodKeySelector() const;
	void				OnMenuOpenChanged_MoodKeySelector(bool bMenuOpen);
	const FSlateBrush*	Image_MoodKeySelector() const;
	FGameplayTag		GetCurrentMoodKey() const;
	
	// ------------------------------------------
	TSharedRef<SWidget> CreateMoodKeyMenuEntryWidget(FGameplayTag InIconName, bool bSelected = false, const FText& InLabel = FText::GetEmpty(), FName InTextStyle = TEXT("ButtonText"));

	FReply				OnClicked_MoodKeyMenuEntry(FGameplayTag NewValue);

	FText ToolTipText_TitleText() const;
	EVisibility Visibility_TitleTextEdit() const;
	FReply OnClicked_TitleTextExpandButton();
	// ------------------------------------------
	TSharedRef<SWidget> CreateTitleTextWidget();

	EVisibility			Visibility_TitleText() const;
	FText				Text_TitleText() const;
	void				OnTextCommitted_TitleText(const FText& CommittedText, ETextCommit::Type CommitType);
	void				OnTextCommitted_TitleTextSafe(const FText& CommittedText, ETextCommit::Type CommitType);

	// ------------------------------------------
	TSharedRef<SWidget>	CreateFragmentTagWidget();
	
	FGameplayTag		Value_FragmentTag() const;
	void				OnTagChanged_FragmentTag(FGameplayTag GameplayTag);

	// ------------------------------------------
	TSharedRef<SBox>	CreateBottomRowWidget();

	ECheckBoxState		IsChecked_UseProjectDefaultTimeSettingsButton() const;
	void				OnCheckStateChanged_UseProjectDefaultTimeSettingsButton(ECheckBoxState CheckBoxState);
	
	bool				IsEnabled_UseManuallyEnteredTimeButton() const;
	ECheckBoxState		IsChecked_UseManuallyEnteredTimeButton() const;
	void				OnCheckStateChanged_UseManuallyEnteredTimeButton(ECheckBoxState CheckBoxState);

	bool				IsEnabled_UseTextTimeButton() const;
	ECheckBoxState		IsChecked_UseTextTimeButton() const;
	void				OnCheckStateChanged_UseTextTimeButton(ECheckBoxState CheckBoxState);
	
	bool				IsEnabled_UseAudioTimeButton() const;
	ECheckBoxState		IsChecked_UseAudioTimeButton() const;
	void				OnCheckStateChanged_UseAudioTimeButton(ECheckBoxState CheckBoxState);

	bool				IsEnabled_TimeEntryBox() const;
	TOptional<double>	Value_TimeEntryBox() const;
	void				OnValueCommitted_TimeEntryBox(double NewValue, ETextCommit::Type CommitType);

	// ------------------------------------------
	TSharedRef<SWidget> CreateAudioAssetWidget(TAttribute<EVisibility> VisibilityAtt, TAttribute<FString> ObjectPathAtt, TDelegate<void(const FAssetData&)> OnObjectChangedAtt);

	FText				ObjectPathText_AudioAsset() const;
	FString				ObjectPath_AudioAsset() const;
	void				OnObjectChanged_AudioAsset(const FAssetData& InAssetData);
	EVisibility			Visibility_AudioAssetErrorState() const;
	FSlateColor			ColorAndOpacity_AudioAssetErrorState() const;
	EYapErrorLevel	AudioAssetErrorLevel() const;

	EVisibility			Visibility_AudioButton() const;

	// ------------------------------------------
	// HELPERS
protected:
	UFlowNode_YapDialogue* GetDialogueNode() const;

	FYapFragment& GetFragment() const;

	bool IsFragmentFocused() const;

	EVisibility			Visibility_RowHighlight() const;
	FSlateColor			BorderBackgroundColor_RowHighlight() const;

	// ------------------------------------------
	// OVERRIDES
public:
	FSlateColor GetNodeTitleColor() const; // non-virtual override

	void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;
	
	TSharedRef<SWidget>	CreateRightFragmentPane();

	TSharedPtr<SBox>	EndPinBox;
	TSharedPtr<SBox>	StartPinBox;
	TSharedPtr<SBox>	PromptOutPinBox;
	
	TSharedPtr<SBox> GetPinContainer(const FFlowPin& Pin);
	
	EVisibility			Visibility_EnableOnStartPinButton() const;
	EVisibility			Visibility_EnableOnEndPinButton() const;
	
	FReply				OnClicked_EnableOnStartPinButton();
	FReply				OnClicked_EnableOnEndPinButton();

	bool GetNodeSelected() const;
};