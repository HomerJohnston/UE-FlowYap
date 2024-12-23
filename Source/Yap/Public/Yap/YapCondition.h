#pragma once

#include "YapCondition.generated.h"

struct FPropertyChangedEvent;

UCLASS(Blueprintable, Abstract, DefaultToInstanced, EditInlineNew)
class YAP_API UYapCondition : public UObject
{
	GENERATED_BODY()

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditDefaultsOnly)
	FString DefaultTitle = "Unnamed Condition";

	UPROPERTY(EditInstanceOnly)
	TOptional<FString> TitleOverride;
	
	UPROPERTY(EditDefaultsOnly)
	FLinearColor Color = FLinearColor(0.080, 0.200, 0.100, 1.0);
#endif
	
public:
	UFUNCTION(BlueprintNativeEvent)
	bool EvaluateCondition() const;
	
#if WITH_EDITOR
public:
	UFUNCTION(BlueprintNativeEvent)
	FString GetTitle() const;

	UFUNCTION(BlueprintNativeEvent)
	FLinearColor GetColor() const;

	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

	TMulticastDelegate<void(FPropertyChangedEvent&)> OnPropertyChanged;
#endif
};
