#pragma once

#include "FlowYapCondition.generated.h"

UCLASS(Blueprintable, Abstract, DefaultToInstanced, EditInlineNew)
class FLOWYAP_API UFlowYapCondition : public UObject
{
	GENERATED_BODY()

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditDefaultsOnly)
	FString DefaultDescription = "Unnamed Condition";

	UPROPERTY(EditInstanceOnly)
	TOptional<FString> DescriptionOverride;
	
	UPROPERTY(EditDefaultsOnly)
	FLinearColor DefaultNodeColor = FLinearColor(0.080, 0.200, 0.100, 1.0);

	UPROPERTY(EditInstanceOnly)
	TOptional<FLinearColor> NodeColorOverride;
#endif

public:
	UFUNCTION(BlueprintNativeEvent)
	FString GetDescription() const;

	UFUNCTION(BlueprintNativeEvent)
	FLinearColor GetNodeColor() const;
	
	UFUNCTION(BlueprintNativeEvent)
	bool Evaluate() const;
};

inline FString UFlowYapCondition::GetDescription_Implementation() const
{
	return DescriptionOverride.Get(DefaultDescription);
}

inline FLinearColor UFlowYapCondition::GetNodeColor_Implementation() const
{
	return NodeColorOverride.Get(DefaultNodeColor);
}

inline bool UFlowYapCondition::Evaluate_Implementation() const
{
	return true;
}
