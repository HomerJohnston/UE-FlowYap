#pragma once

#include "IPropertyTypeCustomization.h"

class SMultiLineEditableTextBox;
class UFlowNode_YapDialogue;
struct FFlowYapFragment;
struct FGameplayTag;
enum class EFlowYapErrorLevel : uint8;

class FPropertyCustomization_FlowYapFragment : public IPropertyTypeCustomization
{
	TSharedPtr<IPropertyHandle> FragmentHandle;

	TWeakObjectPtr<UFlowNode_YapDialogue> Dialogue;
	
	TSharedPtr<SMultiLineEditableTextBox> DialogueBox;
	
	TSharedPtr<SEditableTextBox> TitleTextBox;

	bool bCtrlPressed = false;
	
	bool MoodKeySelectorMenuOpen = false;

	TSharedPtr<IPropertyHandle> BitHandle;

	TSharedPtr<IPropertyHandle> Bit_CharacterHandle;

	TSharedPtr<IPropertyHandle> Bit_TitleTextHandle;

	TSharedPtr<IPropertyHandle> Bit_DialogueTextHandle;

	TSharedPtr<IPropertyHandle> Bit_DialogueAudioAssetHandle;

	TSharedPtr<IPropertyHandle> Bit_MoodKeyHandle;

	TSharedPtr<IPropertyHandle> Bit_bUseProjectDefaultTimeSettingsHandle;

	TSharedPtr<IPropertyHandle> Bit_TimeModeHandle;

	TSharedPtr<IPropertyHandle> Bit_InterruptibleHandle;

	TSharedPtr<IPropertyHandle> Bit_ManualTimeHandle;

	TSharedPtr<IPropertyHandle> Bit_CachedWordCountHandle;

	TSharedPtr<IPropertyHandle> Bit_CachedAudioTimeHandle;
	
	TSharedPtr<IPropertyHandle> ActivationLimitHandle;

	TSharedPtr<IPropertyHandle> FragmentTagHandle;

	TSharedPtr<IPropertyHandle> PaddingToNextFragmentHandle;

	TSharedPtr<IPropertyHandle> CommonPaddingSettingHandle;

	
public:
	static  TSharedRef<IPropertyTypeCustomization> MakeInstance();

	void CustomizeHeader(TSharedRef<IPropertyHandle> InPropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils) override;

	void Init(TSharedRef<IPropertyHandle> InPropertyHandle);
	
	void CustomizeChildren(TSharedRef<IPropertyHandle> InPropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils) override;

	// ------------------------------------------
	// WIDGETS
protected:
	// ------------------------------------------
	FOptionalSize		Fragment_WidthOverride() const;
	EVisibility			FragmentBottomSection_Visibility() const;

	// ------------------------------------------
	TSharedRef<SBox>	CreateDialogueWidget();

	FVector2D			DialogueScrollBar_Thickness() const;
	FOptionalSize		Dialogue_MaxDesiredHeight() const;
	FText				Dialogue_Text() const;
	void				Dialogue_OnTextCommitted(const FText& CommittedText, ETextCommit::Type CommitType);
	FText				Dialogue_ToolTipText() const;
	FSlateColor			Dialogue_BackgroundColor() const;
	FSlateColor			Dialogue_ForegroundColor() const;

	EVisibility			DialogueBackground_Visibility() const;
	FSlateColor			Dialogue_BorderBackgroundColor() const;
	
	// ---------------------------------------------------
	TSharedRef<SBox>	CreateActivationLimiterWidget();

	EVisibility			ActivationLimiter_Visibility() const;
	FText				ActivationLimiter_Text() const;	
	EVisibility			ActivationDot_Visibility() const;
	FSlateColor			ActivationDot_ColorAndOpacity() const;
	FReply				ActivationDot_OnClicked();
	
	// ------------------------------------------
	TSharedRef<SWidget>	CreateFragmentTagPreviewWidget();

	EVisibility			FragmentTagPreview_Visibility() const;
	FText				FragmentTagPreview_Text() const;
	FSlateColor			FragmentTagPreview_BorderBackgroundColor() const;
	FLinearColor		FragmentTagPreview_ColorAndOpacity() const;

	// ---------------------------------------------------
	TSharedRef<SWidget>	CreateFragmentTimePaddingWidget();
	
	TOptional<float>	FragmentTimePadding_Percent() const;
	float				FragmentTimePadding_Value() const;
	EVisibility			FragmentTimePaddingSlider_Visibility() const;
	void				FragmentTimePadding_OnValueChanged(float X);
	FSlateColor			FragmentTimePadding_FillColorAndOpacity() const;
	FText				FragmentTimePadding_ToolTipText() const;
	
	// ------------------------------------------
	TSharedRef<SBox>	CreatePortraitWidget();

	EVisibility			PortraitImage_Visibility() const;
	const FSlateBrush*	PortraitImage_Image() const;
	EVisibility			MissingPortraitWarning_Visibility() const;
	EVisibility			CharacterSelect_Visibility() const;
	FString				CharacterSelect_ObjectPath() const;
	void				CharacterSelect_OnObjectChanged(const FAssetData& InAssetData);
	// ------------------------------------------
	TSharedRef<SBox>	CreateMoodKeySelectorWidget();

	EVisibility			MoodKeySelector_Visibility() const;
	void				MoodKeySelector_OnMenuOpenChanged(bool bMenuOpen);
	const FSlateBrush*	MoodKeyBrush_GetBrush() const;
	FGameplayTag		GetCurrentMoodKey() const;

	// ------------------------------------------
	TSharedRef<SWidget> CreateMoodKeyMenuEntryWidget(FGameplayTag MoodKey, bool bSelected = false, const FText& InLabel = FText::GetEmpty(), FName InTextStyle = TEXT("ButtonText"));

	FReply				MoodKeyMenuEntry_OnClicked(FGameplayTag NewValue);

	// ------------------------------------------
	TSharedRef<SWidget> CreateTitleTextWidget();

	EVisibility			TitleText_Visibility() const;
	FText				TitleText_Text() const;
	void				TitleText_OnTextCommitted(const FText& CommittedText, ETextCommit::Type CommitType);

	// ------------------------------------------
	TSharedRef<SWidget>	CreateFragmentTagWidget();
	
	FGameplayTag		FragmentTag_Tag() const;
	void				FragmentTag_OnTagChanged(FGameplayTag GameplayTag);

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

	// ------------------------------------------
	// HELPERS
protected:
	FFlowYapFragment*	GetFragment() const;

	bool IsHovered() const { return false; };
	
	bool InErrorState() const;
	
	// ------------------------------------------
	// OVERRIDES
public:
	FSlateColor GetNodeTitleColor() const; // non-virtual override

	
};
