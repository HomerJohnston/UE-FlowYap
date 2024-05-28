#pragma once

#include "CoreMinimal.h"
#include "GraphNodes/FlowGraphNode_YapSpeechBase.h"
#include "Graph/Widgets/SFlowGraphNode.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Input/SSegmentedControl.h"

class UFlowNode_YapSpeechBase;
class UFlowGraphNode_YapDialogueBase;

class SMultiLineEditableTextBox;
/**
 *
 */
class SFlowGraphNode_YapSpeechBaseWidget : public SFlowGraphNode
{
protected:
	// TODO: why doesn't this redraw when I disable the node? might have to ask on discord
	void UpdateGraphNode() override;
	
	virtual TSharedRef<SWidget> CreateDialogueContentArea();

	TSharedRef<SBox> GetPortraitWidget();
	
	TSharedRef<SBox> GetPortraitKeySelectorWidget();
	
	TSharedRef<SWidget> CreatePortraitKeyButton(FName InIconName = FName(), bool bSelected = false, const FText& InLabel = FText::GetEmpty(), FName InTextStyle = TEXT("ButtonText"));

	virtual TSharedRef<SBox> GetAdditionalOptionsWidget() = 0;

	FVector2D CalculateOutputPinVerticalOffset() const;

	virtual void AddPinButton(TSharedPtr<SVerticalBox> OutputBox, TSharedRef<SWidget> ButtonContent, const EEdGraphPinDirection Direction, FString DocumentationExcerpt = FString(), TSharedPtr<SToolTip> CustomTooltip = nullptr);

	FOptionalSize GetMaxDialogueWidgetHeight() const;
	
	FOptionalSize GetMaxDialogueWidgetWidth() const;
	
	TSharedRef<SWidget> CreateNodeContentArea() override;
	
public:
	FText GetTitleText() const;

	FText GetDialogueText() const;

	FString GetSelectedDialogueAudioAssetPath() const;

	FName GetPortraitKey() const;

	const FSlateBrush* GetPortraitBrush() const;
	
	EVisibility GetVisibilityForMissingPortraitText() const;

protected:
	void HandleTitleTextCommitted(const FText& CommittedText, ETextCommit::Type CommitType);
	
	void HandleDialogueTextCommitted(const FText& CommittedText, ETextCommit::Type CommitType);

	void HandleDialogueAudioAssetChanged(const FAssetData& InAssetData);

	FReply HandlePortraitKeyChanged(FName NewValue);

	virtual void AddPin(const TSharedRef<SGraphPin>& PinToAdd) override;

	void CreateOutputSideAddButton(TSharedPtr<SVerticalBox> OutputBox) override;
public:
	SLATE_BEGIN_ARGS(SFlowGraphNode_YapSpeechBaseWidget)
	{
		
	}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, UFlowGraphNode* InNode);

protected:
	UFlowGraphNode_YapSpeechBase* FlowGraphNode_YapSpeechBase = nullptr;
	
	UFlowNode_YapSpeechBase* FlowNode_YapSpeechBase = nullptr;

	TSharedPtr<SMultiLineEditableTextBox> DialogueBox;
	
};