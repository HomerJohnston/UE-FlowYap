// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license.

#pragma once

#define LOCTEXT_NAMESPACE "YapEditor"

DECLARE_DELEGATE(FOnOpened)

class SYapTimeSettingsPopup : public SMenuAnchor
{
public:
	SLATE_BEGIN_ARGS(SYapTimeSettingsPopup)
		: _Content()
		{}

		SLATE_DEFAULT_SLOT(FArguments, Content)
		
		SLATE_NAMED_SLOT( FArguments, ButtonContent )

		SLATE_NAMED_SLOT( FArguments, MenuContent )

		SLATE_ARGUMENT( EHorizontalAlignment, HAlign )
		
		SLATE_ARGUMENT( EVerticalAlignment, VAlign )
		
		SLATE_EVENT( FOnIsOpenChanged, OnMenuOpenChanged )

		SLATE_ATTRIBUTE( FLinearColor, ButtonColor )		

	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	FReply OnClicked_Button();

	FOnOpened OnOpened;
	
	TWeakPtr<SWidget> ContentWidgetPtr;

	TAttribute<FLinearColor> ButtonColor;
	
	SHorizontalBox::FSlot* ButtonContentSlot;

	TSharedPtr<SButton> Button;

	virtual void SetMenuContent(TSharedRef<SWidget> InMenuContent) override;

	FSlateColor ButtonColorAndOpacity() const;
};

#undef LOCTEXT_NAMESPACE