#include "Yap/YapPromptHandle.h"

#include "Yap/Nodes/FlowNode_YapDialogue.h"
#include "Yap/FlowYapSubsystem.h"

FYapPromptHandle::FYapPromptHandle()
{
	DialogueNode = nullptr;
	FragmentIndex = 0;
}

FYapPromptHandle::FYapPromptHandle(UFlowNode_YapDialogue* InDialogueNode, uint8 InFragmentIndex)
{
	DialogueNode = InDialogueNode;
	FragmentIndex = InFragmentIndex;
}

void FYapPromptHandle::Select(UObject* WorldContextObject)
{
	UWorld* World = WorldContextObject->GetWorld();

	if (!World)
	{
		return;
	}
	
	UFlowYapSubsystem* Subsystem = World->GetSubsystem<UFlowYapSubsystem>();

	Subsystem->ActivatePrompt(*this);
}

