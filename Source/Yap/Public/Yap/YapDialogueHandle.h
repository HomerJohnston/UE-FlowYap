// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

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

private:
	UPROPERTY(Transient, BlueprintReadOnly, meta = (AllowPrivateAccess))
	TObjectPtr<const UFlowNode_YapDialogue> DialogueNode = nullptr;

	UPROPERTY(Transient, BlueprintReadOnly, meta = (AllowPrivateAccess))
	uint8 FragmentIndex = 0;

	UPROPERTY(Transient, BlueprintReadOnly, meta = (AllowPrivateAccess, IgnoreForMemberInitializationTest))
	FGuid Guid;
	
	UPROPERTY(Transient, BlueprintReadOnly, meta = (AllowPrivateAccess))
	bool bSkippable = true;

public:
	const UFlowNode_YapDialogue* GetDialogueNode() const { return DialogueNode; }

	uint8 GetFragmentIndex() const { return FragmentIndex; }

	const FGuid& GetGuid() const { return Guid; }

	bool GetSkippable() const { return bSkippable; }
	
	bool operator==(const FYapDialogueHandle& Other) const
	{
		return Guid == Other.Guid;
	}
};

FORCEINLINE uint32 GetTypeHash(const FYapDialogueHandle& Struct)
{
	return GetTypeHash(Struct.GetGuid());
}