// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once
#if WITH_EDITOR
#include "ISettingsModule.h"
#endif
#include "YapProjectSettings.h"
#include "Interfaces/IPluginManager.h"

namespace Yap
{
	YAP_API FString GetPluginFolder();

	YAP_API FString GetResourcesFolder();

#if WITH_EDITOR
	YAP_API void OpenProjectSettings();

	YAP_API void PostNotificationInfo_Warning(FText Title, FText Description, float Duration = 6.0f);
#endif

}
