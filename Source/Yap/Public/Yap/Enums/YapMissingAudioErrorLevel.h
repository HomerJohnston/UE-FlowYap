// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once

#include "YapMissingAudioErrorLevel.generated.h"

/**
 * Controls how Yap behaves when dialogue is set to use Audio Time, but audio is not set.
 */
UENUM()
enum class EYapMissingAudioErrorLevel : uint8
{
	OK		UMETA(ToolTip = "Always use text time when audio is missing"),
	Warning	UMETA(ToolTip = "Use text time, but write a warning log and highlight the node yellow"),
	Error	UMETA(ToolTip = "Throw a fatal error, and attempt to prevent packaging"),
};