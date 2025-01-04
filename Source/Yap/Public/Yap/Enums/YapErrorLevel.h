#pragma once

#include "YapErrorLevel.generated.h"

UENUM()
enum class EYapErrorLevel : uint8
{
	OK		UMETA(DisplayName = "Use Text Time"),
	Warning	UMETA(DisplayName = "Use Text Time (Warning)"),
	Error	UMETA(DisplayName = "Prevent Packaging"),
};