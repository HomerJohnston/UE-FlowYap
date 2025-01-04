#pragma once

#include "YapTimeMode.generated.h"

UENUM()
enum class EYapTimeMode : uint8
{
	/** Dialogue can only be progressed by the game returning the dialogue handle to the FlowYapSubsystem */
	None,
	/** Dialogue will automatically progress after an automatically determined time, determined by the selected audio asset */
	AudioTime,
	/** Dialogue will automatically progress after an automatically determined time, see project settings for Words Per Minute */
	TextTime,
	/** Dialogue will automatically progress after a set time */
	ManualTime,
};
