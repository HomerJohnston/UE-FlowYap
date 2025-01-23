// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license.

#include "YapEditor/NodeWidgets/SYapConditionsScrollBox.h"

#include "YapEditor/YapColors.h"
#include "Yap/YapCondition.h"
#include "YapEditor/YapEditorStyle.h"
#include "YapEditor/YapTransactions.h"
#include "Yap/Nodes/FlowNode_YapDialogue.h"
#include "YapEditor/NodeWidgets/SYapButtonPopup.h"
#include "YapEditor/NodeWidgets/SYapConditionDetailsViewWidget.h"

#define LOCTEXT_NAMESPACE "YapEditor"

// ------------------------------------------------------------------------------------------------
void SYapConditionsScrollBox::Construct(const FArguments& InArgs)
{
	DialogueNode = InArgs._DialogueNode;
	FragmentIndex = InArgs._FragmentIndex;
	OnConditionsArrayChanged = InArgs._OnConditionsArrayChanged;
	OnConditionDetailsViewBuilt = InArgs._OnConditionDetailsViewBuilt;
	ConditionsArrayProperty = InArgs._ConditionsArrayProperty;
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
	
	RebuildConditionButtons();
}

SYapConditionsScrollBox::~SYapConditionsScrollBox()
{
	//DestroyConditionDetailsWidget();
}

// ------------------------------------------------------------------------------------------------
void SYapConditionsScrollBox::RebuildConditionButtons()
{
	ScrollBox->ClearChildren();
	ConditionButtons.Empty();
	
	for (int32 i = 0; i < ConditionsArray->Num(); ++i)
	{
		TSharedRef<SWidget> Button = CreateConditionButton(i);
		ConditionButtons.Add(Button);

		ScrollBox->AddSlot()
		.Padding(0, 0, 4, 0)
		[
			Button
		];
	}

	ScrollBox->AddSlot()
	.Padding(0, 0, 4, 0)
	[
		CreateAddConditionButton()
	];
}

// ------------------------------------------------------------------------------------------------
TSharedRef<SWidget> SYapConditionsScrollBox::CreateAddConditionButton()
{
	return SNew(SButton)
	.Cursor(EMouseCursor::Default)
	.HAlign(HAlign_Center)
	.ButtonStyle(FAppStyle::Get(), "SimpleButton")
	.ToolTipText(LOCTEXT("AddCondition_ToolTip", "Add Condition"))
	.OnClicked(this, &SYapConditionsScrollBox::OnClicked_AddConditionButton)
	.ContentPadding(0)
	[
		SNew(SBox)
		.VAlign(VAlign_Center)
		[
			SNew(SImage)
			.Image(FAppStyle::GetBrush(TEXT("Icons.PlusCircle")))
			.ColorAndOpacity(YapColor::Button_Unset())
		]
	];
}

// ------------------------------------------------------------------------------------------------
FReply SYapConditionsScrollBox::OnClicked_AddConditionButton()
{
	if (!DialogueNode.IsValid())
	{
		UE_LOG(LogYap, Warning, TEXT("Dialogue node is null - this should not happen, please inform the plugin author!"))
		return FReply::Handled();
	}
	
	FYapTransactions::BeginModify(LOCTEXT("AddCondition", "Add condition"), DialogueNode.Get());

	//DestroyConditionDetailsWidget();

	ConditionsArray->Add(nullptr);

	FYapTransactions::EndModify();
	
	(void)OnConditionsArrayChanged.ExecuteIfBound();

	// The dialogue widget will be rebuilt fully by the dialogue widget (which needs to rebuild to check if Bypass pin enables/disables), no need to rebuild conditions again here.
	// Leave this existing and ommented out for future reference.
	// RebuildConditionButtons();
	
	return FReply::Handled();
}

