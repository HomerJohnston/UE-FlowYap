#pragma once

#include "Widgets/SCompoundWidget.h"

class SYapConditionDetailsViewWidget;
class UFlowNode_YapDialogue;
class UYapCondition;
class SConditionEntryWidget;
class SYapConditionsScrollBox;

DECLARE_DELEGATE_OneParam(FOnUpdateConditionDetailsWidget, TSharedPtr<SYapConditionDetailsViewWidget>);
DECLARE_DELEGATE_OneParam(FOnClickNewConditionButton, int32 /*FragmentIndex*/)

class SYapConditionsScrollBox : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SYapConditionsScrollBox) :
		_DialogueNode(nullptr),
		_FragmentIndex(INDEX_NONE)
		{}
		SLATE_ARGUMENT(UFlowNode_YapDialogue*, DialogueNode)
		SLATE_ARGUMENT(int32, FragmentIndex)
		SLATE_EVENT(FOnUpdateConditionDetailsWidget, OnUpdateConditionDetailsWidget)
		SLATE_EVENT(FOnClickNewConditionButton, OnClickNewConditionButton)
		SLATE_ARGUMENT(FArrayProperty*, ConditionsArray)
		SLATE_ARGUMENT(void*, ConditionsContainer)
	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs);

public:
	FReply OnClicked_AddConditionButton();

	FSlateColor ButtonColorAndOpacity_ConditionButton(UYapCondition* Condition) const;

	FSlateColor ForegroundColor_ConditionButton(UYapCondition* Condition) const;
	
	void OnConditionsUpdated();

	void UpdateConditionDetailsWidget(TSharedPtr<SYapConditionDetailsViewWidget> InConditionDetailsWidget = nullptr);
	
private:
	TSharedPtr<SScrollBox> ScrollBox = nullptr;

	TArray<TSharedPtr<SWidget>> ConditionButtons;
	
private:
	UFlowNode_YapDialogue* DialogueNode = nullptr;

	int32 FragmentIndex = INDEX_NONE;

	FOnUpdateConditionDetailsWidget OnUpdateConditionDetailsWidget;

	FOnClickNewConditionButton OnClickNewConditionButton;
	
	FArrayProperty* ConditionsArrayProperty = nullptr;

	void* ConditionsContainer = nullptr;

	TArray<UYapCondition*>* ConditionsArray = nullptr;
	
 	FText Text_ConditionButton(int32 ConditionIndex) const;

	void OnClickedDelete_Condition(int ConditionIndex);

	void OnClickedNewClass_Condition(int ConditionIndex);
	
	FReply OnConditionClicked(int32 ConditionIndex);
	
	UYapCondition* GetCondition(int32 ConditionIndex) const;
	
	UYapCondition* GetCondition(int32 ConditionIndex);

	static TWeakPtr<SYapConditionDetailsViewWidget> ConditionDetailsWidget;
	static int32 SelectedFragmentIndex;
	static int32 SelectedConditionIndex;
};
