// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once
#include "ISettingsModule.h"
#include "YapDeveloperSettings.h"
#include "Interfaces/IPluginManager.h"

namespace Yap
{
	static void OpenDeveloperSettings()
	{
		if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
		{
			SettingsModule->ShowViewer("Project", "Yap", FName(UYapDeveloperSettings::StaticClass()->GetName()));
		}
	}
}
