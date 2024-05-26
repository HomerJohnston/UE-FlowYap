#pragma once

#include "FlowYapFlowModes.generated.h"

UENUM(BlueprintType)
enum class EFlowYapFlowMode : uint8
{
	
	UserResponse,

	COUNT			UMETA(Hidden)
};

enum class EFlowYapMultiOutputFlowMode : uint8
{
	Random,
};
