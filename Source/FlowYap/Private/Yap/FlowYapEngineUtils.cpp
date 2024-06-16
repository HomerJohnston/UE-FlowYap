// Copyright Ghost Pepper Games, Inc. All Rights Reserved.

#include "Yap/FlowYapEngineUtils.h"

#include "Interfaces/IPluginManager.h"

#define LOCTEXT_NAMESPACE "FlowYap"

FString FFlowYapEngineUtils::GetFlowYapPluginDir()
{
	FString EnginePluginDir = FPaths::EnginePluginsDir() / TEXT("Runtime/FlowYap");
	if (FPaths::DirectoryExists(EnginePluginDir))
		return EnginePluginDir;

	FString ProjectPluginDir = FPaths::ProjectPluginsDir() / TEXT("Runtime/FlowYap");
	if (FPaths::DirectoryExists(ProjectPluginDir))
		return ProjectPluginDir;

	TSharedPtr<IPlugin> FlowYapPlugin = IPluginManager::Get().FindPlugin(TEXT("FlowYap"));
	FString PluginBaseDir = FlowYapPlugin.IsValid() ? FlowYapPlugin->GetBaseDir() : EnginePluginDir;
	if (FPaths::DirectoryExists(PluginBaseDir))
		return PluginBaseDir;

	return EnginePluginDir;
}

void FFlowYapEngineUtils::MakeTexture2DGrayscaleDestructive(UTexture2D* Texture)
{
	uint8* TextureData = static_cast<uint8*>(Texture->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE));

	int32 TextureWidth = Texture->GetSizeX();
	int32 TextureHeight = Texture->GetSizeY();

	for (int32 y = 0; y < TextureHeight; ++y)
	{
		for (int32 x = 0; x < TextureWidth; ++x)
		{
			int32 Index = (y * TextureWidth + x) * 4;

			uint8 GrayValue = static_cast<uint8>(0.3 * TextureData[Index] + 0.587 * TextureData[Index + 1] + 0.114 * TextureData[Index + 2]);
				
			TextureData[Index] = GrayValue;
			TextureData[Index + 1] = GrayValue;
			TextureData[Index + 2] = GrayValue;
		}
	}

	Texture->GetPlatformData()->Mips[0].BulkData.Unlock();
	Texture->UpdateResource();
}

#undef LOCTEXT_NAMESPACE