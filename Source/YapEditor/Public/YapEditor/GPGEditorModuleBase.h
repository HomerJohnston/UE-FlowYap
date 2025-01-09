// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

// This is a drop-in header to assist with basic module definition, customization registration/deregistration, etc.
// It is intended to be copy/pasted in any module that wishes to use it.
// For usage, see comments in SETTINGS section below.

#pragma once

#include "AssetToolsModule.h"
#include "AssetTypeCategories.h"

class IAssetTypeActions;
class IAssetTools;

class FGPGEditorModuleBase
{
	// ============================================================================================
	// SETTINGS
	/* Assign info to any of these four variables in your StartupModule(), and then call StartupModuleBase(). Call ShutdownModuleBase() in your ShutdownModule(). */
protected:
	TPair<FName, FText> AssetCategory;
	TArray<TSharedRef<IAssetTypeActions>> AssetTypeActions;
	TArray<TPair<TSubclassOf<UObject>, const FOnGetDetailCustomizationInstance>> DetailCustomizations;
	TArray<TPair<const UScriptStruct&, const FOnGetPropertyTypeCustomizationInstance>> PropertyCustomizations;
	
	/* Example, this code would be in StartupModule():
	// --------
	AssetCategory = { "Yap", LOCTEXT("Yap", "Yap") };

	AssetTypeActions.Add(MakeShareable(new FAssetTypeActions_Type()));
	AssetTypeActions.Add(MakeShareable(new FAssetTypeActions_Type()));
	
	DetailCustomizations.Add(UClassType::StaticClass(), FOnGetDetailCustomizationInstance::CreateStatic(&FDetailCustomization_ClassType::MakeInstance));
	DetailCustomizations.Add(UClassType::StaticClass(), FOnGetDetailCustomizationInstance::CreateStatic(&FDetailCustomization_ClassType::MakeInstance));
	
	PropertyCustomizations.Add(*FStructType::StaticStruct(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FPropertyCustomization_StructType::MakeInstance));
	PropertyCustomizations.Add(*UClassType::StaticStruct(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FPropertyCustomization_ClassType::MakeInstance));

	StartupModuleBase();
	// --------
	*/

	// ============================================================================================
	// STATE
private:
	TArray<TSharedRef<IAssetTypeActions>> RegisteredAssetTypeActions;
	TSet<FName> RegisteredCustomClassLayouts;
	TSet<FName> RegisteredCustomStructLayouts;
	static inline EAssetTypeCategories::Type RegisteredAssetCategory;

public: 
    static EAssetTypeCategories::Type GetAssetCategory()
    {
	    return RegisteredAssetCategory;
    };
	
protected:
	void StartupModuleBase()
	{
		if (AssetCategory.Key != NAME_None && !AssetCategory.Value.IsEmpty())
		{
			RegisteredAssetCategory = RegisterAssetCategory(AssetCategory.Key, AssetCategory.Value);
		}
	
		RegisterAssetTypeActions();
		RegisterDetailCustomizations();
		RegisterPropertyCustomizations();
	}
	
	void ShutdownModuleBase()
	{
		UnregisterAssetTypeActions();
		UnregisterCustomizations();
	}
	
private:
	// ------------------------------------------
	EAssetTypeCategories::Type RegisterAssetCategory(FName Name, FText Text)
	{
		if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
		{
			IAssetTools& AssetTools = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get();
			return AssetTools.RegisterAdvancedAssetCategory(Name, Text);
		}

		return EAssetTypeCategories::None;
	}
	
private:
	// ------------------------------------------
	void RegisterAssetTypeActions()
	{
		if (FAssetToolsModule* AssetToolsModule = FModuleManager::GetModulePtr<FAssetToolsModule>("AssetTools"))
		{
			IAssetTools& AssetTools = AssetToolsModule->Get();

			for (TSharedRef<IAssetTypeActions> Action : AssetTypeActions)
			{
				RegisterAssetTypeAction(AssetTools, Action);
			}
		}
	}

