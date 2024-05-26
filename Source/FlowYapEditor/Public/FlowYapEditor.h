#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

#include "AssetTypeCategories.h"

class IAssetTypeActions;
class IAssetTools;

class FFlowYapEditorModule : public IModuleInterface
{
private:
    /** Asset category, used for RMB context "Create Asset" menu */
    static EAssetTypeCategories::Type SkepsiAssetCategory;

    /** Store created Asset Type Actions here for unregistering on shutdown */
    TArray<TSharedPtr<IAssetTypeActions>> CreatedAssetTypeActions;

    static EAssetTypeCategories::Type FlowYapAssetCategory;

public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

    
    static void RegisterAssetCategories(IAssetTools& AssetTools);
    
    void RegisterAssetTypeActions(IAssetTools& AssetTools);

    void RegisterAssetTypeAction(IAssetTools& AssetTools, const TSharedRef<IAssetTypeActions> Action);
    
    static void UnregisterDetailCustomizations(FPropertyEditorModule& PropertyEditorModule);
	
    void UnregisterAssetTypeActions(IAssetTools& AssetTools);
    
    static EAssetTypeCategories::Type GetAssetCategory(); 
};
