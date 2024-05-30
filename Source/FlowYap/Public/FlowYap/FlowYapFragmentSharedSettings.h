#pragma once

#include "FlowYapFragmentSharedSettings.generated.h"

USTRUCT()
struct FFlowYapFragmentSharedSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	bool bIsTimed = true;

	UPROPERTY(EditAnywhere, meta = (EditCondition="bIsTimed", ClampMin = 0.0, UIMin = 0.0, UIMax = 30.0))
	double TimeManual = 0.0;
	
	UPROPERTY(EditAnywhere, meta = (EditCondition="bIsTimed"))
	bool bUseAutoTime = true;
	
	UPROPERTY(EditAnywhere, meta = (EditCondition="bIsTimed"))
	bool bUseAudioLength = false;
	
	/** After each dialogue is finished being spoken, a brief extra pause can be inserted before moving onto the next node. */
	UPROPERTY(EditAnywhere, meta = (EditCondition="bIsTimed"))
	float EndPaddingTime = 0.5f;
	
	UPROPERTY(EditAnywhere, meta = (EditCondition="bIsTimed"))
	bool bUserInterruptible = true;
};