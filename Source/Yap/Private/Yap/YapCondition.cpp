// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "Yap/YapCondition.h"

#define LOCTEXT_NAMESPACE "Yap"

bool UYapCondition::EvaluateCondition_Implementation() const
{
	return true;
}

#if WITH_EDITOR
inline FString UYapCondition::GetTitle_Implementation() const
{
	return TitleOverride.Get(DefaultTitle);
}

inline FLinearColor UYapCondition::GetColor_Implementation() const
{
	return Color;
}

void UYapCondition::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	OnPropertyChanged.Broadcast(PropertyChangedEvent);
}
#endif

#undef LOCTEXT_NAMESPACE