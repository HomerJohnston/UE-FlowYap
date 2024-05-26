#pragma once

#include "CoreMinimal.h"
#include "Graph/Widgets/SFlowGraphNode.h"
#include "Widgets/SCompoundWidget.h"

class UFlowNode_YapDialogue;
class UFlowGraphNode_YapDialogue;
/**
 *
 */
class SFlowGraphNode_YapDialogue : public SFlowGraphNode
{
protected:

	void UpdateGraphNode() override;

	bool HasSingleOutput() const;

	bool HasMultipleOutput() const;

	int GetSingleFlowMode() const;

	void HandleSingleFlowModeChanged(int I);

	int GetMultiFlowMode() const;

	void HandleMultiFlowModeChanged(int I);
	
	virtual TSharedRef<SWidget> CreateDialogueContentArea();

	TSharedRef<SBox> CreatePortraitKeyButton(FName InIconName = FName(), bool bSelected = false, const FText& InLabel = FText::GetEmpty(), FName InTextStyle = TEXT("ButtonText"));
	
	TSharedRef<SWidget> CreateNodeContentArea() override;
	
	void CreateBelowPinControls(TSharedPtr<SVerticalBox> MainBox) override;

	void HandleTitleTextCommitted(const FText& CommittedText, ETextCommit::Type CommitType);
	
	void HandleDialogueTextCommitted(const FText& CommittedText, ETextCommit::Type CommitType);

	void HandleDialogueAudioAssetChanged(const FAssetData& InAssetData);

	void HandlePortraitKeyChanged(FName NewValue);
public:
	FText GetHeadTitle() const;

	FText GetTitleText() const;

	FText GetDialogueText() const;

	FString GetSelectedDialogueAudioAssetPath() const;

	FName GetPortraitKey() const;

	const FSlateBrush* GetPortraitBrush() const;
	
	EVisibility GetVisibilityForMissingPortraitText() const;

public:
	SLATE_BEGIN_ARGS(SFlowGraphNode_YapDialogue) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, UFlowGraphNode* InNode);


	/** Used to display the name of the node and allow renaming of the node */
	TSharedPtr<SVerticalBox> TitleBox;
	TSharedPtr<SHorizontalBox> TitleBoxH;

	//FSlateBrush PortraitBrush;

	FSlateColor SpeakerTextColor;
	FSlateColor PronounTextColor;

	FSlateColor SpeakerBgColor;
	FSlateBrush SpeakerBgBrush;
	FSlateBrush SpeakerBgCornerBrush;

	static TMap<FName, TSharedPtr<FSlateBrush>> CachedPortraitKeyIcons;

	bool bHasValidPortraitBrush = false;
	
protected:
	UFlowGraphNode_YapDialogue* FlowGraphNode_YapDialogue = nullptr;
	UFlowNode_YapDialogue* FlowNode_YapDialogue = nullptr;
public:
	
	FSlateColor GetNodeBodyColor() const;
};