#include "Yap/NodeWidgets/SConditionDetailsViewWidget.h"

#include "ISinglePropertyView.h"
#include "PropertyCustomizationHelpers.h"
#include "Widgets/SVirtualWindow.h"
#include "Yap/YapEditorStyle.h"
#include "Yap/Nodes/FlowNode_YapDialogue.h"

void SConditionDetailsViewWidget::Construct(const FArguments& InArgs)
{
	TSharedRef<SVirtualWindow> VirtualWindow = SNew(SVirtualWindow);

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

	UFlowNode_YapDialogue* DialogueNode = Cast<UFlowNode_YapDialogue>(InArgs._Condition->GetOuter());
	
	FSinglePropertyParams Params;
	Params.NamePlacement = EPropertyNamePlacement::Hidden;
	TSharedPtr<ISinglePropertyView> SinglePropertyViewWidget = PropertyEditorModule.CreateSingleProperty(DialogueNode, "Conditions", Params);
	
	VirtualWindow->SetContent(
		SNew(SBorder)
		.Padding(1, -3, 1, 1) // No idea why but the details view already has a 4 pixel transparent area on top
		.BorderImage(FYapEditorStyle::GetImageBrush(YapBrushes.Box_SolidLightGray_Rounded))
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 8, 0, 4)
			[
				// SPropertyEditorInline is private, fucking epic
				SNew(SSpacer)
				.Size(16)//SinglePropertyViewWidget.ToSharedRef()	
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				DetailsWidget.ToSharedRef()
			]
		]
		);
	
	ChildSlot
	[
		VirtualWindow
	];
}
