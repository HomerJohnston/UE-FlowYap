﻿// Copyright Ghost Pepper Games, Inc. All Rights Reserved.

#include "Yap/FlowYapEditor.h"

#include "AssetToolsModule.h"
#include "Yap/AssetFactory_FlowYapCharacter.h"
#include "Yap/NodeWidgets/GameplayTagFilteredStyle.h"

#define LOCTEXT_NAMESPACE "FlowYap"

EAssetTypeCategories::Type FFlowYapEditorModule::FlowYapAssetCategory;

#pragma region REGISTRATION_MACROS
#define REGISTER_PROP_CUSTOMIZATION(_MODULE_, _NAME_) _MODULE_##.RegisterCustomPropertyTypeLayout("FlowYap" ###_NAME_##, FOnGetPropertyTypeCustomizationInstance::CreateStatic(FPropertyCustomization_FlowYap ## _NAME_ ## ::MakeInstance))
#define REGISTER_CLASS_CUSTOMIZATION(_MODULE_, _NAME_) _MODULE_##.RegisterCustomClassLayout("FlowYap" ###_NAME_##, FOnGetDetailCustomizationInstance::CreateStatic(&FDetailCustomization_FlowYap ##_NAME_## ::MakeInstance))
#define REGISTER_ASSET_TYPE_ACTION(_MODULE_, _NAME_) RegisterAssetTypeAction(##_MODULE_##, MakeShareable(new FAssetTypeActions_FlowYap ##_NAME_##()))
#define UNREGISTER_PROP_CUSTOMIZATION(_MODULE_, _NAME_) _MODULE_##.UnregisterCustomPropertyTypeLayout("FlowYap" ###_NAME_##)
#define UNREGISTER_CLASS_CUSTOMIZATION(_MODULE_, _NAME_) _MODULE_##.UnregisterCustomClassLayout("FlowYap" ###_NAME_##)
#pragma endregion

void FFlowYapEditorModule::EnableMultipleInputs()
{
	UE_LOG(LogTemp, Warning, TEXT("Enable Multiple Inputs Command"));
}

void FFlowYapEditorModule::StartupModule()
{
	if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
	{
		IAssetTools& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
		RegisterAssetCategories(AssetToolsModule);
		RegisterAssetTypeActions(AssetToolsModule);
	}

	FGameplayTagFilteredStyle::Initialize();
}

void FFlowYapEditorModule::ShutdownModule()
{
	if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
	{
		IAssetTools& AssetTools = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get();
		UnregisterAssetTypeActions(AssetTools);
	}

	if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
	{
		FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
		UnregisterDetailCustomizations(PropertyEditorModule);
	}
}

void FFlowYapEditorModule::RegisterAssetCategories(IAssetTools& AssetTools)
{
	FlowYapAssetCategory = AssetTools.RegisterAdvancedAssetCategory(FName(TEXT("FlowYap")), LOCTEXT("FlowYap", "FlowYap"));
}

void FFlowYapEditorModule::RegisterAssetTypeActions(IAssetTools& AssetTools)
{
	// TODO
	// 0>FlowYapEditor.cpp(60): Warning C5103 : pasting '(' and 'AssetTools' does not result in a valid preprocessing token
    // 0>FlowYapEditor.cpp(16): Reference  : in expansion of macro 'REGISTER_ASSET_TYPE_ACTION'
	REGISTER_ASSET_TYPE_ACTION(AssetTools, Character);
}

void FFlowYapEditorModule::RegisterAssetTypeAction(IAssetTools& AssetTools, const TSharedRef<IAssetTypeActions> Action)
{
	AssetTools.RegisterAssetTypeActions(Action);
	CreatedAssetTypeActions.Add(Action);
}

void FFlowYapEditorModule::UnregisterDetailCustomizations(FPropertyEditorModule& PropertyEditorModule)
{
	PropertyEditorModule.NotifyCustomizationModuleChanged();
}

void FFlowYapEditorModule::UnregisterAssetTypeActions(IAssetTools& AssetTools)
{
	for (int32 i = 0; i < CreatedAssetTypeActions.Num(); i++)
		AssetTools.UnregisterAssetTypeActions(CreatedAssetTypeActions[i].ToSharedRef());

	CreatedAssetTypeActions.Empty();
}

EAssetTypeCategories::Type FFlowYapEditorModule::GetAssetCategory()
{
	return FlowYapAssetCategory;
}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FFlowYapEditorModule, FlowYapEditor)