// ------------------------------------------------------------------------------------------------
TSharedRef<SWidget> SYapConditionsScrollBox::CreateConditionButton(int32 ConditionIndex)
{
	return SNew(SYapButtonPopup)
	.PopupContentGetter(FPopupContentGetter::CreateSP(this, &SYapConditionsScrollBox::PopupContentGetter, ConditionIndex))
	.PopupPlacement(MenuPlacement_BelowAnchor)
	.ButtonStyle(FYapEditorStyle::Get(), YapStyles.ButtonStyle_ConditionWidget)
	.ButtonForegroundColor(this, &SYapConditionsScrollBox::ForegroundColor_ConditionButton, ConditionIndex)
	.ButtonBackgroundColor(this, &SYapConditionsScrollBox::ButtonColorAndOpacity_ConditionButton, ConditionIndex)
	.ButtonContentPadding(FMargin(8, 2, 8, 0))
	.ButtonContent()
	[
		SNew(STextBlock)
		.Text(this, &SYapConditionsScrollBox::Text_ConditionButton, ConditionIndex)
		.ColorAndOpacity(FSlateColor::UseForeground())
		.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
	];
}

TSharedRef<SWidget> SYapConditionsScrollBox::PopupContentGetter(int32 ConditionIndex)
{
	return SNew(SYapConditionDetailsViewWidget)
		.Dialogue(DialogueNode)
		.FragmentIndex(FragmentIndex)
		.ConditionIndex(ConditionIndex)
		.OnClickedDelete(this, &SYapConditionsScrollBox::OnClicked_DeleteConditionButton)
		.OnClickedNewClass(this, &SYapConditionsScrollBox::OnSet_NewConditionClass);
}

// ------------------------------------------------------------------------------------------------
FLinearColor SYapConditionsScrollBox::ButtonColorAndOpacity_ConditionButton(int32 ConditionIndex) const
{
	UYapCondition* Condition = GetCondition(ConditionIndex);
	
	return IsValid(Condition) ? Condition->GetColor() : YapColor::Red;
}

// ------------------------------------------------------------------------------------------------
FLinearColor SYapConditionsScrollBox::ForegroundColor_ConditionButton(int32 ConditionIndex) const
{
	FSlateColor ButtonColor = ButtonColorAndOpacity_ConditionButton(ConditionIndex);
	
	return ButtonColor.GetSpecifiedColor().GetLuminance() < 0.7 ? YapColor::White : YapColor::Black;
}

// ------------------------------------------------------------------------------------------------
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

// ------------------------------------------------------------------------------------------------
void SYapConditionsScrollBox::OnClicked_DeleteConditionButton(int ConditionIndex)
{
	if (!DialogueNode.IsValid())
	{
		UE_LOG(LogYap, Warning, TEXT("Dialogue node is null - this should not happen, please inform the plugin author!"))
		return;
	}
	
	FYapTransactions::BeginModify(LOCTEXT("DeleteYapCondition", "Delete condition"), DialogueNode.Get());

	ConditionsArray->RemoveAt(ConditionIndex);

	FYapTransactions::EndModify();

	(void)OnConditionsArrayChanged.ExecuteIfBound();

	// The dialogue widget will be rebuilt fully by the dialogue widget (which needs to rebuild to check if Bypass pin enables/disables), no need to rebuild conditions again here
	// RebuildConditionButtons();
}

// ------------------------------------------------------------------------------------------------
void SYapConditionsScrollBox::OnSet_NewConditionClass(int ConditionIndex)
{
	RebuildConditionButtons();

	TSharedPtr<SWidget> X = ConditionButtons[ConditionIndex];
	TSharedPtr<SYapButtonPopup> Popup = StaticCastSharedPtr<SYapButtonPopup>(ConditionButtons[ConditionIndex]);

	Popup->OnClicked_Button();
}


void SYapConditionsScrollBox::DestroyConditionDetailsWidget()
{
	//(void)OnConditionDetailsViewBuilt.ExecuteIfBound(nullptr, nullptr);
}

// ------------------------------------------------------------------------------------------------
TSharedPtr<SWidget> SYapConditionsScrollBox::GetEditedButton(int32 ConditionIndex)
{
	return ConditionButtons[ConditionIndex];
}

// ------------------------------------------------------------------------------------------------
UYapCondition* SYapConditionsScrollBox::GetCondition(int32 ConditionIndex) const
{
	return ConditionsArray->IsValidIndex(ConditionIndex) ? (*ConditionsArray)[ConditionIndex] : nullptr;
}

// ------------------------------------------------------------------------------------------------
UYapCondition* SYapConditionsScrollBox::GetCondition(int32 ConditionIndex)
{
	return const_cast<UYapCondition*>(const_cast<const SYapConditionsScrollBox*>(this)->GetCondition(ConditionIndex));
}

#undef LOCTEXT_NAMESPACE