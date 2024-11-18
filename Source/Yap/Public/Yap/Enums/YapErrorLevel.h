#pragma once

#include "YapErrorLevel.generated.h"

UENUM()
enum class EYapErrorLevel : uint8
{
	OK,
	Warning,
	Error,
};