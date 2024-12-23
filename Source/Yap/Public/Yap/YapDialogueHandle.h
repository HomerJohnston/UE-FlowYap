#pragma once

#include "CoreMinimal.h"

#include "YapDialogueHandle.generated.h"

class UObject;
class UFlowNode_YapDialogue;

USTRUCT(BlueprintType)
struct YAP_API FYapDialogueHandle
{
	GENERATED_BODY()

public:
	FYapDialogueHandle();

	FYapDialogueHandle(const UFlowNode_YapDialogue* InDialogueNode, uint8 InFragmentIndex);

	UPROPERTY(Transient)
	TObjectPtr<UFlowNode_YapDialogue> DialogueNode = nullptr;

	UPROPERTY(Transient)
	uint8 FragmentIndex = 0;
};