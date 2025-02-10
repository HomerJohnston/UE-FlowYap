// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "Yap/YapDialogueHandle.h"

#define LOCTEXT_NAMESPACE "Yap"

FYapDialogueHandle::FYapDialogueHandle(UFlowNode_YapDialogue* InDialogueNode, uint8 InFragmentIndex, bool bInSkippable, bool bInManualAdvance)
{
	DialogueNode = InDialogueNode;
	FragmentIndex = InFragmentIndex;
	bSkippable = bInSkippable;
	bManualAdvance = bInManualAdvance;

	Guid = FGuid::NewGuid();
}

void FYapDialogueHandle::Invalidate()
{
	DialogueNode = nullptr;
	FragmentIndex = INDEX_NONE;
	Guid.Invalidate();
}

#undef LOCTEXT_NAMESPACE
