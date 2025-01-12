// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once
#include "Interfaces/IPluginManager.h"

namespace Yap
{
	static FString GetPluginFolder()
	{
		static FString PluginDir;

		if (PluginDir.IsEmpty())
		{
			PluginDir = IPluginManager::Get().FindPlugin(TEXT("Yap"))->GetBaseDir();
		}

		return PluginDir;
	}

	static FString GetResourcesFolder()
	{
		return GetPluginFolder() / "Resources";
	}
}
