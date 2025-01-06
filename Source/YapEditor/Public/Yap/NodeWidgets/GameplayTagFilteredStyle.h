// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once

#include "Styling/SlateStyle.h"

class FGameplayTagFilteredStyle
{
public:
	static void Initialize();
	static void Shutdown();
	static ISlateStyle& Get() { return *StyleSet.Get(); }
	static FName GetStyleSetName();

	static const FSlateBrush* GetBrush(FName PropertyName, const ANSICHAR* Specifier = nullptr)
	{
		return StyleSet->GetBrush(PropertyName, Specifier);
	}

	static const FLinearColor& GetColor(FName PropertyName, const ANSICHAR* Specifier = nullptr)
	{
		return StyleSet->GetColor(PropertyName, Specifier);
	}

private:
	static TSharedPtr<FSlateStyleSet> StyleSet;
};
