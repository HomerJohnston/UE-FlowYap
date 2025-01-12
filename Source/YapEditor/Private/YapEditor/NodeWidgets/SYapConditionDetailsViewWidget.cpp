// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license.

#include "YapEditor/NodeWidgets/SYapConditionDetailsViewWidget.h"

#include "PropertyCustomizationHelpers.h"
#include "YapEditor/YapColors.h"
#include "YapEditor/YapEditorStyle.h"
#include "YapEditor/YapTransactions.h"
#include "Yap/Nodes/FlowNode_YapDialogue.h"
#include "YapEditor/YapEditorSettings.h"
#include "YapEditor/YapEditorSubsystem.h"
#include "YapEditor/YapLogEditor.h"

#define LOCTEXT_NAMESPACE "YapEditor"

// ----------------------------------------------
void SYapConditionDetailsViewWidget::Construct(const FArguments& InArgs)
{
	check(InArgs._Dialogue.IsValid());
	check(InArgs._ConditionIndex != INDEX_NONE)
	
	Dialogue = InArgs._Dialogue;
	FragmentIndex = InArgs._FragmentIndex;
	ConditionIndex = InArgs._ConditionIndex;
	OnClickedDelete = InArgs._OnClickedDelete;
	OnSelectedNewClass = InArgs._OnClickedNewClass;
	
	//ConditionsArrayProperty = InArgs._ConditionsArray;
	
	//TArray<UYapCondition*>* ConditionsArray = ConditionsArrayProperty->ContainerPtrToValuePtr<TArray<UYapCondition*>>(InArgs._ConditionsContainer);

	//ConditionWeakPtr = (*ConditionsArray)[ConditionIndex];
	
	FDetailsViewArgs Args;
	Args.bHideSelectionTip = true;
	Args.bLockable = false;
	Args.bShowOptions = false;
	Args.bAllowSearch = false;
	Args.bShowPropertyMatrixButton = false;
	Args.DefaultsOnlyVisibility = EEditDefaultsOnlyNodeVisibility::Automatic;
	
	FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");

	TSharedRef<IDetailsView> DetailsWidget = PropertyEditorModule.CreateDetailView(Args);
	DetailsWidget->SetObject(GetCondition());

	FVector2D CachedSize = UYapEditorSubsystem::Get()->GetCachedSize(GetCondition()->GetClass());

	float MinDesiredWidth = GetCondition()->GetDetailsViewWidth();
	float MinDesiredHeight =  GetCondition()->GetDetailsViewHeight();
	
	if (MinDesiredWidth == 0.0)
	{
		MinDesiredWidth = CachedSize.X;
	}

	if (MinDesiredHeight == 0.0)
	{
		MinDesiredHeight = CachedSize.Y;
	}

	MinDesiredWidth = FMath::Min(MinDesiredWidth, UYapEditorSettings::GetMaxConditionDetailsWidth());
	MinDesiredHeight = FMath::Min(MinDesiredHeight, UYapEditorSettings::GetMaxConditionDetailsHeight());

	ChildSlot
	[
		SNew(SBox)
		.MinDesiredWidth(MinDesiredWidth)
		.MinDesiredHeight(MinDesiredHeight)
		.MaxDesiredWidth(UYapEditorSettings::GetMaxConditionDetailsWidth())
		.MaxDesiredHeight(UYapEditorSettings::GetMaxConditionDetailsHeight())
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(8, 8, 8, 2)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.Padding(0, 0, 2, 0)
				.FillWidth(1.0)
				[
					SNew(SClassPropertyEntryBox)
					.OnSetClass(this, &SYapConditionDetailsViewWidget::OnSetClass_ConditionProperty)
					.MetaClass(UYapCondition::StaticClass())
					.SelectedClass(this, &SYapConditionDetailsViewWidget::SelectedClass_ConditionProperty)
					.AllowAbstract(false)
					.ShowTreeView(false)
					.AllowNone(false)
				]
				+ SHorizontalBox::Slot()
				.Padding(2, 1, 0, 1)
				.AutoWidth()
				[
					SNew(SButton)
					.ButtonColorAndOpacity(YapColor::DeepOrangeRed)
					.ContentPadding(FMargin(4, 2, 4, 2))
					.ForegroundColor(YapColor::White)
					.ButtonStyle(FYapEditorStyle::Get(), YapStyles.ButtonStyle_ConditionWidget)
					.ToolTipText(LOCTEXT("DeleteCondition", "Delete condition"))
					.OnClicked(this, &SYapConditionDetailsViewWidget::OnClicked_Delete)
					[
						SNew(SImage)
						//.Image(FYapEditorStyle::GetImageBrush(YapBrushes.Icon_Delete))
						.Image(FAppStyle::Get().GetBrush("Icons.Delete"))
						.DesiredSizeOverride(FVector2D(16, 16))
						.ColorAndOpacity(FSlateColor::UseStyle())
					]
				]
			]
			+ SVerticalBox::Slot()
			.FillHeight(1.0)
			.Padding(8, 0, 8, 8)
			[
				DetailsWidget
			]
		]
	];
	
	DetailsWidgetWeakPtr = DetailsWidget;
	ConditionClass = GetCondition()->GetClass();
}

