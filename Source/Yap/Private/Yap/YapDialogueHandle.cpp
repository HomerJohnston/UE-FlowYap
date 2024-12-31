#include "Yap/YapDialogueHandle.h"

FYapDialogueHandle::FYapDialogueHandle(const UFlowNode_YapDialogue* InDialogueNode, uint8 InFragmentIndex, bool bInSkippable)
{
	DialogueNode = InDialogueNode;
	FragmentIndex = InFragmentIndex;
	bSkippable = bInSkippable;
}
