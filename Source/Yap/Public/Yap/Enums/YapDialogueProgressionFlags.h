// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license.

#pragma once

// ------------------------------------------------------------------------------------------------
/**
 * Controls whether an ongoing dialogue can be interrupted or not. See UYapSubsystem:: // TODO implement interruption
 */

/*
UENUM(BlueprintType, meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EYapDialogueProgressionFlags : uint8
{
	NONE					UMETA(Hidden, DisplayName = "Not skippable, manual advance only"),
	Skippable	= 1 << 0	UMETA(ToolTip = "Dialogue can be skipped at any time, and will automatically advance when its duration completes."),
	AutoAdvance	= 1 << 1	UMETA(ToolTip = "Dialogue can be skipped at any time, and requires skipping via the Dialogue Handle to advance."),
};
ENUM_CLASS_FLAGS(EYapDialogueProgressionFlags)
*/