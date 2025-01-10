// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license.

#include "YapEditor/NodeWidgets/SYapButtonPopup.h"

#include "YapEditor/YapColors.h"
#include "YapEditor/YapEditorStyle.h"

#define LOCTEXT_NAMESPACE "YapEditor"

void SYapButtonPopup::Construct(const FArguments& InArgs)
{
	ButtonColor = InArgs._ButtonColor;

	MenuContentGetter = InArgs._PopupContentGetter;
	
	SMenuAnchor::Construct(SMenuAnchor::FArguments()
	.Placement(InArgs._PopupPlacement)
	.Method(EPopupMethod::UseCurrentWindow)
	.IsCollapsedByParent(true)
	.OnMenuOpenChanged(InArgs._OnPopupOpenChanged)
	[
		SAssignNew(Button, SButton)
		.Cursor(EMouseCursor::Default)
		.ButtonStyle(FYapEditorStyle::Get(), YapStyles.ButtonStyle_TimeSettingOpener)
		.OnClicked(this, &SYapButtonPopup::OnClicked_Button)
		.ForegroundColor(this, &SYapButtonPopup::ButtonColorAndOpacity)
		.ContentPadding(0)
		[
			InArgs._ButtonContent.Widget
		]
	]);
}

FReply SYapButtonPopup::OnClicked_Button()
{
	FReply ButtonReply = FReply::Handled();

	if (!MenuContentGetter.IsBound())
	{
		return ButtonReply;
	}
	
	SetMenuContent(MenuContentGetter.Execute());
	
	SetIsOpen(ShouldOpenDueToClick(), false);
	
	if (IsOpen())
	{
		(void)OnOpened.ExecuteIfBound();

		ButtonReply.SetUserFocus(MenuContent.ToSharedRef(), EFocusCause::SetDirectly);
	}

	return ButtonReply;
}

void SYapButtonPopup::SetMenuContent(TSharedRef<SWidget> InMenuContent)
{
	WrappedContent = MenuContent =
		SNew(SBox)
		.Padding(8)
		[
			InMenuContent
		];
}

FSlateColor SYapButtonPopup::ButtonColorAndOpacity() const
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