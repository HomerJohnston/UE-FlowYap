#include "Yap/NodeWidgets/SYapTest.h"

#include "Widgets/SCanvas.h"
#include "Yap/YapEditorStyle.h"

void SYapPopupTest::Construct(const FArguments& InArgs)
{
	ContentWidgetPtr = InArgs._MenuContent.Widget;

	SMenuAnchor::Construct(SMenuAnchor::FArguments()
	.OnMenuOpenChanged(InArgs._OnMenuOpenChanged)
	.Placement(MenuPlacement_Center)
	[
		SNew(SButton)
		.Cursor(EMouseCursor::Default)
		.OnClicked(this, &SYapPopupTest::OnClicked_Button)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.Expose(ButtonContentSlot)
			.HAlign(InArgs._HAlign)
			.VAlign(InArgs._VAlign)
			[
				InArgs._ButtonContent.Widget
			]
		]
	]);
}

FReply SYapPopupTest::OnClicked_Button()
{
	SetIsOpen(ShouldOpenDueToClick(), false);
	
	if (IsOpen())
	{
		(void)OnOpened.ExecuteIfBound();
	}
	
	return FReply::Handled();
}
