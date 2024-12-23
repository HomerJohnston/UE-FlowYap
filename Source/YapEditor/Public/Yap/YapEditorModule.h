﻿#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "GPGEditorModuleBase.h"

#include "AssetTypeCategories.h"

class IAssetTypeActions;
class IAssetTools;

class FYapEditorModule : public IModuleInterface, public FGPGEditorModuleBase
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
};
