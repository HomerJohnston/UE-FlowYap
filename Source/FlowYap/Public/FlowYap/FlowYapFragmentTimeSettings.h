#pragma once

#include "FlowYapFragmentTimeSettings.generated.h"

UENUM()
enum class EFlowYapTimedMode : uint8
{
	None,
	UseEnteredTime,
	AutomaticFromText,
	AutomaticFromAudio,
};

USTRUCT()
struct FFlowYapFragmentTimeSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	EFlowYapTimedMode TimedMode = EFlowYapTimedMode::AutomaticFromText;
	
	UPROPERTY(EditAnywhere, meta = (EditCondition="TimedMode==EFlowYapTimedMode::UseEnteredTime", ClampMin = 0.0, UIMin = 0.0, UIMax = 30.0))
	double EnteredTime = 0.0;
	
	/** After each dialogue is finished being spoken, a brief extra pause can be inserted before moving onto the next node. */
	UPROPERTY(EditAnywhere, meta = (EditCondition="TimedMode!=EFlowYapTimedMode::None"))
	float EndPaddingTime = 0.5f;
	
	UPROPERTY(EditAnywhere, meta = (EditCondition="TimedMode!=EFlowYapTimedMode::None"))
	bool bUserInterruptible = true;
};