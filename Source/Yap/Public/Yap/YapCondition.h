// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once

#include "YapCondition.generated.h"

struct FPropertyChangedEvent;

#define LOCTEXT_NAMESPACE "Yap"

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

#undef LOCTEXT_NAMESPACE