#pragma once

#include "FlowYapTextCalculator.generated.h"

UCLASS(Abstract)
class FLOWYAP_API UFlowYapTextCalculator : public UObject
{
	GENERATED_BODY()

public:
	virtual int16 CalculateWordCount(const FText& Text);
	
	virtual double CalculateTextTime(int16 WordCount);
};