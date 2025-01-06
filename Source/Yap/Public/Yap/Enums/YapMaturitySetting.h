// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once

UENUM(BlueprintType)
enum class EYapMaturitySetting : uint8
{
	Unspecified UMETA(Hidden),
	ChildSafe,
	Mature,
};