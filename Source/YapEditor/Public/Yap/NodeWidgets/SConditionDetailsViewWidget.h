#pragma once

#include "Widgets/SCompoundWidget.h"
#include "Yap/YapCondition.h"

class SConditionDetailsViewWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SConditionDetailsViewWidget)
		: _Condition(nullptr)
		{}
		SLATE_ARGUMENT(UYapCondition*, Condition)

	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs);

private:
	TSharedPtr<IDetailsView> DetailView;
	TSharedPtr<IStructureDetailsView> StructDetailView;
};
