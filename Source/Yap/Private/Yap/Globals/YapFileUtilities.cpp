// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "Yap/Globals/YapFileUtilities.h"

#include "Interfaces/IPluginManager.h"

#define LOCTEXT_NAMESPACE "Yap"

FString Yap::FileUtilities::GetPluginFolder()
{
	static FString PluginDir;

	if (PluginDir.IsEmpty())
	{
		PluginDir = IPluginManager::Get().FindPlugin(TEXT("Yap"))->GetBaseDir();
	}

	return PluginDir;
}

FString Yap::FileUtilities::GetResourcesFolder()
{
	return GetPluginFolder() / "Resources";
}

FName Yap::FileUtilities::GetTagConfigFileName()
{
	return FName("YapGameplayTags.ini");
}

#undef LOCTEXT_NAMESPACE