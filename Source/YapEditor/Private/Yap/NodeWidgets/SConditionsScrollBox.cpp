#include "Yap/NodeWidgets/SConditionsScrollBox.h"

#include "Yap/YapColors.h"
#include "Yap/YapCondition.h"
#include "Yap/YapEditorStyle.h"
#include "Yap/Nodes/FlowNode_YapDialogue.h"
#include "Yap/NodeWidgets/SConditionDetailsViewWidget.h"

TSharedPtr<SConditionDetailsViewWidget> SConditionsScrollBox::ConditionDetailsWidget = nullptr;
int32 SConditionsScrollBox::SelectedFragmentIndex = INDEX_NONE;
int32 SConditionsScrollBox::SelectedConditionIndex = INDEX_NONE;

// ----------------------------------------------
void SConditionsScrollBox::Construct(const FArguments& InArgs)
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
FReply SConditionsScrollBox::OnClicked_AddConditionButton()
{
	OnClickNewConditionButton.Execute(FragmentIndex);

	OnConditionsUpdated();

	OnConditionClicked(ConditionsArray->Num() - 1);
	
	return FReply::Handled();
}

// ----------------------------------------------
FSlateColor SConditionsScrollBox::ButtonColorAndOpacity_ConditionButton(UYapCondition* Condition) const
{
	return IsValid(Condition) ? Condition->GetNodeColor() : YapColor::DeepOrangeRed;
}

// ----------------------------------------------
FSlateColor SConditionsScrollBox::ForegroundColor_ConditionButton(UYapCondition* Condition) const
{
	FSlateColor ButtonColor = ButtonColorAndOpacity_ConditionButton(Condition);
	
	return ButtonColor.GetSpecifiedColor().GetLuminance() < 0.7 ? YapColor::White : YapColor::Black;
}

// ----------------------------------------------
FReply SConditionsScrollBox::OnConditionClicked(int32 ConditionIndex)
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
	
	TSharedPtr<SConditionDetailsViewWidget> NewConditionDetailsWidget = SNew(SConditionDetailsViewWidget)
		.Dialogue(DialogueNode)
		.FragmentIndex(FragmentIndex)
		.ConditionIndex(ConditionIndex)
		.OnClickedDelete(this, &SConditionsScrollBox::OnClickedDelete_Condition)
		.OnClickedNewClass(this, &SConditionsScrollBox::OnClickedNewClass_Condition);

	TSharedPtr<SWidget> ConditionButton = ConditionButtons[ConditionIndex];
	NewConditionDetailsWidget->ParentButton = ConditionButtons[ConditionIndex];

	UpdateConditionDetailsWidget(NewConditionDetailsWidget);
	
	return FReply::Handled();
}

// ----------------------------------------------
UYapCondition* SConditionsScrollBox::GetCondition(int32 ConditionIndex) const
{
	return ConditionsArray->IsValidIndex(ConditionIndex) ? (*ConditionsArray)[ConditionIndex] : nullptr;
}

// ----------------------------------------------
UYapCondition* SConditionsScrollBox::GetCondition(int32 ConditionIndex)
{
	return const_cast<UYapCondition*>(const_cast<const SConditionsScrollBox*>(this)->GetCondition(ConditionIndex));
}

// ----------------------------------------------
void SConditionsScrollBox::OnConditionsUpdated()
{
	ScrollBox->ClearChildren();
	ConditionButtons.Empty();

	for (int32 i = 0; i < ConditionsArray->Num(); ++i)
	{
		UYapCondition* Condition = (*ConditionsArray)[i];
		
		TSharedRef<SWidget> ConditionButton = SNew(SButton)
			.ButtonColorAndOpacity(this, &SConditionsScrollBox::ButtonColorAndOpacity_ConditionButton, Condition)
			.ContentPadding(FMargin(4, 2, 4, 0))
			.HAlign(HAlign_Center)
			.ForegroundColor(this, &SConditionsScrollBox::ForegroundColor_ConditionButton, Condition)
			.ButtonStyle(FYapEditorStyle::Get(), YapStyles.ButtonStyle_ConditionWidget)
			.ToolTipText(INVTEXT("Prerequisite for this to run."))
			.OnClicked(this, &SConditionsScrollBox::OnConditionClicked, i)
			[
				SNew(STextBlock)
				.Text(this, &SConditionsScrollBox::Text_ConditionButton, i)
				.ColorAndOpacity(FSlateColor::UseForeground())
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
			];
		
		ConditionButtons.Add(ConditionButton);
		
		ScrollBox->AddSlot()
		.Padding(0, 0, 4, 0)
		[
			SNew(SBox)
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
		.HAlign(HAlign_Center)
		.ButtonStyle(FAppStyle::Get(), "SimpleButton")
		.ToolTipText(INVTEXT("Add Condition"))
		.OnClicked(this, &SConditionsScrollBox::OnClicked_AddConditionButton)
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
void SConditionsScrollBox::UpdateConditionDetailsWidget(TSharedPtr<SConditionDetailsViewWidget> InConditionDetailsWidget)
{
	ConditionDetailsWidget = InConditionDetailsWidget;
	
	OnUpdateConditionDetailsWidget.Execute(ConditionDetailsWidget);
}

// ----------------------------------------------
FText SConditionsScrollBox::Text_ConditionButton(int32 ConditionIndex) const
{
	UYapCondition* Condition = GetCondition(ConditionIndex);

	if (Condition)
	{
		return FText::FromString(GetCondition(ConditionIndex)->GetDescription());
	}
	else
	{
		return FText::FromString("-");
	}
}

// ----------------------------------------------
void SConditionsScrollBox::OnClickedDelete_Condition(int ConditionIndex)
{
	ConditionsArray->RemoveAt(ConditionIndex);

	OnConditionsUpdated();
	UpdateConditionDetailsWidget();
}

void SConditionsScrollBox::OnClickedNewClass_Condition(int ConditionIndex)
{
	OnConditionsUpdated();
	UpdateConditionDetailsWidget();
}
