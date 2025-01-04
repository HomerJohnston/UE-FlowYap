#include "Yap/NodeWidgets/SYapTest.h"

#include "Widgets/SCanvas.h"
#include "Yap/YapEditorStyle.h"

void SYapTimeSettingsPopup::Construct(const FArguments& InArgs)
{
	ContentWidgetPtr = InArgs._MenuContent.Widget;

	SMenuAnchor::Construct(SMenuAnchor::FArguments()
	.OnMenuOpenChanged(InArgs._OnMenuOpenChanged)
	.MenuContent(InArgs._MenuContent.Widget)
	.Placement(MenuPlacement_CenteredAboveAnchor)
	[
		SNew(SButton)
		.Cursor(EMouseCursor::Default)
		.OnClicked(this, &SYapTimeSettingsPopup::OnClicked_Button)
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

FReply SYapTimeSettingsPopup::OnClicked_Button()
{
	FReply ButtonReply = FReply::Handled();
	
	SetIsOpen(ShouldOpenDueToClick(), false);
	
	if (IsOpen())
	{
		(void)OnOpened.ExecuteIfBound();

		ButtonReply.SetUserFocus(MenuContent.ToSharedRef(), EFocusCause::SetDirectly);
	}
	
	return ButtonReply;
}

