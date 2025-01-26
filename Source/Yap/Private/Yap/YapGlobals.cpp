// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

/*
#include "Yap/YapGlobals.h"

#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"

#define LOCTEXT_NAMESPACE "Yap"

FString Yap::GetPluginFolder()
{
	static FString PluginDir;

	if (PluginDir.IsEmpty())
	{
		PluginDir = IPluginManager::Get().FindPlugin(TEXT("Yap"))->GetBaseDir();
	}

	return PluginDir;
}

FString Yap::GetResourcesFolder()
{
	return GetPluginFolder() / "Resources";
}

#if WITH_EDITOR
void Yap::OpenProjectSettings()
{
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->ShowViewer("Project", "Yap", FName(UYapProjectSettings::StaticClass()->GetName()));
	}
}
#endif

#if WITH_EDITOR
void Yap::PostNotificationInfo_Warning(FText Title, FText Description, float Duration)
{
	FNotificationInfo NotificationInfo(Title);
	NotificationInfo.ExpireDuration = Duration;
	NotificationInfo.Image = FAppStyle::GetBrush("Icons.WarningWithColor");
	NotificationInfo.SubText = Description;
	FSlateNotificationManager::Get().AddNotification(NotificationInfo);
}
#endif

#if WITH_EDITOR
FName Yap::GetIniFile()
{
	return FName("YapGameplayTags.ini");
}
#endif

#undef LOCTEXT_NAMESPACE
*/