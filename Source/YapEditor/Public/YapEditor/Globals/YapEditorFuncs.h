﻿// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once

namespace Yap::EditorFuncs
{
	static void OpenDeveloperSettings();

	void OpenProjectSettings();

	void PostNotificationInfo_Warning(FText Title, FText Description, float Duration = 6.0f);

	bool SaveAsset(UObject* Asset);

	TArray<FAssetIdentifier> FindTagReferences(FName TagName);
}
