// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license.

#pragma once

#include "CoreMinimal.h"
#include "EditorUndoClient.h"
#include "Yap/YapTimeMode.h"

enum class EYapDialogueProgressionFlags : uint8;
class UYapCharacter;
class SYapConditionsScrollBox;
class UYapCondition;
class SObjectPropertyEntryBox;
class SMultiLineEditableText;
class SFlowGraphNode_YapDialogueWidget;
class UFlowNode_YapDialogue;
class UFlowGraphNode_YapDialogue;
class SMultiLineEditableTextBox;
class UYapBroker;

struct FYapBit;
struct FFlowPin;
struct FYapBitReplacement;
struct FYapFragment;
struct FGameplayTag;

enum class EYapTimeMode : uint8;
enum class EYapMissingAudioErrorLevel : uint8;
enum class EYapErrorLevel : uint8;
enum class EYapMaturitySetting : uint8;

#define LOCTEXT_NAMESPACE "YapEditor"

enum class EYapFragmentControlsDirection : uint8
{
	Up,
	Down,
};

class SFlowGraphNode_YapFragmentWidget : public SCompoundWidget
{
	SLATE_DECLARE_WIDGET(SFlowGraphNode_YapFragmentWidget, SCompoundWidget)

	SLATE_BEGIN_ARGS(SFlowGraphNode_YapFragmentWidget)
	{
		
	}
		SLATE_END_ARGS()
	// ------------------------------------------
	// SETTINGS
	TMap<EYapTimeMode, FLinearColor> TimeModeButtonColors;
	
	// ------------------------------------------
	// STATE
protected:
	SFlowGraphNode_YapDialogueWidget* Owner = nullptr;

	TSharedPtr<SEditableTextBox> TitleTextBox;

	TSharedPtr<SWidget> DirectedAtWidget;
	
	bool bCursorContained = false;
	bool MoodTagSelectorMenuOpen = false;

	uint8 FragmentIndex = 0;

	bool bCtrlPressed = false;

	float Opacity = 0;

	double InitTime = -1;
	
	uint64 LastBitReplacementCacheFrame = 0;
	FYapBitReplacement* CachedBitReplacement = nullptr;

	bool NeedsChildSafeData() const;
	bool HasAnyChildSafeData() const;
	bool HasCompleteChildSafeData() const;
	bool HasCompleteChildSafeTextData() const;
	
	TSharedPtr<SOverlay> FragmentWidgetOverlay;
	TSharedPtr<SWidget> MoveFragmentControls = nullptr;
	TSharedRef<SWidget> CreateCentreTextDisplayWidget();

	TSharedPtr<SOverlay> FragmentOverlay;

	TSharedPtr<SButton> TitleTextEditButtonWidget;
	TSharedPtr<SButton> DialogueEditButtonWidget;
	TSharedPtr<SYapConditionsScrollBox> ConditionsScrollBox;

	TSharedPtr<SBox> SpeakerSelectionContainer;
	
	TSharedPtr<SWidget> ChildSafeCheckBox;

	bool bChildSafeCheckBoxHovered = false;

	EYapMaturitySetting GetDisplayMaturitySetting() const;

	static FSlateFontInfo DialogueTextFont;

	float CachedAudioTime = -1;
	TWeakObjectPtr<UObject> CachedAudioAssetPtr = nullptr;
	
public:
	TSharedPtr<SYapConditionsScrollBox> GetConditionsScrollBox() { return ConditionsScrollBox; }

	// ------------------------------------------
	// CONSTRUCTION
public:
	void Construct(const FArguments& InArgs, SFlowGraphNode_YapDialogueWidget* InOwner, uint8 InFragmentIndex); // non-virtual override
	
	// ------------------------------------------
	// WIDGETS
protected:
	int32 GetFragmentActivationCount() const;
	int32 GetFragmentActivationLimit() const;
	EVisibility Visibility_FragmentControlsWidget() const;
	EVisibility Visibility_FragmentShiftWidget(EYapFragmentControlsDirection YapFragmentControlsDirection) const;
	FReply OnClicked_FragmentShift(EYapFragmentControlsDirection YapFragmentControlsDirection);
	FReply OnClicked_FragmentDelete();
	TSharedRef<SWidget> CreateFragmentControlsWidget();
	bool Enabled_AudioPreviewButton() const;
	FReply OnClicked_AudioPreviewWidget(const TSoftObjectPtr<>* Object);
	TSharedRef<SWidget> CreateAudioPreviewWidget(const TSoftObjectPtr<UObject>* AudioAsset, TAttribute<EVisibility> VisibilityAtt);

