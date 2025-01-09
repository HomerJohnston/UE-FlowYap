// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license.

#include "YapEditor/NodeWidgets/SYapTest.h"

#include "YapEditor/YapColors.h"
#include "YapEditor/YapEditorStyle.h"

#define LOCTEXT_NAMESPACE "YapEditor"

void SYapTimeSettingsPopup::Construct(const FArguments& InArgs)
{
	ContentWidgetPtr = InArgs._MenuContent.Widget;

	ButtonColor = InArgs._ButtonColor;
	
	SMenuAnchor::Construct(SMenuAnchor::FArguments()
	.Placement(MenuPlacement_CenteredAboveAnchor)
	.Method(EPopupMethod::UseCurrentWindow)
	.IsCollapsedByParent(true)
	.OnMenuOpenChanged(InArgs._OnMenuOpenChanged)
	[
		SAssignNew(Button, SButton)
		.Cursor(EMouseCursor::Default)
		.ButtonStyle(FYapEditorStyle::Get(), YapStyles.ButtonStyle_TimeSettingOpener)
		.OnClicked(this, &SYapTimeSettingsPopup::OnClicked_Button)
		.ForegroundColor(this, &SYapTimeSettingsPopup::ButtonColorAndOpacity)
		.ContentPadding(0)
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

FSlateColor SYapTimeSettingsPopup::ButtonColorAndOpacity() const
{
	FLinearColor Col = ButtonColor.Get();

	if (!Button->IsHovered())
	{
		Col *= YapColor::LightGray;
	}
	else if (Button->IsPressed())
	{
		Col *= YapColor::LightGray;
	}

	return Col;
}

#undef LOCTEXT_NAMESPACE