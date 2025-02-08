// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once

enum class EYapLoadContext : uint8
{
	Sync, // Used during gameplay for immediate demand of the asset
	Async, // Used during gameplay for preloading assets
	AsyncEditorOnly, // Only used in the editor, avoids storing the asset in any UPROPERTY - just loads it
	DoNotLoad,
};