	TSharedRef<SWidget> CreateFragmentHighlightWidget();
	void OnTextCommitted_FragmentActivationLimit(const FText& Text, ETextCommit::Type Arg);

	TSharedRef<SWidget> CreateUpperFragmentBar();
	EVisibility Visibility_FragmentTagWidget() const;
	
	ECheckBoxState		IsChecked_ChildSafeSettings() const;
	void				OnCheckStateChanged_MaturitySettings(ECheckBoxState CheckBoxState);
	FSlateColor			ColorAndOpacity_ChildSafeSettingsCheckBox() const;
	
	FSlateColor BorderBackgroundColor_DirectedAtImage() const;
	void OnAssetsDropped_DirectedAtWidget(const FDragDropEvent& DragDropEvent, TArrayView<FAssetData> AssetDatas);
	bool OnAreAssetsAcceptableForDrop_DirectedAtWidget(TArrayView<FAssetData> AssetDatas) const;
	FReply OnClicked_DirectedAtWidget();
	TSharedRef<SWidget> PopupContentGetter_DirectedAtWidget();
	const FSlateBrush* Image_DirectedAtWidget()const;
	TSharedRef<SWidget> CreateDirectedAtWidget();
	
	bool OnAreAssetsAcceptableForDrop_ChildSafeButton(TArrayView<FAssetData> AssetDatas) const;
	void OnAssetsDropped_ChildSafeButton(const FDragDropEvent& DragDropEvent, TArrayView<FAssetData> AssetDatas);
	// ------------------------------------------
	TSharedRef<SWidget> CreateFragmentWidget();

	EVisibility Visibility_DialogueEdit() const;
	EVisibility Visibility_EmptyTextIndicator(const FText* Text) const;
	TOptional<float> Value_TimeSetting_Default(EYapMaturitySetting MaturitySetting) const;
	TOptional<float> Value_TimeSetting_AudioTime(EYapMaturitySetting MaturitySetting) const;
	TOptional<float> Value_TimeSetting_TextTime(EYapMaturitySetting MaturitySetting) const;
	TOptional<float> Value_TimeSetting_ManualTime(EYapMaturitySetting MaturitySetting) const;

	TSharedRef<SWidget>	MakeTimeSettingRow(EYapTimeMode TimeMode, EYapMaturitySetting MaturitySetting);

	FSlateColor ButtonColor_TimeSettingButton() const;
	EVisibility Visibility_AudioSettingsButton() const;
	EVisibility Visibility_DialogueErrorState() const;
	// ------------------------------------------
	TSharedRef<SWidget>	CreateDialogueDisplayWidget();

	FVector2D			DialogueScrollBar_Thickness() const;
	FOptionalSize		Dialogue_MaxDesiredHeight() const;
	FText				Text_TextDisplayWidget(const FText* MatureText, const FText* SafeText) const;
	
	EVisibility			Visibility_DialogueBackground() const;
	FSlateColor			BorderBackgroundColor_Dialogue() const;

	TSharedRef<SWidget> PopupContentGetter_ExpandedEditor();

	TSharedRef<SWidget> BuildDialogueEditors_ExpandedEditor(float Width);
	TSharedRef<SWidget> BuildTimeSettings_ExpandedEditor(float Width);
	TSharedRef<SWidget> BuildPaddingSettings_ExpandedEditor(float Width);
	
	// ------------------------------------------

	FText				FragmentTagPreview_Text() const;
	// ---------------------------------------------------
	TOptional<float>	FragmentTimePadding_Percent() const;
	TOptional<float> FragmentTime_Percent() const;

	float				Value_FragmentTimePadding() const;
	void				OnValueChanged_FragmentTimePadding(float X);
	FSlateColor			FillColorAndOpacity_FragmentTimePadding() const;
	FText				ToolTipText_FragmentTimePadding() const;

	FLinearColor BorderBackgroundColor_CharacterImage() const;
	void OnSetNewSpeakerAsset(const FAssetData& AssetData);
	void OnSetNewDirectedAtAsset(const FAssetData& AssetData);
	
	TSharedRef<SWidget> PopupContentGetter_SpeakerWidget(TSoftObjectPtr<UYapCharacter>* CharacterAsset, const UYapCharacter* Character);

	FText Text_SpeakerWidget() const;
	FText ToolTipText_SpeakerWidget() const;
	
	bool OnAreAssetsAcceptableForDrop_SpeakerWidget(TArrayView<FAssetData> AssetDatas) const;
	void OnAssetsDropped_SpeakerWidget(const FDragDropEvent& DragDropEvent, TArrayView<FAssetData> AssetDatas);

