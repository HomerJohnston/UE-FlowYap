#pragma once

#include "FlowYapFragmentSharedSettings.generated.h"

UENUM()
enum class EFlowYapTimedMode : uint8
{
	UseProjectDefaults,
	None,
	UseEnteredTime,
	AutomaticFromText,
	AutomaticFromAudio,
};

USTRUCT()
struct FFlowYapFragmentSharedSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	EFlowYapTimedMode TimedMode = EFlowYapTimedMode::AutomaticFromText;
	
	UPROPERTY(EditAnywhere, meta = (EditCondition="bIsTimed && TimeMode==EFlowYapTimedMode::UseEnteredTime", ClampMin = 0.0, UIMin = 0.0, UIMax = 30.0))
	double EnteredTime = 0.0;
	
	/** After each dialogue is finished being spoken, a brief extra pause can be inserted before moving onto the next node. */
	UPROPERTY(EditAnywhere, meta = (EditCondition="bIsTimed"))
	float EndPaddingTime = 0.5f;
	
	UPROPERTY(EditAnywhere, meta = (EditCondition="bIsTimed"))
	bool bUserInterruptible = true;
};