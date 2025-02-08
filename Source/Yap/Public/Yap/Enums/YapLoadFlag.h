// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once

enum class EYapLoadFlag : uint8
{
	Sync, // Only used during gameplay for immediate demand of the asset
	Async, // Only used during gameplay for preloading the asset
	AsyncEditorOnly, // Only used in the editor to avoid storing the asset in a handle
};