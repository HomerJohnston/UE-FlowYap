#pragma once

#include "CoreMinimal.h"

#include "YapDialogueHandle.generated.h"

class UObject;
class UFlowNode_YapDialogue;

/** Handle for a piece of running dialogue. Used to track individual running dialogues. Can be used to instruct the Yap Subsystem to interrupt a piece of dialogue, unless it is marked not skippable. */
USTRUCT(BlueprintType)
struct YAP_API FYapDialogueHandle
{
	GENERATED_BODY()

public:
	FYapDialogueHandle() { };

	FYapDialogueHandle(const UFlowNode_YapDialogue* InDialogueNode, uint8 InFragmentIndex, bool bInSkippable);

	UPROPERTY(Transient)
	TObjectPtr<const UFlowNode_YapDialogue> DialogueNode = nullptr;

	UPROPERTY(Transient)
	uint8 FragmentIndex = 0;

	UPROPERTY(BlueprintReadOnly, Transient)
	bool bSkippable = true;

	bool operator==(const FYapDialogueHandle& Other) const
	{
		return DialogueNode == Other.DialogueNode && FragmentIndex == Other.FragmentIndex;
	}
};

FORCEINLINE uint32 GetTypeHash(const FYapDialogueHandle& Struct)
{
	return HashCombine(GetTypeHash(Struct.DialogueNode), GetTypeHash(Struct.FragmentIndex));
}