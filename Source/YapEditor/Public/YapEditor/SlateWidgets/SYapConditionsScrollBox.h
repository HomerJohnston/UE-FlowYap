// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license.

#pragma once

#include "Widgets/SCompoundWidget.h"

class SYapConditionDetailsViewWidget;
class UFlowNode_YapDialogue;
class UYapCondition;
class SConditionEntryWidget;
class SYapConditionsScrollBox;

#define LOCTEXT_NAMESPACE "YapEditor"

DECLARE_DELEGATE_TwoParams(FOnConditionDetailsViewBuilt, TSharedPtr<SYapConditionDetailsViewWidget>, TSharedPtr<SWidget>);
DECLARE_DELEGATE(FOnConditionsArrayChanged);

class SYapConditionsScrollBox : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SYapConditionsScrollBox) :
		_DialogueNode(nullptr),
		_FragmentIndex(INDEX_NONE)
		{}
		SLATE_ARGUMENT(UFlowNode_YapDialogue*, DialogueNode)
		SLATE_ARGUMENT(int32, FragmentIndex)
		SLATE_EVENT(FOnConditionsArrayChanged, OnConditionsArrayChanged)
		SLATE_EVENT(FOnConditionDetailsViewBuilt, OnConditionDetailsViewBuilt)
		SLATE_ARGUMENT(FArrayProperty*, ConditionsArrayProperty)
		SLATE_ARGUMENT(void*, ConditionsContainer)
	SLATE_END_ARGS()

	// ------------------------------------------
	// SLATE INPUTS
private:
	TWeakObjectPtr<UFlowNode_YapDialogue> DialogueNode = nullptr;
	int32 FragmentIndex = INDEX_NONE;
	FOnConditionsArrayChanged OnConditionsArrayChanged;
	FOnConditionDetailsViewBuilt OnConditionDetailsViewBuilt;
	FArrayProperty* ConditionsArrayProperty = nullptr;
	void* ConditionsContainer = nullptr;
	
	// ------------------------------------------
	// STATE
public:	
	TArray<UYapCondition*>* ConditionsArray = nullptr;
	TSharedPtr<SScrollBox> ScrollBox = nullptr;
	TArray<TSharedPtr<SWidget>> ConditionButtons;
	TWeakPtr<SYapConditionDetailsViewWidget> EditedConditionWidget;
	
	// ------------------------------------------
	// CONSTRUCTION
public:
	void Construct(const FArguments& InArgs);
	
	~SYapConditionsScrollBox();
	
protected:
	// ------------------------------------------
	void RebuildConditionButtons();
	
	// ------------------------------------------
	TSharedRef<SWidget> CreateAddConditionButton();	
	
	FReply OnClicked_AddConditionButton();

	TSharedRef<SWidget> PopupContentGetter(int32 ConditionIndex);
	// ------------------------------------------
	TSharedRef<SWidget> CreateConditionButton(int32 ConditionIndex);
	
	FLinearColor ButtonColorAndOpacity_ConditionButton(int32 ConditionIndex) const;
	FLinearColor ForegroundColor_ConditionButton(int32 ConditionIndex) const;
	FText Text_ConditionButton(int32 ConditionIndex) const;
	void OnClicked_DeleteConditionButton(int ConditionIndex);
	void OnSet_NewConditionClass(int ConditionIndex);
	FReply OnClicked_AddConditionButton(int ConditionIndex);
	// ------------------------------------------
	
public:
	void DestroyConditionDetailsWidget();
	
	TSharedPtr<SWidget> GetEditedButton(int32 ConditionIndex);
	
protected:
	UYapCondition* GetCondition(int32 ConditionIndex) const;
	
	UYapCondition* GetCondition(int32 ConditionIndex);
	
public:
	const TSharedPtr<SWidget> GetConditionButton(int32 ConditionIndex) const { return ConditionButtons[ConditionIndex]; };
};



#undef LOCTEXT_NAMESPACE
