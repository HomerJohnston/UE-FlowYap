// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once

#include "YapTimeMode.generated.h"

UENUM()
enum class EYapTimeMode : uint8
{
	/** Use project default setting */
	Default UMETA(Hidden),
	/** Dialogue can only be progressed by the game returning the dialogue handle to the FlowYapSubsystem */
	None,
	/** Dialogue will automatically progress after an automatically determined time, determined by the selected audio asset */
	AudioTime,
	/** Dialogue will automatically progress after an automatically determined time, see project settings for Words Per Minute */
	TextTime,
	/** Dialogue will automatically progress after a set time */
	ManualTime,
};
