#pragma once

#include "FlowNode_YapDialogue_Settings.generated.h"

class UFlowYapCharacter;

UENUM(BlueprintType)
enum class EFlowNode_YapDialogue_Settings : uint8
{
	Timed,
	UserInterruptible,
	MAX					UMETA(Hidden)
};