// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "Yap/YapDialogueHandle.h"

#define LOCTEXT_NAMESPACE "Yap"

FYapDialogueHandle::FYapDialogueHandle(UFlowNode_YapDialogue* InDialogueNode, uint8 InFragmentIndex, bool bInSkippable)
{
	DialogueNode = InDialogueNode;
	FragmentIndex = InFragmentIndex;
	bSkippable = bInSkippable;

	Guid = FGuid::NewGuid();
}

void FYapDialogueHandle::Invalidate()
{
	DialogueNode = nullptr;
	FragmentIndex = INDEX_NONE;
	Guid.Invalidate();
	bSkippable = false;
}

#undef LOCTEXT_NAMESPACE
