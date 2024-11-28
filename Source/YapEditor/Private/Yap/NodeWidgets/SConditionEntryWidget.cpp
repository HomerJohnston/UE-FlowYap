#include "Yap/NodeWidgets/SConditionEntryWidget.h"

#include "Graph/FlowGraphEditor.h"
#include "Graph/FlowGraphUtils.h"
#include "Input/HittestGrid.h"
#include "Widgets/SVirtualWindow.h"
#include "Yap/YapColors.h"
#include "Yap/YapEditorStyle.h"
#include "Widgets/SCanvas.h"
#include "Yap/Nodes/FlowNode_YapDialogue.h"
#include "Yap/NodeWidgets/SConditionDetailsViewWidget.h"

void SConditionEntryWidget::Construct(const FArguments& InArgs)
{
	Condition = InArgs._Condition;
	DialogueNode = InArgs._DialogueNode;
	
	FString Description = IsValid(InArgs._Condition) ? InArgs._Condition->GetDescription() : "-";

	FLinearColor ButtonColor = IsValid(InArgs._Condition) ? InArgs._Condition->GetNodeColor() : YapColor::DeepOrangeRed;
	FLinearColor TextColor = ButtonColor.GetLuminance() < 0.6 ? YapColor::White : YapColor::Black;
	
	ChildSlot
	[
		SNew(SOverlay)
		+ SOverlay::Slot()
		[
			SNew(SButton)
			.ButtonColorAndOpacity(ButtonColor)
			.ContentPadding(FMargin(4, 0, 4, 0))
			.ForegroundColor(TextColor)
			.ButtonStyle(FYapEditorStyle::Get(), YapStyles.ButtonStyle_ConditionWidget)
			.ToolTipText(INVTEXT("Prerequisite for this to run."))
			.OnClicked(this, &SConditionEntryWidget::OnClicked)
			[
				SNew(STextBlock)
				.Text(FText::FromString(Description))
				.ColorAndOpacity(FSlateColor::UseForeground())
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
			]
		]
	];
}

FReply SConditionEntryWidget::OnClicked()
{
	if (!DialogueNode.IsValid())
	{
		return  FReply::Handled();
	}

	if (!Condition.IsValid())
	{
		return FReply::Handled();
	}

	SetNodeSelected();

	TSharedRef<SConditionEntryWidget> ThisRef = SharedThis(this);
	
	OnClick.Execute(Condition.Get(), ThisRef);
	
	return FReply::Handled();
}

void SConditionEntryWidget::Focus()
{	
	SetNodeSelected();
}

void SConditionEntryWidget::SetNodeSelected()
{
	TSharedPtr<SFlowGraphEditor> GraphEditor = FFlowGraphUtils::GetFlowGraphEditor(DialogueNode->GetGraphNode()->GetGraph());
	if (GraphEditor)
	{
		GraphEditor->SelectSingleNode(DialogueNode->GetGraphNode());
	}
}

void SConditionEntryWidget::Unfocus()
{
}
