﻿#pragma once

#include "Widgets/SCompoundWidget.h"

class SActivationCounterWidget : public SCompoundWidget
{
public:
	SLATE_USER_ARGS( SActivationCounterWidget )
		: _FontHeight(8) {}

	SLATE_ATTRIBUTE(int32, ActivationCount)
	SLATE_ATTRIBUTE(int32, ActivationLimit)
	SLATE_ARGUMENT(int32, FontHeight)
	
	SLATE_END_ARGS()

	TAttribute<int32> ActivationCount;
	TAttribute<int32> ActivationLimit;
	int32 FontHeight;
	
	FText NumeratorText() const;
	FText DenominatorText() const;

	FSlateColor NumeratorColor() const;
	FSlateColor DenominatorColor() const;

	TSharedPtr<SEditableText> Denominator;
	
	virtual void Construct(const FArguments& InArgs, FOnTextCommitted OnTextCommitted);
};
