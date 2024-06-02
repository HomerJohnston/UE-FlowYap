#pragma once

#include "FlowYapAssetWrapper.generated.h"

USTRUCT(BlueprintType)
struct FFlowYapAssetWrapper
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UObject> AudioAsset;
};