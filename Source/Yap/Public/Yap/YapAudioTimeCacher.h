// Copyright Ghost Pepper Games Inc., all rights reserved.

#pragma once

#include "YapLog.h"
#include "YapAudioTimeCacher.generated.h"

/**
 * Interface class for an object to cache audio times.
 * If you set an audio time caching class in project settings, Yap will use the CDO of that class to read dialogue audio assets and determine how long they are.
 */
UCLASS(Abstract)
class YAP_API UYapAudioTimeCacher : public UObject
{
	GENERATED_BODY()

public:
	virtual double GetAudioLengthInSeconds(const TSoftObjectPtr<UObject>& Object)
	{
		UE_LOG(LogYap, Warning, TEXT("No audio time cache class found in project settings! Cannot set audio time! You must inherit a class from %s and set it in project settings for this to work."), *UYapAudioTimeCacher::StaticClass()->GetFName().ToString());

		return -1.0;
	};
};