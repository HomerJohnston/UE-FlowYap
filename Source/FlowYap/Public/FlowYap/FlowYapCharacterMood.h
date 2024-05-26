#pragma once

#include "FlowYapCharacterMood.generated.h"

UENUM()
enum class EFlowYapCharacterMood : uint8
{
	Calm,
	Angry,
	Sad,
	Happy,
	Surprised,
	Confused,
	
	COUNT			UMETA(Hidden)
};

ENUM_RANGE_BY_COUNT(EFlowYapCharacterMood, EFlowYapCharacterMood::COUNT);
