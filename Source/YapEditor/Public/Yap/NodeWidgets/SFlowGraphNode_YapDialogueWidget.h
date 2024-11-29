#pragma once
#include "SFlowGraphNode_YapFragmentWidget.h"
#include "Graph/Widgets/SFlowGraphNode.h"
#include "Yap/Nodes/FlowNode_YapDialogue.h"

class SCanvas;
class SConditionEntryWidget;
class SConditionDetailsViewWidget;
class SVirtualWindow;
struct FYapFragment;
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
	
	TSharedPtr<SBox> BypassOutputBox;

	// TODO make style set junk, for both this and Fragment widget. Pull it out of
	// TODO search engine source for final : public FSlateStyleSet for examples
	FTextBlockStyle NormalText;

	FLinearColor DialogueButtonsColor;
	
	FLinearColor ConnectedBypassPinColor;
	FLinearColor DisconnectedBypassPinColor;

	FLinearColor ConnectedFragmentPinColor;
	FLinearColor DisconnectedFragmentPinColor;
	
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

	TSharedPtr<SBox> FragmentSequencingButton_Box;
	TSharedPtr<SButton> FragmentSequencingButton_Button;
	TSharedPtr<SImage> FragmentSequencingButton_Image;
	TSharedPtr<STextBlock> FragmentSequencingButton_Text;
	
	TArray<TSharedPtr<SFlowGraphNode_YapFragmentWidget>> FragmentWidgets;
	
	// ------------------------------------------
	// CONSTRUCTION
public:
	void Construct(const FArguments& InArgs, UFlowGraphNode* InNode);

	void RequestUpdateGraphNode() { UpdateGraphNode(); };
	
	// ------------------------------------------
	// WIDGETS
protected:
	int32 GetDialogueActivationCount() const;

	int32 GetDialogueActivationLimit() const;
	EVisibility Visibility_InterruptibleToggleIconOff() const;
	void OnTextCommitted_DialogueActivationLimit(const FText& Text, ETextCommit::Type Arg);
	FGameplayTag Value_DialogueTag() const;
	void OnTagChanged_DialogueTag(FGameplayTag GameplayTag);
	int32 GetMaxNodeWidth() const;
	// ------------------------------------------
	TSharedRef<SWidget> CreateTitleWidget(TSharedPtr<SNodeTitle> NodeTitle) override;
	
	ECheckBoxState		IsChecked_InterruptibleToggle() const;
	void				OnCheckStateChanged_InterruptibleToggle(ECheckBoxState CheckBoxState);
	FSlateColor			ColorAndOpacity_InterruptibleToggleIcon() const;

	// ------------------------------------------
	TSharedRef<SWidget>	CreateNodeContentArea() override;
	
	// ------------------------------------------

	FSlateColor ColorAndOpacity_NodeHeaderButton() const;
	FSlateColor ColorAndOpacity_NodeHeader() const;
	FText Text_FragmentSequencingButton() const;
	FReply OnClicked_TogglePlayerPrompt();

	TSharedRef<SWidget> CreateContentHeader();

	EVisibility			Visibility_FragmentSequencingButton() const;
	FReply				OnClicked_FragmentSequencingButton();
	const FSlateBrush*	Image_FragmentSequencingButton() const;
	FText				ToolTipText_FragmentSequencingButton() const;
	FSlateColor			ColorAndOpacity_FragmentSequencingButton() const;
	
	FText				Text_NodeHeader() const;
	EVisibility			FragmentRowHighlight_Visibility(uint8 f) const;
	FSlateColor			FragmentRowHighlight_BorderBackgroundColor(uint8 f) const;

	// ------------------------------------------
	TSharedRef<SWidget> CreateFragmentBoxes();

	TSharedRef<SWidget>	CreateFragmentSeparatorWidget(uint8 FragmentIndex) const;

	EVisibility			FragmentSeparator_Visibility() const;
	FSlateColor			ColorAndOpacity_FragmentSeparator() const;
	FReply				FragmentSeparator_OnClicked(uint8 Index);

	// ------------------------------------------
	TSharedRef<SWidget>	CreateFragmentRowWidget(uint8 FragmentIndex);
	
	TSharedRef<SBox>	CreateLeftFragmentPane(uint8 FragmentIndex);

	// ------------------------------------------
	TSharedRef<SBox>	CreateLeftSideNodeBox();

	// ------------------------------------------
	TSharedRef<SHorizontalBox> CreateContentFooter();

	EVisibility			Visibility_BottomAddFragmentButton() const;
	FReply				OnClicked_BottomAddFragmentButton();

	// UNSORTED

	TSharedRef<SWidget> CreateDialogueTagPreviewWidget() const;
	FText Text_DialogueTagPreview() const;
	EVisibility Visibility_DialogueTagPreview() const;

	EVisibility Visibility_ConditionWidgets() const;
	void OnClick_ConditionEntryButton(UYapCondition* Condition, TSharedRef<SConditionEntryWidget> ConditionEntryWidget, int32 ConditionIndexInArray);
	bool IsEnabled_ConditionWidgetsScrollBox() const;
	TSharedRef<SWidget> CreateConditionWidgets();
	TSharedRef<SWidget> CreateConditionWidget(const UYapCondition* Condition);
	
	// ------------------------------------------
	// PUBLIC API & THEIR HELPERS
public:
	bool GetIsSelected() const { return bIsSelected; };

	void SetSelected();

	bool GetShiftHooked() const { return bShiftHooked; };
	
	void SetFocusedFragmentIndex(uint8 FragmentIndex);

	void ClearFocusedFragmentIndex(uint8 FragmentIndex);
	
	const bool GetFocusedFragmentIndex(uint8& OutFragmentIndex) const;

	void SetTypingFocus();

	void ClearTypingFocus();
	
	UFlowNode_YapDialogue* GetFlowYapDialogueNodeMutable();
	
	const UFlowNode_YapDialogue* GetFlowYapDialogueNode() const;
	
	TOptional<uint8> GetFocusedFragmentIndex() { return FocusedFragmentIndex; };

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

public:
	void CreatePinWidgets() override;

	const FYapFragment& GetFragment(uint8 FragmentIndex) const;

	FYapFragment& GetFragmentMutable(uint8 FragmentIndex);

	void CreateStandardPinWidget(UEdGraphPin* Pin) override;

	TArray<FOverlayWidgetInfo> GetOverlayWidgets(bool bSelected, const FVector2D& WidgetSize) const override;

	TSharedPtr<SConditionDetailsViewWidget> ConditionDetailsPane;
	TWeakObjectPtr<UYapCondition> EditedCondition;
	
	FVector2D ConditionDetailsPaneOffset;

	virtual TSharedPtr<IToolTip> GetToolTip() override { return nullptr; };
};
