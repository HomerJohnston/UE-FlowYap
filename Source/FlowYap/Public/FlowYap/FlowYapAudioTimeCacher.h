#pragma once
#include "FlowYapLog.h"

#include "FlowYapAudioTimeCacher.generated.h"

UCLASS(Abstract)
class FLOWYAP_API UFlowYapAudioTimeCacher : public UObject
{
	GENERATED_BODY()

public:
	virtual double GetAudioLengthInSeconds(const TSoftObjectPtr<UObject>& Object)
	{
		UE_LOG(FlowYap, Warning, TEXT("No audio time cache class found in project settings! Cannot set audio time!"));
		return -1.0;
	};
};