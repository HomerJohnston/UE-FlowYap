#pragma once

#include "FlowYapErrorLevel.generated.h"

UENUM()
enum class EFlowYapErrorLevel : uint8
{
	OK,
	Warning,
	Error,
};