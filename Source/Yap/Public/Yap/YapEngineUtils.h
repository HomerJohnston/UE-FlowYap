#pragma once

struct YAP_API FYapEngineUtils
{	
	static FString GetFlowYapPluginDir();

	static void MakeTexture2DGrayscaleDestructive(UTexture2D* Texture);
};