// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license.

#pragma once

// ------------------------------------------------------------------------------------------------
/**
 * Controls whether an ongoing dialogue can be interrupted or not. See UYapSubsystem:: // TODO implement interruption
 */
UENUM()
enum class EYapDialogueSkippable : uint8
{
	Default,
	NotSkippable,
	Skippable,
	COUNT				UMETA(Hidden)
};