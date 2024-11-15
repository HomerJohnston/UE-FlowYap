// Copyright Ghost Pepper Games, Inc. All Rights Reserved.

#include "Yap/FlowYapEditor.h"

#include "AssetToolsModule.h"
#include "GameplayTagsManager.h"
#include "Yap/AssetFactory_FlowYapCharacter.h"
#include "Yap/Nodes/FlowNode_YapDialogue.h"
#include "Yap/NodeWidgets/GameplayTagFilteredStyle.h"

#define LOCTEXT_NAMESPACE "FlowYap"

#pragma region REGISTRATION_MACROS
#define REGISTER_PROP_CUSTOMIZATION(_MODULE_, _NAME_) _MODULE_##.RegisterCustomPropertyTypeLayout("FlowYap" ###_NAME_##, FOnGetPropertyTypeCustomizationInstance::CreateStatic(FPropertyCustomization_FlowYap ## _NAME_ ## ::MakeInstance))
#define REGISTER_CLASS_CUSTOMIZATION(_MODULE_, _NAME_) _MODULE_##.RegisterCustomClassLayout("FlowYap" ###_NAME_##, FOnGetDetailCustomizationInstance::CreateStatic(&FDetailCustomization_FlowYap ##_NAME_## ::MakeInstance))
#define REGISTER_ASSET_TYPE_ACTION(_MODULE_, _NAME_) RegisterAssetTypeAction(##_MODULE_##, MakeShareable(new FAssetTypeActions_FlowYap ##_NAME_##()))
#define UNREGISTER_PROP_CUSTOMIZATION(_MODULE_, _NAME_) _MODULE_##.UnregisterCustomPropertyTypeLayout("FlowYap" ###_NAME_##)
#define UNREGISTER_CLASS_CUSTOMIZATION(_MODULE_, _NAME_) _MODULE_##.UnregisterCustomClassLayout("FlowYap" ###_NAME_##)
#pragma endregion

void FFlowYapEditorModule::StartupModule()
{
	AssetCategory = { "Yap", LOCTEXT("Yap", "Yap") };
	
	AssetTypeActions.Add(MakeShareable(new FAssetTypeActions_FlowYapCharacter()));
	
	//DetailCustomizations.Add(UFlowNode_YapDialogue::StaticClass(), FOnGetDetailCustomizationInstance::CreateStatic(&FDetailCustomization_FlowYapDialogueNode::MakeInstance));
	
	//PropertyCustomizations.Add(*FFlowYapFragment::StaticStruct(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FPropertyCustomization_FlowYapFragment::MakeInstance));
	
	StartupModuleBase();

	// TODO fix this retardation into my style proper
	FGameplayTagFilteredStyle::Initialize();
}

void FFlowYapEditorModule::ShutdownModule()
{
	ShutdownModuleBase();
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FFlowYapEditorModule, FlowYapEditor)