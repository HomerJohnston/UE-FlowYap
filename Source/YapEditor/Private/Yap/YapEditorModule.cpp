// Copyright Ghost Pepper Games, Inc. All Rights Reserved.

#include "Yap/YapEditorModule.h"

#include "Yap/AssetFactory_YapCharacter.h"
#include "Yap/YapCharacter.h"
#include "Yap/YapProjectSettings.h"
#include "Yap/YapEditorStyle.h"
#include "Yap/DetailCustomizations/DetailCustomization_YapProjectSettings.h"
#include "Yap/DetailCustomizations/DetailCustomization_YapCharacter.h"
#include "Yap/NodeWidgets/GameplayTagFilteredStyle.h"

#define LOCTEXT_NAMESPACE "Yap"

void FYapEditorModule::StartupModule()
{
	AssetCategory = { "Yap", LOCTEXT("Yap", "Yap") };
	
	AssetTypeActions.Add(MakeShareable(new FAssetTypeActions_FlowYapCharacter()));
	
	DetailCustomizations.Add({UYapProjectSettings::StaticClass(), FOnGetDetailCustomizationInstance::CreateStatic(&FDetailCustomization_YapProjectSettings::MakeInstance)});
	DetailCustomizations.Add({UYapCharacter::StaticClass(), FOnGetDetailCustomizationInstance::CreateStatic(&FDetailCustomization_YapCharacter::MakeInstance)});
	
	//PropertyCustomizations.Add(*FFlowYapFragment::StaticStruct(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FPropertyCustomization_FlowYapFragment::MakeInstance));
	
	StartupModuleBase();

	// TODO fix this retardation into my style proper
	FGameplayTagFilteredStyle::Initialize();

	// Force the style to load (for some reason stuff is not initalized on the first call to this otherwise???
	FYapEditorStyle::Get();
}

void FYapEditorModule::ShutdownModule()
{
	ShutdownModuleBase();
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FYapEditorModule, FlowYapEditor)