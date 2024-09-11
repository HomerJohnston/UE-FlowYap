#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "GPGEditorModuleBase.h"

#include "AssetTypeCategories.h"

class IAssetTypeActions;
class IAssetTools;

class FFlowYapEditorModule : public IModuleInterface, public FGPGEditorModuleBase
{
private:
    static EAssetTypeCategories::Type FlowYapAssetCategory;

public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
    
    static EAssetTypeCategories::Type GetAssetCategory();
};
