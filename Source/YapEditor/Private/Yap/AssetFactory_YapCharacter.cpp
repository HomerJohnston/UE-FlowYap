// Copyright Ghost Pepper Games, Inc. All Rights Reserved.


#define LOCTEXT_NAMESPACE "FlowYap"
#include "Yap/AssetFactory_YapCharacter.h"

#include "Yap/YapCharacter.h"
#include "Yap/YapEditorModule.h"

UAssetFactory_YapCharacter::UAssetFactory_YapCharacter()
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UYapCharacter::StaticClass();
}

UObject* UAssetFactory_YapCharacter::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	// Uncomment one of these

	// Create an Object asset of this class (this creates an instance of the class in the Content Folder as a project asset)
	return NewObject<UYapCharacter>(InParent, Class, Name, Flags | RF_Transactional);

	// Create a Blueprint Class asset of this class (this creates a child Blueprint class in the Content Folder, same as right clicking on class in C++ folder and choosing "Create Blueprint Child from Class")
	//return FKismetEditorUtilities::CreateBlueprint(Class, InParent, Name, BPTYPE_Normal, UBlueprint::StaticClass(), UBlueprintGeneratedClass::StaticClass());
}

uint32 UAssetFactory_YapCharacter::GetMenuCategories() const
{
	return FYapEditorModule::GetAssetCategory();
}

FText UAssetFactory_YapCharacter::GetDisplayName() const
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
	return UYapCharacter::StaticClass();
}

uint32 FAssetTypeActions_FlowYapCharacter::GetCategories()
{
	return FYapEditorModule::GetAssetCategory();
}

#undef LOCTEXT_NAMESPACE

