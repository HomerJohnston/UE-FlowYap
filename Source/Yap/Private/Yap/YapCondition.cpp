// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "Yap/YapCondition.h"

#define LOCTEXT_NAMESPACE "Yap"

// ------------------------------------------------------------------------------------------------

bool UYapCondition::EvaluateCondition_Implementation() const
{
	return true;
}

// ------------------------------------------------------------------------------------------------

#if WITH_EDITOR
inline FText UYapCondition::GetTitle_Implementation() const
{
	if (TitleOverride.IsSet())
	{
		return FText::FromString(TitleOverride.GetValue());
	}
	
	return DefaultTitle;
}
#endif

// ------------------------------------------------------------------------------------------------

#if WITH_EDITOR
inline FLinearColor UYapCondition::GetColor_Implementation() const
{
	return Color;
}
#endif

// ------------------------------------------------------------------------------------------------

#undef LOCTEXT_NAMESPACE
bool UYapCondition::EvaluateCondition_Internal()
{
#if WITH_EDITOR
	bool Value = EvaluateCondition();
	LastEvaluation = Value;
	return Value;
#else
	return EvaluateCondition();
#endif
}