	// ------------------------------------------
	void RegisterDetailCustomizations()
	{
		if (FPropertyEditorModule* PropertyEditorModule = FModuleManager::GetModulePtr<FPropertyEditorModule>("PropertyEditor"))
		{
			for (TPair<TSubclassOf<UObject>, const FOnGetDetailCustomizationInstance>& Customization : DetailCustomizations)
			{
				RegisterCustomClassLayout(Customization.Key, Customization.Value);
			}
		
			PropertyEditorModule->NotifyCustomizationModuleChanged();
		}
	}

	// ------------------------------------------
	void RegisterPropertyCustomizations()
	{
		if (FPropertyEditorModule* PropertyEditorModule = FModuleManager::GetModulePtr<FPropertyEditorModule>("PropertyEditor"))
		{
			for (TPair<const UScriptStruct&, const FOnGetPropertyTypeCustomizationInstance>& Customization : PropertyCustomizations)
			{
				RegisterCustomStructLayout(Customization.Key, Customization.Value);
			}
		
			PropertyEditorModule->NotifyCustomizationModuleChanged();
		}
	}

private:
	// ------------------------------------------
	void RegisterAssetTypeAction(IAssetTools& AssetTools, const TSharedRef<IAssetTypeActions> Action)
	{
		AssetTools.RegisterAssetTypeActions(Action);
		RegisteredAssetTypeActions.Add(Action);
	}

	// ------------------------------------------
	void RegisterCustomClassLayout(const TSubclassOf<UObject> Class, const FOnGetDetailCustomizationInstance DetailLayout)
	{
		if (!Class)
		{
			return;
		}

		if (FPropertyEditorModule* PropertyEditorModule = FModuleManager::GetModulePtr<FPropertyEditorModule>("PropertyEditor"))
		{
			PropertyEditorModule->RegisterCustomClassLayout(Class->GetFName(), DetailLayout);
			RegisteredCustomClassLayouts.Add(Class->GetFName());
		}
	}

	// ------------------------------------------
	void RegisterCustomStructLayout(const UScriptStruct& Struct, const FOnGetPropertyTypeCustomizationInstance DetailLayout)
	{
		if (FPropertyEditorModule* PropertyEditorModule = FModuleManager::GetModulePtr<FPropertyEditorModule>("PropertyEditor"))
		{
			PropertyEditorModule->RegisterCustomPropertyTypeLayout(Struct.GetFName(), DetailLayout);
			RegisteredCustomStructLayouts.Add(Struct.GetFName());
		}
	}

private:
	// ------------------------------------------
	void UnregisterAssetTypeActions()
	{
		if (FAssetToolsModule* AssetToolsModule = FModuleManager::GetModulePtr<FAssetToolsModule>("AssetTools"))
		{
			IAssetTools& AssetTools = AssetToolsModule->Get();

			for (const TSharedRef<IAssetTypeActions>& Action : RegisteredAssetTypeActions)
			{
				AssetTools.UnregisterAssetTypeActions(Action);
			}
		}
	
		RegisteredAssetTypeActions.Empty();
	}

	// ------------------------------------------
	void UnregisterCustomizations()
	{
		// unregister details customizations
		if (FPropertyEditorModule* PropertyEditorModule = FModuleManager::GetModulePtr<FPropertyEditorModule>("PropertyEditor"))
		{
			for (auto It = RegisteredCustomClassLayouts.CreateConstIterator(); It; ++It)
			{
				if (It->IsValid())
				{
					PropertyEditorModule->UnregisterCustomClassLayout(*It);
				}
			}

			for (auto It = RegisteredCustomStructLayouts.CreateConstIterator(); It; ++It)
			{
				if (It->IsValid())
				{
					PropertyEditorModule->UnregisterCustomPropertyTypeLayout(*It);
				}
			}

			PropertyEditorModule->NotifyCustomizationModuleChanged();
		}
	}
};

