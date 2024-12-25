#include "Yap/NodeWidgets/SYapConditionsScrollBox.h"

#include "Yap/YapColors.h"
#include "Yap/YapCondition.h"
#include "Yap/YapEditorStyle.h"
#include "Yap/Nodes/FlowNode_YapDialogue.h"
#include "Yap/NodeWidgets/SYapConditionDetailsViewWidget.h"

TWeakPtr<SYapConditionDetailsViewWidget> SYapConditionsScrollBox::ConditionDetailsWidget = nullptr;
int32 SYapConditionsScrollBox::SelectedFragmentIndex = INDEX_NONE;
int32 SYapConditionsScrollBox::SelectedConditionIndex = INDEX_NONE;

// ----------------------------------------------
void SYapConditionsScrollBox::Construct(const FArguments& InArgs)
{
	DialogueNode = InArgs._DialogueNode;
	FragmentIndex = InArgs._FragmentIndex;
	
	OnUpdateConditionDetailsWidget = InArgs._OnUpdateConditionDetailsWidget;
	
	OnClickNewConditionButton = InArgs._OnClickNewConditionButton;
	
	ConditionsArrayProperty = InArgs._ConditionsArray;

	ConditionsContainer = InArgs._ConditionsContainer;

	ConditionsArray = ConditionsArrayProperty->ContainerPtrToValuePtr<TArray<UYapCondition*>>(ConditionsContainer);

	ChildSlot
	[
		SAssignNew(ScrollBox, SScrollBox)
		.ScrollBarVisibility(EVisibility::Collapsed)
		.ConsumeMouseWheel(EConsumeMouseWheel::Always)
		.AllowOverscroll(EAllowOverscroll::No)
		.AnimateWheelScrolling(true)
		.Orientation(Orient_Horizontal)
	];
	
	OnConditionsUpdated();
}

// ----------------------------------------------
FReply SYapConditionsScrollBox::OnClicked_AddConditionButton()
{
	OnClickNewConditionButton.Execute(FragmentIndex);

	OnConditionsUpdated();

	OnConditionClicked(ConditionsArray->Num() - 1);
	
	return FReply::Handled();
}

// ----------------------------------------------
FSlateColor SYapConditionsScrollBox::ButtonColorAndOpacity_ConditionButton(UYapCondition* Condition) const
{
	return IsValid(Condition) ? Condition->GetColor() : YapColor::DeepOrangeRed;
}

// ----------------------------------------------
FSlateColor SYapConditionsScrollBox::ForegroundColor_ConditionButton(UYapCondition* Condition) const
{
	FSlateColor ButtonColor = ButtonColorAndOpacity_ConditionButton(Condition);
	
	return ButtonColor.GetSpecifiedColor().GetLuminance() < 0.7 ? YapColor::White : YapColor::Black;
}

// ----------------------------------------------
FReply SYapConditionsScrollBox::OnConditionClicked(int32 ConditionIndex)
{
	if (ConditionDetailsWidget != nullptr)
	{
		if (SelectedFragmentIndex == FragmentIndex && SelectedConditionIndex == ConditionIndex)
		{
			UpdateConditionDetailsWidget();
			return FReply::Handled();
		}
	}
	
	if (!IsValid(DialogueNode))
	{
		return  FReply::Handled();
	}

	SelectedFragmentIndex = FragmentIndex;
	SelectedConditionIndex = ConditionIndex;
	
	TSharedPtr<SYapConditionDetailsViewWidget> NewConditionDetailsWidget = SNew(SYapConditionDetailsViewWidget)
		.Dialogue(DialogueNode)
		.FragmentIndex(FragmentIndex)
		.ConditionIndex(ConditionIndex)
		.OnClickedDelete(this, &SYapConditionsScrollBox::OnClickedDelete_Condition)
		.OnClickedNewClass(this, &SYapConditionsScrollBox::OnClickedNewClass_Condition);

	TSharedPtr<SWidget> ConditionButton = ConditionButtons[ConditionIndex];
	NewConditionDetailsWidget->ParentButton = ConditionButtons[ConditionIndex];

	UpdateConditionDetailsWidget(NewConditionDetailsWidget);
	
	return FReply::Handled();
}

