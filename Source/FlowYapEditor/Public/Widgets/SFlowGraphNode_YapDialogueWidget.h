#pragma once
#include "SFlowGraphNode_YapFragmentWidget.h"
#include "Graph/Widgets/SFlowGraphNode.h"

struct FFlowYapFragment;
class SFlowGraphNode_YapFragmentWidget;
class UFlowNode_YapDialogue;
class UFlowGraphNode_YapDialogue;

class SFlowGraphNode_YapDialogueWidget : public SFlowGraphNode
{
	// ------------------------------------------
	// STATE
protected:
	UFlowGraphNode_YapDialogue* FlowGraphNode_YapDialogue = nullptr;

	TSharedPtr<SVerticalBox> FragmentBox;
	TArray<TSharedPtr<SFlowGraphNode_YapFragmentWidget>> FragmentWidgets;
	TArray<TSharedPtr<SVerticalBox>> FragmentInputBoxes;
	TArray<TSharedPtr<SVerticalBox>> FragmentOutputBoxes;
	TSharedPtr<SBox> BypassOutputBox;

	// TODO make style set junk, for both this and Fragment widget. Pull it out of
	// TODO search engine source for final : public FSlateStyleSet for examples
	FTextBlockStyle NormalText;

	FLinearColor DialogueButtonsColor;

	FLinearColor ConnectedEndPinColor;
	FLinearColor DisconnectedEndPinColor;

	FLinearColor ConnectedStartPinColor;
	FLinearColor DisconnectedStartPinColor;

	FLinearColor ConnectedInterruptPinColor;	
	FLinearColor DisconnectedInterruptPinColor;	
	FLinearColor ConnectedInterruptPinColor_Disabled;	
	FLinearColor DisconnectedInterruptPinColor_Disabled;	
	
	FLinearColor ConnectedBypassPinColor;
	FLinearColor DisconnectedBypassPinColor;

	// TODO move to a proper style
	FCheckBoxStyle InterruptibleCheckBoxStyle;

	TOptional<uint8> FocusedFragmentIndex;

	bool bIsSelected = false;
	bool bShiftHooked = false;
	bool bKeyboardFocused = false;

	TOptional<uint8> FlashFragmentIndex; 
	double FlashHighlight = 0.0;
	
	// ------------------------------------------
	// CONSTRUCTION
public:
	void Construct(const FArguments& InArgs, UFlowGraphNode* InNode);

	// ------------------------------------------
	// WIDGETS
protected:
	// ------------------------------------------
	TSharedRef<SWidget> CreateTitleWidget(TSharedPtr<SNodeTitle> NodeTitle) override;
	
	ECheckBoxState		PlayerPromptCheckBox_IsChecked() const;
	void				PlayerPromptCheckBox_OnCheckStateChanged(ECheckBoxState CheckBoxState);
	ECheckBoxState		InterruptibleToggle_IsChecked() const;
	void				InterruptibleToggle_OnCheckStateChanged(ECheckBoxState CheckBoxState);
	FSlateColor			InterruptibleToggleIcon_ColorAndOpacity() const;

	// ------------------------------------------
	TSharedRef<SWidget>	CreateNodeContentArea() override;

	EVisibility			FragmentRowHighlight_Visibility(uint8 f) const;
	FSlateColor			FragmentRowHighlight_BorderBackgroundColor(uint8 f) const;

	// ------------------------------------------
	TSharedRef<SWidget>	CreateFragmentSeparatorWidget(int FragmentIndex);

	FSlateColor			FragmentSeparator_ColorAndOpacity() const;
	FReply				FragmentSeparator_OnClicked(int Index);

	// ------------------------------------------
	TSharedRef<SWidget>	CreateFragmentRowWidget(FFlowYapFragment& Fragment);
	
	// ------------------------------------------
	TSharedRef<SBox>	CreateLeftFragmentPane(FFlowYapFragment& Fragment);
	
	// ------------------------------------------
	TSharedRef<SBox>	CreateLeftSideNodeBox();
	
	// ------------------------------------------
	TSharedRef<SBox>	CreateActivationLimiterWidget(FFlowYapFragment& Fragment);
	
	EVisibility			ActivationLimiter_Visibility(SFlowGraphNode_YapDialogueWidget* FlowGraphNode_YapDialogueWidget, FFlowYapFragment* FlowYapFragment) const;
	FSlateColor			ActivationDot_ColorAndOpacity(FFlowYapFragment* Fragment, int32 ActivationIndex) const;
	FReply				ActivationDot_OnClicked(FFlowYapFragment* Fragment, int ActivationIndex);

	// ------------------------------------------
	TSharedRef<SBox>	CreateRightFragmentPane();
	
	// ------------------------------------------
	TSharedRef<SHorizontalBox> CreateBottomBarWidget();

	EVisibility			FragmentSequencingButton_Visibility() const;
	FReply				FragmentSequencingButton_OnClicked();
	const FSlateBrush*	FragmentSequencingButton_Image() const;
	FText				FragmentSequencingButton_ToolTipText() const;
	FSlateColor			FragmentSequencingButton_ColorAndOpacity() const;
	EVisibility			BottomAddFragmentButton_Visibility() const;
	FReply				BottomAddFragmentButton_OnClicked();

	// ------------------------------------------
	// PUBLIC API & THEIR HELPERS
public:
	void DeleteFragment(uint8 FragmentIndex);
	
	void MoveFragmentUp(uint8 FragmentIndex);

	void MoveFragmentDown(uint8 FragmentIndex);

protected:
	void MoveFragment(uint8 FragmentIndex, int16 By);

public:
	bool GetIsSelected() const { return bIsSelected; };

	bool GetShiftHooked() const { return bShiftHooked; };
	
	void SetFocusedFragment(uint8 FragmentIndex);

	void ClearFocusedFragment(uint8 FragmentIndex);
	
	const TSharedPtr<SFlowGraphNode_YapFragmentWidget> GetFocusedFragment() const;

	void SetTypingFocus();

	void ClearTypingFocus();
	
	UFlowNode_YapDialogue* GetFlowYapDialogueNode();
	
	const UFlowNode_YapDialogue* GetFlowYapDialogueNode() const;
	
	TOptional<uint8> GetFocusedFragmentIndex() { return FocusedFragmentIndex; };

protected:
	void SetFlashFragment(uint8 FragmentIndex);
	
	// ------------------------------------------
	// OVERRIDES & THEIR HELPERS
public:
	void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

	const FSlateBrush* GetShadowBrush(bool bSelected) const override;
	
	void AddPin(const TSharedRef<SGraphPin>& PinToAdd) override;

protected:
	void AddBypassPin(const TSharedRef<SGraphPin>& PinToAdd);
};
