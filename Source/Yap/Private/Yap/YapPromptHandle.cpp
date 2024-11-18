#include "Yap/YapPromptHandle.h"

#include "Yap/Nodes/FlowNode_YapDialogue.h"
#include "Yap/YapSubsystem.h"

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

void FYapPromptHandle::RunPrompt(UObject* WorldContextObject)
{
	UWorld* World = WorldContextObject->GetWorld();

	if (!World)
	{
		return;
	}
	
	UYapSubsystem* Subsystem = World->GetSubsystem<UYapSubsystem>();

	Subsystem->RunPrompt(*this);
}

