#pragma once

#include "PropertyPath.h"
#include "Widgets/SCompoundWidget.h"
#include "Yap/YapCondition.h"

class SWidget;
struct FYapFragment;
class UFlowNode_YapDialogue;

DECLARE_DELEGATE_OneParam(FOnClickedDeleteCondition, int32 /*ConditionIndex*/);
DECLARE_DELEGATE_OneParam(FOnClickedNewClassCondition, int32 /*ConditionIndex*/);

class SYapConditionDetailsViewWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SYapConditionDetailsViewWidget)
		:
		_Dialogue(nullptr),
		_FragmentIndex(INDEX_NONE),
		_ConditionIndex(INDEX_NONE)
		{}
		SLATE_ARGUMENT(UFlowNode_YapDialogue*, Dialogue)
		SLATE_ARGUMENT(int32, FragmentIndex)
		SLATE_ARGUMENT(int32, ConditionIndex)
		SLATE_EVENT(FOnClickedDeleteCondition, OnClickedDelete)
		SLATE_EVENT(FOnClickedNewClassCondition, OnClickedNewClass)
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

	TWeakObjectPtr<UFlowNode_YapDialogue> Dialogue;
	
	FPropertyPath ConditionPropertyPath;
	
	FOnClickedDeleteCondition OnClickedDelete;

	FOnClickedNewClassCondition OnClickedNewClassCondition;
	
//	FArrayProperty* ConditionsArrayProperty;
	
//	TWeakObjectPtr<UYapCondition> ConditionWeakPtr = nullptr;

public:
	TSharedPtr<SWidget> ParentButton;
	
	int32 FragmentIndex = INDEX_NONE;

	int32 ConditionIndex = INDEX_NONE;
};
