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