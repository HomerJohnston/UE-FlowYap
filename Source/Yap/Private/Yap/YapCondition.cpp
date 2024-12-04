#include "Yap/YapCondition.h"

inline bool UYapCondition::Evaluate_Implementation() const
{
	return true;
}

#if WITH_EDITOR
inline FString UYapCondition::GetDescription_Implementation() const
{
	return DescriptionOverride.Get(DefaultDescription);
}

inline FLinearColor UYapCondition::GetNodeColor_Implementation() const
{
	return NodeColorOverride.Get(DefaultNodeColor);
}

void UYapCondition::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	OnPropertyChanged.Broadcast(PropertyChangedEvent);
}
#endif
