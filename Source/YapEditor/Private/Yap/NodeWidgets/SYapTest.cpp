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
	.Content()
	[
		SNew(SButton)
		.Cursor(EMouseCursor::Default)
		.ButtonStyle(FYapEditorStyle::Get(), YapStyles.ButtonStyle_TimeSetting)
		.OnClicked(this, &SYapTimeSettingsPopup::OnClicked_Button)
		.ForegroundColor(YapColor::Green)
		.ButtonColorAndOpacity(InArgs._ButtonColor)
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

