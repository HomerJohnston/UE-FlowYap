#pragma once
#include "GameplayTagContainer.h"

#include "FlowYapEntity.generated.h"

UCLASS(BlueprintType)
class FLOWYAP_API UFlowYapEntity : public UObject
{
	GENERATED_BODY()
public:
	UFlowYapEntity();
	
protected:
	/** Human-readable name or title of this entity */
	UPROPERTY(EditAnywhere)
	FText EntityName;

	UPROPERTY(EditAnywhere)
	FLinearColor EntityColor;
	
	/** Used to find this actor in the world (e.g. for sending Flow tag notifications) */
	UPROPERTY(EditAnywhere)
	FGameplayTag IdentityTag;

public:
	UFUNCTION(BlueprintCallable)
	FText GetEntityName() const { return EntityName; }

	FLinearColor GetEntityColor() const { return EntityColor; }

	FGameplayTag GetIdentityTag() const { return IdentityTag; }
};
