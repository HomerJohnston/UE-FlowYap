#pragma once

#include "CoreMinimal.h"

#include "YapPromptHandle.generated.h"

class UObject;
class UFlowNode_YapDialogue;

USTRUCT(BlueprintType)
struct FLOWYAP_API FYapPromptHandle
{
	GENERATED_BODY()

public:
	FYapPromptHandle();

	FYapPromptHandle(UFlowNode_YapDialogue* InDialogueNode, uint8 InFragmentIndex);

	UPROPERTY(Transient)
	TObjectPtr<UFlowNode_YapDialogue> DialogueNode;

	UPROPERTY(Transient)
	uint8 FragmentIndex;

	void RunPrompt(UObject* WorldContextObject);
};