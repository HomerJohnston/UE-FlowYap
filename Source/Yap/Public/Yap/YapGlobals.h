// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once
#include "ISettingsModule.h"
#include "YapProjectSettings.h"
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

	static void OpenProjectSettings()
	{
		if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
		{
			SettingsModule->ShowViewer("Project", "Yap", FName(UYapProjectSettings::StaticClass()->GetName()));
		}
	}
}
