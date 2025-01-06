// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once

#include "CoreMinimal.h"

#include "YapPromptHandle.generated.h"

class UObject;
class UFlowNode_YapDialogue;

USTRUCT(BlueprintType)
struct YAP_API FYapPromptHandle
{
	GENERATED_BODY()

public:
	FYapPromptHandle();

	FYapPromptHandle(UFlowNode_YapDialogue* InDialogueNode, uint8 InFragmentIndex);

	UPROPERTY(Transient)
	TObjectPtr<UFlowNode_YapDialogue> DialogueNode;

	UPROPERTY(Transient)
	uint8 FragmentIndex;

	UPROPERTY(BlueprintReadOnly, Transient)
	bool bSkippable = true;
	
	void RunPrompt(UObject* WorldContextObject);
};