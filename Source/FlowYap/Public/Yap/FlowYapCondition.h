#pragma once

#include "FlowYapCondition.generated.h"

UCLASS(Blueprintable, Abstract, DefaultToInstanced, EditInlineNew)
class FLOWYAP_API UFlowYapCondition : public UObject
{
	GENERATED_BODY()

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere)
	FString Description;
#endif

public:
	UFUNCTION(BlueprintNativeEvent)
	FString GetDescription() const;
	
	UFUNCTION(BlueprintNativeEvent)
	bool Evaluate() const;
};

inline FString UFlowYapCondition::GetDescription_Implementation() const
{
	return Description;
}

inline bool UFlowYapCondition::Evaluate_Implementation() const
{
	return true;
}
