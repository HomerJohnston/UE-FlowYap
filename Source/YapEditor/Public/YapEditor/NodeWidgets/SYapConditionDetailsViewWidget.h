// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license.

#pragma once

#include "PropertyPath.h"
#include "Widgets/SCompoundWidget.h"
#include "Yap/YapCondition.h"

class SWidget;
struct FYapFragment;
class UFlowNode_YapDialogue;

#define LOCTEXT_NAMESPACE "YapEditor"

DECLARE_DELEGATE_OneParam(FOnClickedDeleteCondition, int32 /*ConditionIndex*/);
DECLARE_DELEGATE_OneParam(FOnSelectedNewConditionClass, int32 /*ConditionIndex*/);

class SYapConditionDetailsViewWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SYapConditionDetailsViewWidget)
		:
		_Dialogue(nullptr),
		_FragmentIndex(INDEX_NONE),
		_ConditionIndex(INDEX_NONE)
		{}
		SLATE_ARGUMENT(TWeakObjectPtr<UFlowNode_YapDialogue>, Dialogue)
		SLATE_ARGUMENT(int32, FragmentIndex)
		SLATE_ARGUMENT(int32, ConditionIndex)
		SLATE_EVENT(FOnClickedDeleteCondition, OnClickedDelete)
		SLATE_EVENT(FOnSelectedNewConditionClass, OnClickedNewClass)
		SLATE_ARGUMENT(FArrayProperty*, ConditionsArray)
		SLATE_ARGUMENT(void*, ConditionsContainer)
	SLATE_END_ARGS()

public:
	FReply OnClicked_Delete() const;
	
	void Construct(const FArguments& InArgs);

private:	
	const UYapCondition* GetCondition() const;
	
	UYapCondition* GetCondition();
	
	const UClass* SelectedClass_ConditionProperty() const;

	void OnSetClass_ConditionProperty(const UClass* NewConditionClass);

private:
	TSharedPtr<IDetailsView> DetailView;

	TSharedPtr<IStructureDetailsView> StructDetailView;

	TWeakPtr<IDetailsView> DetailsWidgetWeakPtr;
	
	FPropertyPath ConditionPropertyPath;
	
	FOnClickedDeleteCondition OnClickedDelete;

	FOnSelectedNewConditionClass OnSelectedNewClass;

	TWeakObjectPtr<UClass> ConditionClass;
	
public:
	TWeakObjectPtr<UFlowNode_YapDialogue> Dialogue;

	int32 FragmentIndex = INDEX_NONE;

	int32 ConditionIndex = INDEX_NONE;

	void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

	/** When you first open details widgets, they go through a ridiculous resizing process that takes about 4 frames.
	 * What I do is cache it in here on tick, so that next time you open the widget, it doesn't do it again. */
	static TMap<TWeakObjectPtr<UClass>, float> CachedDetailsWidgetHeights; 

public:
	static float GetCachedHeight(TWeakObjectPtr<UClass> ConditionClass);

	static void SetCachedHeight(TWeakObjectPtr<UClass> ConditionClass, float);
	
};

#undef LOCTEXT_NAMESPACE