	bool OnAreAssetsAcceptableForDrop_TextWidget(TArrayView<FAssetData> AssetDatas) const;
	void OnAssetsDropped_TextWidget(const FDragDropEvent& DragDropEvent, TArrayView<FAssetData> AssetDatas);
	
	// ------------------------------------------
	TSharedRef<SOverlay>	CreateSpeakerWidget();

	EVisibility			Visibility_PortraitImage() const;
	const FSlateBrush*	Image_SpeakerImage() const;
	EVisibility			Visibility_MissingPortraitWarning() const;
	EVisibility			Visibility_CharacterSelect() const;
	FString				ObjectPath_CharacterSelect() const;

	FText ToolTipText_MoodTagSelector() const;
	FSlateColor ForegroundColor_MoodTagSelectorWidget() const;
	// ------------------------------------------
	TSharedRef<SWidget>	CreateMoodTagSelectorWidget();

	EVisibility			Visibility_MoodTagSelector() const;
	void				OnMenuOpenChanged_MoodTagSelector(bool bMenuOpen);
	const FSlateBrush*	Image_MoodTagSelector() const;
	FGameplayTag		GetCurrentMoodTag() const;
	
	// ------------------------------------------
	TSharedRef<SWidget> CreateMoodTagMenuEntryWidget(FGameplayTag InIconName, bool bSelected = false, const FText& InLabel = FText::GetEmpty(), FName InTextStyle = TEXT("ButtonText"));

	FReply				OnClicked_MoodTagMenuEntry(FGameplayTag NewValue);

	FText Text_EditedText(FText* Text) const;
	void OnTextCommitted_EditedText(const FText& NewValue, ETextCommit::Type CommitType, void (FYapBit::*Func)(const FText& NewValue));

	FText ToolTipText_TextDisplayWidget(FText Label, const FText* MatureText, const FText* SafeText) const;
	FSlateColor ColorAndOpacity_TextDisplayWidget(FLinearColor BaseColor, const FText* MatureText, const FText* SafeText) const;

	EVisibility Visibility_TitleTextErrorState() const;
	// ------------------------------------------
	TSharedRef<SWidget> CreateTitleTextDisplayWidget();

	EVisibility			Visibility_TitleText() const;

	// ------------------------------------------
	TSharedRef<SWidget>	CreateFragmentTagWidget();
	
	FGameplayTag		Value_FragmentTag() const;
	void				OnTagChanged_FragmentTag(FGameplayTag GameplayTag);

	// ------------------------------------------

	FReply				OnClicked_SetTimeModeButton(EYapTimeMode TimeMode);

	void				OnValueUpdated_ManualTime(float NewValue);
	void				OnValueCommitted_ManualTime(float NewValue, ETextCommit::Type CommitType);
	FSlateColor			ButtonColorAndOpacity_UseTimeMode(EYapTimeMode TimeMode, FLinearColor ColorTint) const;
	FSlateColor			ButtonColorAndOpacity_PaddingButton() const;
	FSlateColor			ForegroundColor_TimeSettingButton(EYapTimeMode TimeMode, FLinearColor ColorTint) const;

	bool OnShouldFilterAsset_AudioAssetWidget(const FAssetData& AssetData) const;
	// ------------------------------------------
	TSharedRef<SWidget> CreateAudioAssetWidget(const TSoftObjectPtr<UObject>& Asset);

	FText				ObjectPathText_AudioAsset() const;
	FString				ObjectPath_AudioAsset() const;
	EVisibility			Visibility_AudioAssetErrorState(const TSoftObjectPtr<UObject>* Asset) const;

	FSlateColor			ColorAndOpacity_AudioSettingsButton() const;
	EYapErrorLevel		GetFragmentAudioErrorLevel() const;

	FSlateColor			ColorAndOpacity_AudioAssetErrorState(const TSoftObjectPtr<UObject>* Asset) const;
	EYapErrorLevel		GetAudioAssetErrorLevel(const TSoftObjectPtr<UObject>& Asset) const;
	
	// ------------------------------------------
	// HELPERS
protected:
	const UFlowNode_YapDialogue* GetDialogueNode() const;

	UFlowNode_YapDialogue* GetDialogueNode();

	const FYapFragment& GetFragment() const;
	
	FYapFragment& GetFragment();

	FYapFragment& GetFragmentMutable() const;

	const FYapBit& GetBit() const;
	
	FYapBit& GetBit();

	const FYapBit& GetBitConst();

	FYapBit& GetBitMutable() const;

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

#undef LOCTEXT_NAMESPACE