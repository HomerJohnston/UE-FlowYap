#pragma once

#include "YapCondition.generated.h"

struct FPropertyChangedEvent;

UCLASS(Blueprintable, Abstract, DefaultToInstanced, EditInlineNew)
class YAP_API UYapCondition : public UObject
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
	bool Evaluate() const;
	
#if WITH_EDITOR
public:
	UFUNCTION(BlueprintNativeEvent)
	FString GetDescription() const;

	UFUNCTION(BlueprintNativeEvent)
	FLinearColor GetNodeColor() const;

	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

	TMulticastDelegate<void(FPropertyChangedEvent&)> OnPropertyChanged;
#endif
};

