#include "Yap/NodeWidgets/SConditionDetailsViewWidget.h"

#include "ISinglePropertyView.h"
#include "PropertyCustomizationHelpers.h"
#include "Widgets/SVirtualWindow.h"
#include "Yap/YapEditorStyle.h"
#include "Yap/Nodes/FlowNode_YapDialogue.h"

void SConditionDetailsViewWidget::Construct(const FArguments& InArgs)
{
	check(InArgs._Dialogue);
	check(InArgs._Condition);
	check(InArgs._ConditionIndexInArray != INDEX_NONE)
	
	Dialogue = InArgs._Dialogue;
	Condition = InArgs._Condition;
	ConditionIndexInArray = InArgs._ConditionIndexInArray;

	Fragment = InArgs._Fragment;
	
	FDetailsViewArgs Args;
	Args.bHideSelectionTip = true;
	Args.bLockable = false;
	Args.bShowOptions = false;
	Args.bAllowSearch = false;
	Args.bShowPropertyMatrixButton = false;
	Args.DefaultsOnlyVisibility = EEditDefaultsOnlyNodeVisibility::Automatic;
	
	FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	TSharedPtr<IDetailsView> DetailsWidget = PropertyEditorModule.CreateDetailView(Args);
	DetailsWidget->SetObject(InArgs._Condition, true);
	
	ChildSlot
	[
		SNew(SBorder)
		.Padding(1, 1, 1, 1) // No idea why but the details view already has a 4 pixel transparent area on top
		.BorderImage(FYapEditorStyle::GetImageBrush(YapBrushes.Panel_Rounded))
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(8, 8, 8, 4)
			[
				SNew(SClassPropertyEntryBox)
				.OnSetClass(this, &SConditionDetailsViewWidget::OnSetClass_ConditionProperty)
				.MetaClass(UYapCondition::StaticClass())
				.SelectedClass(this, &SConditionDetailsViewWidget::SelectedClass_ConditionProperty)
				.AllowAbstract(false)
				.ShowTreeView(true)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(8, 4, 8, 8)
			[
				DetailsWidget.ToSharedRef()
			]
		]
	];
}

const UClass* SConditionDetailsViewWidget::SelectedClass_ConditionProperty() const
{
	if (Condition.IsValid())
	{
		return Condition->GetClass();
	}

	return nullptr;
}

void SConditionDetailsViewWidget::OnSetClass_ConditionProperty(const UClass* NewConditionClass)
{
	if (Fragment)
	{
		
	}
	else
	{
		Dialogue->GetConditionsMutable()[ConditionIndexInArray] = NewObject<UYapCondition>(Dialogue.Get(), NewConditionClass);
		/*
		FArrayProperty* ConditionsProperty = CastField<FArrayProperty>(Dialogue->GetClass()->FindPropertyByName("Conditions"));
		FObjectProperty* ArrayInner = CastField<FObjectProperty>(ConditionsProperty->Inner);

		void* ArrayData = ConditionsProperty->ContainerPtrToValuePtr<void>(Dialogue.Get());
		
		FScriptArrayHelper ArrayHelper(ConditionsProperty, ArrayData);

		ArrayHelper.Insert()
		
		ConditionsProperty->SetValue_InContainer()
		FPropertyPathDialogue->GetClass()->FindPropertyByName("Conditions");
		ConditionPropertyPath.GetRootProperty().Property.Get().set
		//ConditionPropertyPath
		*/
	}
}
