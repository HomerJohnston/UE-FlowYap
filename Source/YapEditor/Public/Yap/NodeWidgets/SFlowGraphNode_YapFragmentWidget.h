#pragma once

#include "CoreMinimal.h"
#include "EditorUndoClient.h"
#include "Yap/YapColors.h"
//#include "Yap/YapFragment.h"

class UYapCharacter;
struct FYapBit;
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
	float ExpandedTextEditorWidget_StartOffset = 0.f;
	float ExpandedTextEditorWidget_Offset = 0.f;
	float ExpandedTextEditorWidget_OffsetAlpha = 0.f;

	bool UseChildSafeSettings() const;

	TSharedPtr<SBox> CentreBox;
	TSharedPtr<SOverlay> FragmentWidgetOverlay;
	TSharedPtr<SWidget> MoveFragmentControls = nullptr;
	TSharedPtr<SWidget> CentreDialogueWidget;
	TSharedPtr<SWidget> CentreSettingsWidget;
	TSharedPtr<SWidget> CreateCentreDialogueWidget();
	TSharedPtr<SWidget> CreateCentreSettingsWidget();

	bool bTextEditorExpanded = false;
	TSharedPtr<SBox> ExpandedTextEditorWidget;
	TSharedPtr<SOverlay> FragmentOverlay;

	TSharedPtr<SButton> TitleTextEditButtonWidget;
	TSharedPtr<SButton> DialogueEditButtonWidget;
	TSharedPtr<SYapConditionsScrollBox> ConditionsScrollBox;

	TSharedPtr<SWidget> WTF;
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
	FSlateColor			ColorAndOpacity_ChildSafeSettingsCheckBox() const;

	FSlateColor BorderBackgroundColor_DirectedAtImage() const;
	void OnAssetsDropped_DirectedAtWidget(const FDragDropEvent& DragDropEvent, TArrayView<FAssetData> AssetDatas);
	bool OnAreAssetsAcceptableForDrop_DirectedAtWidget(TArrayView<FAssetData> AssetDatas) const;
	FReply OnClicked_DirectedAtWidget();
	const FSlateBrush* Image_DirectedAtWidget() const;
	TSharedRef<SOverlay> CreateDirectedAtWidget();
	// ------------------------------------------
	TSharedRef<SWidget> CreateFragmentWidget();

	EVisibility Visibility_DialogueEdit() const;
	TSharedRef<SWidget> CreateTextEditButtonWidget(TAttribute<EVisibility> InVisibility);
	EVisibility Visibility_EmptyTextIndicator(const FText* Text) const;	
	
	// ------------------------------------------
	TSharedRef<SWidget>	CreateDialogueWidget();

	FVector2D			DialogueScrollBar_Thickness() const;
	FOptionalSize		Dialogue_MaxDesiredHeight() const;
	FText				Text_PreviewText(const FText* MatureText, const FText* SafeText) const;
	
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
	void				OnValueChanged_FragmentTimePadding(float X);
	FSlateColor			FillColorAndOpacity_FragmentTimePadding() const;
	FText				ToolTipText_FragmentTimePadding() const;

	FSlateColor BorderBackgroundColor_PortraitImage() const;
	FText ToolTipText_PortraitWidget() const;
	void OnSetNewSpeaker(const FAssetData& AssetData);
	
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

	EVisibility Visibility_TitleTextEdit() const;

	FText Text_EditedText(FText* Text) const;
	void OnTextCommitted_EditedText(const FText& NewValue, ETextCommit::Type CommitType, FText* EditedText);
	FReply OnClicked_ExpandTextEditor(FText* MatureText, FText* SafeText, FName TextStyle, FLinearColor BaseColor);

	FText ToolTipText_PreviewText(FText Label, const FText* MatureText, const FText* SafeText) const;
	FSlateColor ColorAndOpacity_PreviewText(FLinearColor BaseColor, const FText* MatureText, const FText* SafeText) const;
	
	// ------------------------------------------
	TSharedRef<SWidget> CreateTitleTextWidget();

	EVisibility			Visibility_TitleText() const;

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
	const UFlowNode_YapDialogue* GetDialogueNode() const;

	UFlowNode_YapDialogue* GetDialogueNode();

	const FYapFragment& GetFragment() const;
	
	FYapFragment& GetFragment();

	const FYapBit& GetBit() const;
	
	FYapBit& GetBit();
	
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
};