#include "Yap/NodeWidgets/SYapTest.h"

#include "Yap/YapEditorStyle.h"

void SYapTimeSettingsPopup::Construct(const FArguments& InArgs)
{
	ContentWidgetPtr = InArgs._MenuContent.Widget;

	SMenuAnchor::Construct(SMenuAnchor::FArguments()
	.Placement(MenuPlacement_CenteredAboveAnchor)
	.Method(EPopupMethod::UseCurrentWindow)
	.IsCollapsedByParent(true)
	.OnMenuOpenChanged(InArgs._OnMenuOpenChanged)
	//.MenuContent(InArgs._MenuContent.Widget)
	[
		SNew(SButton)
		.Cursor(EMouseCursor::Default)
		.ButtonStyle(FYapEditorStyle::Get(), YapStyles.ButtonStyle_TimeSettingOpener)
		.OnClicked(this, &SYapTimeSettingsPopup::OnClicked_Button)
		.ButtonColorAndOpacity(InArgs._ButtonColor)
		[
			InArgs._ButtonContent.Widget
		]
	]);

	SetMenuContent(InArgs._MenuContent.Widget);
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

void SYapTimeSettingsPopup::SetMenuContent(TSharedRef<SWidget> InMenuContent)
{
	WrappedContent = MenuContent =
		SNew(SBox)
		.Padding(8)
		[
			InMenuContent
		];
}

