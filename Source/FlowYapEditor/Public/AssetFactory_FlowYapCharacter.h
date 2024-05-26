#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "AssetTypeActions_Base.h"

#include "AssetFactory_FlowYapCharacter.generated.h"

UCLASS()
class UAssetFactory_FlowYapCharacter : public UFactory
{
	GENERATED_BODY()
public:
	UAssetFactory_FlowYapCharacter();

	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	virtual uint32 GetMenuCategories() const override;
	virtual FText GetDisplayName() const override;
};

// ================================================================================================
class FAssetTypeActions_FlowYapCharacter : public FAssetTypeActions_Base
{
public:
	virtual FText GetName() const override;
	virtual FColor GetTypeColor() const override;
	virtual UClass* GetSupportedClass() const override;
	virtual uint32 GetCategories() override;
};
