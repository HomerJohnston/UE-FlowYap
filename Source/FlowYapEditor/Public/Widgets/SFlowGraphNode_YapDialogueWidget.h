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

	FCheckBoxStyle InterruptibleCheckBoxStyle;

	TOptional<uint8> FocusedFragmentIndex;

	bool bIsSelected = false;
	bool bShiftHooked = false;
	bool bKeyboardFocused = false;

	// ------------------------------------------
	// CONSTRUCTION
public:
	void Construct(const FArguments& InArgs, UFlowGraphNode* InNode);
	
	UFlowNode_YapDialogue* GetFlowYapDialogueNode();
	
	const UFlowNode_YapDialogue* GetFlowYapDialogueNode() const;
	
	TOptional<uint8> GetFocusedFragmentIndex() { return FocusedFragmentIndex; };

protected:

	FSlateColor GetFragmentSeparatorColor() const;

	TOptional<int32> GetActivationLimit(FFlowYapFragment* Fragment) const;

	void OnActivationLimitChanged(int32 NewValue, FFlowYapFragment* Fragment);

	FSlateColor ActivationDot_ColorAndOpacity(FFlowYapFragment* Fragment, int32 ActivationIndex) const;

	FReply ActivationDot_OnClicked(FFlowYapFragment* Fragment, int ActivationIndex);

	EVisibility GetActivationIndicatorVisibility(SFlowGraphNode_YapDialogueWidget* FlowGraphNode_YapDialogueWidget, FFlowYapFragment* FlowYapFragment) const;

	EVisibility FragmentSequencingButton_Visibility() const;

	FReply FragmentSequencingButton_OnClicked();

	const FSlateBrush* FragmentSequencingButton_Image() const;

	FText FragmentSequencingButton_ToolTipText() const;

	FSlateColor FragmentSequencingButton_ColorAndOpacity() const;

	FReply InsertFragment(int Index);

	TSharedRef<SWidget> CreateNodeContentArea() override;

	ECheckBoxState PlayerPromptCheckBox_IsChecked() const;

	void PlayerPromptCheckBox_OnCheckStateChanged(ECheckBoxState CheckBoxState);

	ECheckBoxState InterruptibleToggle_IsChecked() const;

	void InterruptibleToggle_OnCheckStateChanged(ECheckBoxState CheckBoxState);
	
	TSharedRef<SWidget> CreateTitleWidget(TSharedPtr<SNodeTitle> NodeTitle) override;
	
	EVisibility BottomAddFragmentButton_Visibility() const;
	
	void AddPin(const TSharedRef<SGraphPin>& PinToAdd) override;

	void AddBypassPin(const TSharedRef<SGraphPin>& PinToAdd);
	
	FReply BottomAddFragmentButton_OnClicked();

public:
	void DeleteFragment(uint8 FragmentIndex);
	
	void MoveFragmentUp(uint8 FragmentIndex);

	void MoveFragmentDown(uint8 FragmentIndex);

	void MoveFragment(uint8 FragmentIndex, int16 By);
	
	bool GetNormalisedMousePositionInGeometry(UObject *WorldContextObject, FGeometry Geometry, FVector2D &Position) const;
	
	const FSlateBrush* GetShadowBrush(bool bSelected) const override;

	void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

	bool GetIsSelected() const;

	bool GetControlHooked() const;
	
	void SetFocusedFragment(uint8 FragmentIndex);

	void ClearFocusedFragment(uint8 FragmentIndex);
	
	const TSharedPtr<SFlowGraphNode_YapFragmentWidget> GetFocusedFragment() const;

	void SetTypingFocus(uint8 FragmentIndex);

	void ClearTypingFocus(uint8 FragmentIndex);
};
