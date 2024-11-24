#pragma once

#include "GameplayTagContainer.h"

#include "YapCharacterComponent.generated.h"

UCLASS(meta=(BlueprintSpawnableComponent))
class YAP_API UYapCharacterComponent : public UActorComponent
{
	GENERATED_BODY()

protected:
UPROPERTY(EditAnywhere)
	FGameplayTag Identity;

public:
	void BeginPlay() override;

	void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	const FGameplayTag& GetCharacterTag() { return Identity; }
};