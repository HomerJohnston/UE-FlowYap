#pragma once

#include "YapTextCalculator.generated.h"

UCLASS(Abstract)
class YAP_API UYapTextCalculator : public UObject
{
	GENERATED_BODY()

public:
	virtual int32 CalculateWordCount(const FText& Text);
	
	virtual double CalculateTextTime(int16 WordCount);
};