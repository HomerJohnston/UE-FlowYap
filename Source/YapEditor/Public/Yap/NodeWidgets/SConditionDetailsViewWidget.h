#pragma once

#include "PropertyPath.h"
#include "Widgets/SCompoundWidget.h"
#include "Yap/YapCondition.h"

struct FYapFragment;
class UFlowNode_YapDialogue;

class SConditionDetailsViewWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SConditionDetailsViewWidget)
		:
		_Dialogue(nullptr),
		_Condition(nullptr),
		_Fragment(nullptr),
		_ConditionIndexInArray(INDEX_NONE)
		{}
		SLATE_ARGUMENT(UFlowNode_YapDialogue*, Dialogue)
		SLATE_ARGUMENT(UYapCondition*, Condition)
		SLATE_ARGUMENT(FYapFragment*, Fragment)
		SLATE_ARGUMENT(int32, ConditionIndexInArray)
	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs);

private:
	const UClass* SelectedClass_ConditionProperty() const;

	void OnSetClass_ConditionProperty(const UClass* NewConditionClass);

private:
	TSharedPtr<IDetailsView> DetailView;

	TSharedPtr<IStructureDetailsView> StructDetailView;

	TWeakObjectPtr<UFlowNode_YapDialogue> Dialogue;

	// TODO this is dangerous. this will crash.
	FYapFragment* Fragment = nullptr;
	
	TWeakObjectPtr<UYapCondition> Condition;

	FPropertyPath ConditionPropertyPath;

	int32 ConditionIndexInArray = 0;
};
