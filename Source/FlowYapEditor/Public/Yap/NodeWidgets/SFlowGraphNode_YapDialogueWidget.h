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

	TSharedPtr<SBox> DialogueInputBoxArea;
	TSharedPtr<SBox> DialogueOutputBoxArea;
	
	TArray<TSharedPtr<SVerticalBox>> FragmentInputBoxes;
	TArray<TSharedPtr<SOverlay>> FragmentOutputBoxes;
	
	TSharedPtr<SBox> BypassOutputBox;

	// TODO make style set junk, for both this and Fragment widget. Pull it out of
	// TODO search engine source for final : public FSlateStyleSet for examples
	FTextBlockStyle NormalText;

	FLinearColor DialogueButtonsColor;

	FLinearColor ConnectedEndPinColor;
	FLinearColor DisconnectedEndPinColor;
	FLinearColor DisconnectedEndPinColor_Prompt;
	
	FLinearColor ConnectedBypassPinColor;
	FLinearColor DisconnectedBypassPinColor;

	FLinearColor ConnectedConditionPinColor;
	FLinearColor DisconnectedConditionPinColor;
	
	// TODO move to a proper style
	FCheckBoxStyle InterruptibleCheckBoxStyle;

	static FButtonStyle MoveFragmentButtonStyle;
	static bool bStylesInitialized;
	
	TOptional<uint8> FocusedFragmentIndex;

	bool bIsSelected = false;
	bool bShiftHooked = false;
	bool bKeyboardFocused = false;
	bool bShiftPressed = false;
	bool bCtrlPressed = false;

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

	TSharedRef<SWidget> CreateContentHeader();
	TSharedRef<SWidget> CreateFragmentBoxes();

	FText				NodeHeader_Text() const;
	EVisibility			FragmentRowHighlight_Visibility(uint8 f) const;
	FSlateColor			FragmentRowHighlight_BorderBackgroundColor(uint8 f) const;

	// ------------------------------------------
	TSharedRef<SWidget>	CreateFragmentSeparatorWidget(uint8 FragmentIndex) const;

	EVisibility			FragmentSeparator_Visibility() const;
	FSlateColor			FragmentSeparator_ColorAndOpacity() const;
	FReply				FragmentSeparator_OnClicked(uint8 Index);

	// ------------------------------------------
	TSharedRef<SWidget>	CreateFragmentRowWidget(uint8 FragmentIndex);
	
	// ------------------------------------------
	TSharedRef<SBox>	CreateLeftFragmentPane(uint8 FragmentIndex);
	
	// ------------------------------------------
	TSharedRef<SBox>	CreateFragmentControlsWidget(uint8 FragmentIndex);

	EVisibility			FragmentControls_Visibility(uint8 FragmentIndex) const;
	EVisibility			MoveFragmentUpButton_Visibility(uint8 FragmentIndex) const;
	FReply				MoveFragmentUpButton_OnClicked(uint8 FragmentIndex);
	EVisibility			DeleteFragmentButton_Visibility(uint8 FragmentIndex) const;
	FReply				DeleteFragmentButton_OnClicked(uint8 FragmentIndex);
	EVisibility			MoveFragmentDownButton_Visibility(uint8 FragmentIndex) const;
	FReply				MoveFragmentDownButton_OnClicked(uint8 FragmentIndex);
	
	// ------------------------------------------
	TSharedRef<SBox>	CreateLeftSideNodeBox();
	
	// ------------------------------------------
	TSharedRef<SBox>	CreateRightFragmentPane();
	
	// ------------------------------------------
	TSharedRef<SHorizontalBox> CreateContentFooter();

	EVisibility			FragmentSequencingButton_Visibility() const;
	FReply				FragmentSequencingButton_OnClicked();
	const FSlateBrush*	FragmentSequencingButton_Image() const;
	FText				FragmentSequencingButton_ToolTipText() const;
	FSlateColor			FragmentSequencingButton_ColorAndOpacity() const;
	EVisibility			BottomAddFragmentButton_Visibility() const;
	FReply				BottomAddFragmentButton_OnClicked();

	// UNSORTED

	TSharedRef<SWidget> CreateDialogueTagPreviewWidget() const;
	FText DialogueTagPreview_Text() const;
	EVisibility DialogueTagPreview_Visibility() const;

	EVisibility ConditionWidgets_Visibility() const;
	TSharedRef<SWidget> CreateConditionWidgets();
	TSharedRef<SWidget> CreateConditionWidget(const UFlowYapCondition* Condition);
	
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
	
	void SetFocusedFragmentIndex(uint8 FragmentIndex);

	void ClearFocusedFragmentIndex(uint8 FragmentIndex);
	
	const bool GetFocusedFragmentIndex(uint8& OutFragmentIndex) const;

	void SetTypingFocus();

	void ClearTypingFocus();
	
	UFlowNode_YapDialogue* GetFlowYapDialogueNodeMutable();
	
	const UFlowNode_YapDialogue* GetFlowYapDialogueNode() const;
	
	TOptional<uint8> GetFocusedFragmentIndex() { return FocusedFragmentIndex; };

	void ForceUpdateGraphNode() { UpdateGraphNode(); };
	
protected:
	void SetFlashFragment(uint8 FragmentIndex);
	
	void OnDialogueEnd(uint8 FragmentIndex);
	void OnDialogueStart(uint8 FragmentIndex);
	void OnDialogueInterrupt(uint8 FragmentIndex);
	
	// ------------------------------------------
	// OVERRIDES & THEIR HELPERS
public:
	void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

	const FSlateBrush* GetShadowBrush(bool bSelected) const override;

	void AddPin(const TSharedRef<SGraphPin>& PinToAdd) override;
	
protected:
	void AddInPin(const TSharedRef<SGraphPin> PinToAdd);

	void AddOutPin(const TSharedRef<SGraphPin>& PinToAdd);

	void AddBypassPin(const TSharedRef<SGraphPin>& PinToAdd);

	void AddFragmentPin(const TSharedRef<SGraphPin>& PinToAdd, int32 FragmentIndex);

public:
	void CreateStandardPinWidget(UEdGraphPin* Pin) override;

	const FFlowYapFragment* GetFragment(uint8 FragmentIndex) const;

	FFlowYapFragment* GetFragmentMutable(uint8 FragmentIndex);
};
