﻿#pragma once

#include "Widgets/SCompoundWidget.h"
#include "Yap/YapCondition.h"

class SConditionDetailsViewWidget;
class SCanvas;
class SVirtualWindow;
class UFlowNode_YapDialogue;

class SConditionEntryWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SConditionEntryWidget) :
		_Condition(nullptr),
		_DialogueNode(nullptr)
		{}
		SLATE_ARGUMENT(UYapCondition*, Condition)
		SLATE_ARGUMENT(UFlowNode_YapDialogue*, DialogueNode)

SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs);

private:
	TSharedPtr<IDetailsView> DetailView;
	TSharedPtr<IStructureDetailsView> StructDetailView;
	TWeakObjectPtr<UYapCondition> Condition;
	TWeakObjectPtr<UFlowNode_YapDialogue> DialogueNode;

	FReply OnClicked();

	void Focus();

	void SetNodeSelected();
	
	void Unfocus();

	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

	TSharedRef<IDetailsView> MakeDetailsWidget();

	TSharedPtr<SConditionDetailsViewWidget> VirtualWindow;

public:
	TDelegate<void(TSharedPtr<SConditionDetailsViewWidget>)> OnClick;
};