// ----------------------------------------------
UYapCondition* SYapConditionsScrollBox::GetCondition(int32 ConditionIndex) const
{
	return ConditionsArray->IsValidIndex(ConditionIndex) ? (*ConditionsArray)[ConditionIndex] : nullptr;
}

// ----------------------------------------------
UYapCondition* SYapConditionsScrollBox::GetCondition(int32 ConditionIndex)
{
	return const_cast<UYapCondition*>(const_cast<const SYapConditionsScrollBox*>(this)->GetCondition(ConditionIndex));
}

// ----------------------------------------------
void SYapConditionsScrollBox::OnConditionsUpdated()
{
	ScrollBox->ClearChildren();
	ConditionButtons.Empty();

	for (int32 i = 0; i < ConditionsArray->Num(); ++i)
	{
		UYapCondition* Condition = (*ConditionsArray)[i];
		
		TSharedRef<SWidget> ConditionButton = SNew(SButton)
			
			.ButtonColorAndOpacity(this, &SYapConditionsScrollBox::ButtonColorAndOpacity_ConditionButton, Condition)
			.ContentPadding(FMargin(4, 2, 4, 0))
			.HAlign(HAlign_Center)
			.ForegroundColor(this, &SYapConditionsScrollBox::ForegroundColor_ConditionButton, Condition)
			.ButtonStyle(FYapEditorStyle::Get(), YapStyles.ButtonStyle_ConditionWidget)
			.ToolTipText(INVTEXT("Prerequisite for this to run."))
			.OnClicked(this, &SYapConditionsScrollBox::OnConditionClicked, i)
			[
				SNew(STextBlock)
				.Text(this, &SYapConditionsScrollBox::Text_ConditionButton, i)
				.ColorAndOpacity(FSlateColor::UseForeground())
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
			];
		
		ConditionButtons.Add(ConditionButton);
		
		ScrollBox->AddSlot()
		.Padding(0, 0, 4, 0)
		[
			SNew(SBox)
			.Cursor(EMouseCursor::Default)
			.MinDesiredWidth(24)
			[
				ConditionButton
			]
		];
	}

	ScrollBox->AddSlot()
	.Padding(0, 0, 0, 0)
	[
		SNew(SButton)
		.Cursor(EMouseCursor::Default)
		.HAlign(HAlign_Center)
		.ButtonStyle(FAppStyle::Get(), "SimpleButton")
		.ToolTipText(INVTEXT("Add Condition"))
		.OnClicked(this, &SYapConditionsScrollBox::OnClicked_AddConditionButton)
		.ContentPadding(0)
		[
			SNew(SBox)
			.VAlign(VAlign_Center)
			[
				SNew(SImage)
				.Image(FAppStyle::GetBrush(TEXT("Icons.PlusCircle")))
				.ColorAndOpacity(YapColor::DarkGray)
			]
		]
	];
}

// ----------------------------------------------
void SYapConditionsScrollBox::UpdateConditionDetailsWidget(TSharedPtr<SYapConditionDetailsViewWidget> InConditionDetailsWidget)
{
	ConditionDetailsWidget = InConditionDetailsWidget;
	
	OnUpdateConditionDetailsWidget.Execute(InConditionDetailsWidget);
}

// ----------------------------------------------
FText SYapConditionsScrollBox::Text_ConditionButton(int32 ConditionIndex) const
{
	UYapCondition* Condition = GetCondition(ConditionIndex);

	if (Condition)
	{
		return FText::FromString(GetCondition(ConditionIndex)->GetTitle());
	}
	else
	{
		return FText::FromString("-");
	}
}

// ----------------------------------------------
void SYapConditionsScrollBox::OnClickedDelete_Condition(int ConditionIndex)
{
	ConditionsArray->RemoveAt(ConditionIndex);

	OnConditionsUpdated();
	UpdateConditionDetailsWidget();
}

void SYapConditionsScrollBox::OnClickedNewClass_Condition(int ConditionIndex)
{
	OnConditionsUpdated();
	UpdateConditionDetailsWidget();
}
