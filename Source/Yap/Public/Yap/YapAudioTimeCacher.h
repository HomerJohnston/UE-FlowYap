#pragma once
#include "YapLog.h"

#include "YapAudioTimeCacher.generated.h"

UCLASS(Abstract)
class YAP_API UYapAudioTimeCacher : public UObject
{
	GENERATED_BODY()

public:
	virtual double GetAudioLengthInSeconds(const TSoftObjectPtr<UObject>& Object)
	{
		UE_LOG(FlowYap, Warning, TEXT("No audio time cache class found in project settings! Cannot set audio time! You must inherit from "));
		return -1.0;
	};
};