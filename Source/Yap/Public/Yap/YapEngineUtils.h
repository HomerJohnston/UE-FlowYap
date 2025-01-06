// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once

struct YAP_API FYapEngineUtils
{	
	static FString GetFlowYapPluginDir();

	static void MakeTexture2DGrayscaleDestructive(UTexture2D* Texture);
};