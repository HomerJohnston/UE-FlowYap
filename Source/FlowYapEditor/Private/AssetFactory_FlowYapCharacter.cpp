// Copyright Ghost Pepper Games, Inc. All Rights Reserved.

#include "AssetFactory_FlowYapCharacter.h"
#include "FlowYap/FlowYapCharacter.h"

#include "FlowYapEditor.h"

#define LOCTEXT_NAMESPACE "FlowYap"

UAssetFactory_FlowYapCharacter::UAssetFactory_FlowYapCharacter()
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UFlowYapCharacter::StaticClass();
}

UObject* UAssetFactory_FlowYapCharacter::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	// Uncomment one of these

	// Create an Object asset of this class (this creates an instance of the class in the Content Folder as a project asset)
	return NewObject<UFlowYapCharacter>(InParent, Class, Name, Flags | RF_Transactional);

	// Create a Blueprint Class asset of this class (this creates a child Blueprint class in the Content Folder, same as right clicking on class in C++ folder and choosing "Create Blueprint Child from Class")
	//return FKismetEditorUtilities::CreateBlueprint(Class, InParent, Name, BPTYPE_Normal, UBlueprint::StaticClass(), UBlueprintGeneratedClass::StaticClass());
}

uint32 UAssetFactory_FlowYapCharacter::GetMenuCategories() const
{
	return FFlowYapEditorModule::GetAssetCategory();
}

FText UAssetFactory_FlowYapCharacter::GetDisplayName() const
{
	return LOCTEXT("FlowYapCharacter", "FlowYap Character");
}

// ================================================================================================
FText FAssetTypeActions_FlowYapCharacter::GetName() const
{
	return LOCTEXT("FlowYapCharacter", "FlowYap Character");
}

FColor FAssetTypeActions_FlowYapCharacter::GetTypeColor() const
{
	return FColor(255, 255, 255);
}

UClass* FAssetTypeActions_FlowYapCharacter::GetSupportedClass() const
{
	return UFlowYapCharacter::StaticClass();
}

uint32 FAssetTypeActions_FlowYapCharacter::GetCategories()
{
	return FFlowYapEditorModule::GetAssetCategory();
}

#undef LOCTEXT_NAMESPACE

