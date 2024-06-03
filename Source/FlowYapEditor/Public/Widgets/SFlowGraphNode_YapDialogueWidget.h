#pragma once
#include "Graph/Widgets/SFlowGraphNode.h"
#include "Widgets/SUserWidget.h"

struct FFlowYapFragment;
class SFlowGraphNode_YapFragmentWidget;
class UFlowNode_YapDialogue;
class UFlowGraphNode_YapDialogue;

class SFlowGraphNode_YapDialogueWidget : public SFlowGraphNode
{
protected:
	// TODO: Is this safe?
	UFlowGraphNode_YapDialogue* FlowGraphNode_YapDialogue = nullptr;

	TSharedPtr<SVerticalBox> FragmentBox;
	
	TArray<TSharedPtr<SFlowGraphNode_YapFragmentWidget>> FragmentWidgets;

	TArray<TSharedPtr<SVerticalBox>> FragmentInputBoxes;
	
	TArray<TSharedPtr<SVerticalBox>> FragmentOutputBoxes;

	TSharedPtr<SBox> BypassOutputBox;

	FTextBlockStyle NormalText;

	FLinearColor DialogueButtonsColor;

	FLinearColor ConnectedPinColor;
	
	FLinearColor DisconnectedPinColor;

	FLinearColor ConnectedBypassPinColor;
	
	FLinearColor DisconnectedBypassPinColor;

	// STATE
	bool bIsSelected = false;

	bool bWasSelected = false;

	bool bControlHooked = false;

	const SFlowGraphNode_YapFragmentWidget* FocusedFragment = nullptr;
	
public:
	void Construct(const FArguments& InArgs, UFlowGraphNode* InNode);

	UFlowNode_YapDialogue* GetFlowYapDialogueNode();
	
	const UFlowNode_YapDialogue* GetFlowYapDialogueNode() const;

protected:
	EVisibility GetFragmentMovementVisibility() const;

	FReply MoveFragment(bool bUp, int64 Index);

	FSlateColor GetFragmentSeparatorColor() const;

	TOptional<int32> GetActivationLimit(FFlowYapFragment* Fragment) const;

	void OnActivationLimitChanged(int32 NewValue, FFlowYapFragment* Fragment);

	FSlateColor GetActivationDotColor(FFlowYapFragment* Fragment, int32 ActivationIndex) const;

	FReply OnClickedActivationDot(FFlowYapFragment* Fragment, int ActivationIndex);

	EVisibility GetActivationIndicatorVisibility(SFlowGraphNode_YapDialogueWidget* FlowGraphNode_YapDialogueWidget, FFlowYapFragment* FlowYapFragment) const;
	
	TSharedRef<SWidget> CreateNodeContentArea() override;

	ECheckBoxState GetIsUserPromptDialogue() const;

	void HandleUserPromptDialogueChanged(ECheckBoxState CheckBoxState);
	
	TSharedRef<SWidget> CreateTitleWidget(TSharedPtr<SNodeTitle> NodeTitle) override;
	
	EVisibility GetDialogueFragmentButtonsVisibility() const;
	
	void AddPin(const TSharedRef<SGraphPin>& PinToAdd) override;

	void AddBypassPin(const TSharedRef<SGraphPin>& PinToAdd);
	
	FReply AddFragment();

public:
	FReply DeleteFragment(int64 FragmentID);
	
	bool GetNormalisedMousePositionInGeometry(UObject *WorldContextObject, FGeometry Geometry, FVector2D &Position) const;
	
	const FSlateBrush* GetShadowBrush(bool bSelected) const override;

	void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

	bool GetIsSelected() const;

	bool GetControlHooked() const;
	
	void SetFocusedFragment(const SFlowGraphNode_YapFragmentWidget* InFragment);

	void ClearFocusedFragment(const SFlowGraphNode_YapFragmentWidget* InFragment);
	
	const SFlowGraphNode_YapFragmentWidget* GetFocusedFragment() const;
};
