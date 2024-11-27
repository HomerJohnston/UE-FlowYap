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
	
	VirtualWindow = SNew(SConditionDetailsViewWidget)
		.Condition(Condition.Get());
	
	VirtualWindow->SetVisibility(EVisibility::Collapsed);

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

	Focus();

	return FReply::Handled();
}

void SConditionEntryWidget::Focus()
{
	VirtualWindow->SetVisibility(EVisibility::Visible);

	OnClick.Execute(VirtualWindow);
	
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
	VirtualWindow->SetVisibility(EVisibility::Collapsed);
}

int32 SConditionEntryWidget::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	//	VirtualWindow = SNew(SVirtualWindow); // happens in Construct
	//  VirtualWindow->SetContent(MakeDetailsWidget.ToSharedRef()); // happens upon clicking the button

	/*
	if (VirtualWindow.IsValid())
	{
		FSlateRect NewRect = FSlateRect(0, 0, 1000000, 1000000);
		FGeometry NewGeo = FGeometry::MakeRoot(FVector2f(200, 300), FSlateLayoutTransform(1, FVector2f(200, 200)));
		FPaintArgs NewArgs = Args.WithNewParent(this->GetParentWidget()->GetParentWidget()->GetParentWidget()->GetParentWidget()->GetParentWidget()->GetParentWidget()->GetParentWidget()->GetParentWidget()->GetParentWidget()->GetParentWidget()->GetParentWidget()->GetParentWidget()->GetParentWidget()->GetParentWidget()->GetParentWidget()->GetParentWidget()->GetParentWidget().Get());

		VirtualWindow->SlatePrepass(AllottedGeometry.GetAccumulatedLayoutTransform().GetScale());
		//VirtualWindow->OnPaint(NewArgs, NewGeo, NewRect, OutDrawElements, LayerId+394, InWidgetStyle, true);
	}
*/
	
	return SCompoundWidget::OnPaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);
}

TSharedRef<IDetailsView> SConditionEntryWidget::MakeDetailsWidget()
{
	FDetailsViewArgs Args;
	Args.bHideSelectionTip = true;
	Args.bLockable = false;
	Args.bShowOptions = false;
	Args.bAllowSearch = false;
	Args.bShowPropertyMatrixButton = false;
	Args.DefaultsOnlyVisibility = EEditDefaultsOnlyNodeVisibility::Show;
	Args.bShowScrollBar = false;
			
	FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");

	TSharedRef<IDetailsView> DetailsView = PropertyEditorModule.CreateDetailView(Args);

	DetailsView->SetObject(Condition.Get());

	return DetailsView;
}
