﻿#include "Yap/YapCondition.h"

inline bool UYapCondition::EvaluateCondition_Implementation() const
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
