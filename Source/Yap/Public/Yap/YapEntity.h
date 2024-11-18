#pragma once
#include "GameplayTagContainer.h"

#include "YapEntity.generated.h"

UCLASS(BlueprintType)
class YAP_API UYapEntity : public UObject
{
	GENERATED_BODY()
public:
	UYapEntity();
	
protected:
	/** Human-readable name or title of this entity */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText EntityName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FLinearColor EntityColor;
	
	/** Used to find this actor in the world (e.g. for sending Flow tag notifications) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag IdentityTag;

public:
	FText GetEntityName() const { return EntityName; }

	FLinearColor GetEntityColor() const { return EntityColor; }

	FGameplayTag GetIdentityTag() const { return IdentityTag; }
};
