#pragma once

struct FLOWYAP_API FFlowYapEngineUtils
{	
	static FString GetFlowYapPluginDir();

	static void MakeTexture2DGrayscaleDestructive(UTexture2D* Texture);
};