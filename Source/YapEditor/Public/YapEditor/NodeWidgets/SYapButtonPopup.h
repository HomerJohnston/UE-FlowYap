// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license.

#pragma once

#define LOCTEXT_NAMESPACE "YapEditor"

DECLARE_DELEGATE(FOnOpened)
DECLARE_DELEGATE_RetVal(TSharedRef<SWidget>, FMenuContentGetter)

class SYapButtonPopup : public SMenuAnchor
{
public:
	SLATE_BEGIN_ARGS(SYapButtonPopup)
	{}
		SLATE_NAMED_SLOT( FArguments, ButtonContent )

		SLATE_ARGUMENT( EHorizontalAlignment, HAlign )
		
		SLATE_ARGUMENT( EVerticalAlignment, VAlign )
		
		SLATE_EVENT( FOnIsOpenChanged, OnPopupOpenChanged )

		SLATE_ATTRIBUTE( FLinearColor, ButtonColor )		

		SLATE_ARGUMENT( FMenuContentGetter, PopupContentGetter )

		SLATE_ARGUMENT( EMenuPlacement, PopupPlacement )
		
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	FReply OnClicked_Button();

	FOnOpened OnOpened;
	
	TWeakPtr<SWidget> ContentWidgetPtr;

	TAttribute<FLinearColor> ButtonColor;

	TAttribute<TSharedPtr<SWidget>> MenuContentAttribute;
	
	SHorizontalBox::FSlot* ButtonContentSlot;

	TSharedPtr<SButton> Button;

	FMenuContentGetter MenuContentGetter;
	
	virtual void SetMenuContent(TSharedRef<SWidget> InMenuContent) override;

	FSlateColor ButtonColorAndOpacity() const;
};

#undef LOCTEXT_NAMESPACE