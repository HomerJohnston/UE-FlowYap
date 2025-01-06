// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once

#include "YapMissingAudioBehavior.generated.h"

/**
 * Controls how Yap behaves when dialogue is set to use Audio Time, but audio is not set.
 */
UENUM()
enum class EYapMissingAudioBehavior : uint8
{
	UseTextTime			UMETA(ToolTip = "Silently fall back to using Text Time"),
	UseTextTime_Warn	UMETA(ToolTip = "Use Text Time, but write a warning log and highlight the node"),
	Error				UMETA(ToolTip = "Throw a fatal error warning, and prevent packaging"),
};