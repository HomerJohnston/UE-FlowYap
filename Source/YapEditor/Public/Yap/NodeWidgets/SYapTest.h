﻿#pragma once

DECLARE_DELEGATE(FOnOpened)

class SYapPopupTest : public SMenuAnchor
{
public:
	SLATE_BEGIN_ARGS(SYapPopupTest)
		: _Content()
		{}

		SLATE_DEFAULT_SLOT(FArguments, Content)
		
		SLATE_NAMED_SLOT( FArguments, ButtonContent )

		SLATE_NAMED_SLOT( FArguments, MenuContent )

		SLATE_ARGUMENT( EHorizontalAlignment, HAlign )
		
		SLATE_ARGUMENT( EVerticalAlignment, VAlign )
		
		SLATE_EVENT( FOnIsOpenChanged, OnMenuOpenChanged )
		

	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	FReply OnClicked_Button();

	FOnOpened OnOpened;
	
	TWeakPtr<SWidget> ContentWidgetPtr;
	
	SHorizontalBox::FSlot* ButtonContentSlot;
};