// ----------------------------------------------
FReply SYapConditionDetailsViewWidget::OnClicked_Delete() const
{
	OnClickedDelete.Execute(ConditionIndex);
	return FReply::Handled();
}

// ----------------------------------------------
const UYapCondition* SYapConditionDetailsViewWidget::GetCondition() const
{
	if (FragmentIndex == INDEX_NONE)
	{
		return Dialogue->GetConditionsMutable()[ConditionIndex];		
	}
	else
	{
		return Dialogue->GetFragmentsMutable()[FragmentIndex].GetConditionsMutable()[ConditionIndex];
	}
}

// ----------------------------------------------
UYapCondition* SYapConditionDetailsViewWidget::GetCondition()
{
	return const_cast<UYapCondition*>(const_cast<const SYapConditionDetailsViewWidget*>(this)->GetCondition());
}

// ----------------------------------------------
const UClass* SYapConditionDetailsViewWidget::SelectedClass_ConditionProperty() const
{
	if (GetCondition())
	{
		return GetCondition()->GetClass();
	}

	return nullptr;
}

// ----------------------------------------------
void SYapConditionDetailsViewWidget::OnSetClass_ConditionProperty(const UClass* NewConditionClass)
{
	UClass* ExistingConditionClass = nullptr;

	UYapCondition* ExistingCondition = GetCondition();

	if (IsValid(ExistingCondition))
	{
		ExistingConditionClass = ExistingCondition->GetClass();
	}
	
	if (ExistingConditionClass == NewConditionClass)
	{
		return;
	}

	FYapTransactions::BeginModify(LOCTEXT("ChangeConditionType", "Change Condition type"), Dialogue.Get());

	if (FragmentIndex != INDEX_NONE)
	{
		TArray<TObjectPtr<UYapCondition>>& ConditionsArray = Dialogue->GetFragmentsMutable()[FragmentIndex].GetConditionsMutable();

		if (NewConditionClass)
		{
			ConditionsArray[ConditionIndex] = NewObject<UYapCondition>(Dialogue.Get(), NewConditionClass);
		}
		else
		{
			ConditionsArray[ConditionIndex] = nullptr;
		}
	}
	else
	{
		TArray<UYapCondition*>& ConditionsArray = Dialogue->GetConditionsMutable();

		if (NewConditionClass)
		{
			ConditionsArray[ConditionIndex] = NewObject<UYapCondition>(Dialogue.Get(), NewConditionClass);
		}
		else
		{
			ConditionsArray[ConditionIndex] = nullptr;
		}
	}

	FYapTransactions::EndModify();

	OnSelectedNewClass.Execute(ConditionIndex);
}

void SYapConditionDetailsViewWidget::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	UYapEditorSubsystem::Get()->SetCachedSize(ConditionClass.Get(), FVector2D(AllottedGeometry.Size.X, AllottedGeometry.Size.Y));
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);
}

#undef LOCTEXT_NAMESPACE
