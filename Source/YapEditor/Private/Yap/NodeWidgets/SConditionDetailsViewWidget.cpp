#include "Yap/NodeWidgets/SConditionDetailsViewWidget.h"

#include "Widgets/SVirtualWindow.h"
#include "Yap/YapEditorStyle.h"

void SConditionDetailsViewWidget::Construct(const FArguments& InArgs)
{
	TSharedRef<SVirtualWindow> VirtualWindow = SNew(SVirtualWindow);

	FDetailsViewArgs Args;
	Args.bHideSelectionTip = true;
	Args.bLockable = false;
	Args.bShowOptions = false;
	Args.DefaultsOnlyVisibility = EEditDefaultsOnlyNodeVisibility::Show;
	
	FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	TSharedPtr<IDetailsView> DetailsWidget = PropertyEditorModule.CreateDetailView(Args);
	DetailsWidget->SetObject(InArgs._Condition, true);
	
	VirtualWindow->SetContent(
		SNew(SBorder)
		.Padding(4)
		.BorderImage(FYapEditorStyle::GetImageBrush(YapBrushes.Box_SolidLightGray_Rounded))
		[
			DetailsWidget.ToSharedRef()
		]
		);
	
	ChildSlot
	[
		VirtualWindow
	];
}
