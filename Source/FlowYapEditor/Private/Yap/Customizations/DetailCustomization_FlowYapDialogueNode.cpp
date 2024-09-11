#include "Yap/Customizations/DetailCustomization_FlowYapDialogueNode.h"

#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Yap/Nodes/FlowNode_YapDialogue.h"
#include "PropertyCustomizationHelpers.h"
#include "Yap/DetailsWidgets/SFlowYapBitDetailsWidget.h"

void FDetailCustomization_FlowYapDialogueNode::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	UE_LOG(LogTemp, Display, TEXT("Hello world!"));

	TArray<TWeakObjectPtr<UFlowNode_YapDialogue>> DialoguesBeingCustomized = DetailBuilder.GetObjectsOfTypeBeingCustomized<UFlowNode_YapDialogue>();

	if (DialoguesBeingCustomized.Num() != 1)
	{
		return;
	}

	Dialogue = DialoguesBeingCustomized[0];
	
	TArray<FName> Categories;
	DetailBuilder.GetCategoryNames(Categories);
	
	IDetailCategoryBuilder& TestCategory = DetailBuilder.EditCategory("Test", INVTEXT("Test"));

	TSharedPtr<IPropertyHandle> FragmentPropertyHandle;
	FragmentPropertyHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UFlowNode_YapDialogue, Fragments));

	//const TSharedRef<FDetailArrayBuilder> ArrayBuilder = MakeShareable(new FDetailArrayBuilder(FragmentPropertyHandle.ToSharedRef()));
	//ArrayBuilder->OnGenerateArrayElementWidget(FOnGenerateArrayElementWidget::CreateSP(this, &FDetailCustomization_FlowYapDialogueNode::GenerateFragmentArray));
	
	//TestCategory.AddCustomBuilder(ArrayBuilder);
}

void FDetailCustomization_FlowYapDialogueNode::GenerateFragmentArray(TSharedRef<IPropertyHandle> PropertyHandle, int32 ArrayIndex, IDetailChildrenBuilder& ChildrenBuilder)
{
	TSharedPtr<IPropertyHandle> Bit = PropertyHandle->GetChildHandle("Bit");

	TSharedPtr<IPropertyHandle> Text = Bit->GetChildHandle("DialogueText");

	FText TextValue;
	Text->GetValueAsDisplayText(TextValue);

	/*
	IDetailPropertyRow& PropertyRow = ChildrenBuilder.AddProperty(PropertyHandle);
	PropertyRow.ShowPropertyButtons(true);
	PropertyRow.ShouldAutoExpand(true);
	PropertyRow.CustomWidget(false)
	.ValueContent()
	[
		SNew(SFlowYapBitDetailsWidget, PropertyHandle.ToSharedPtr(), Dialogue)
	];
	*/
}
