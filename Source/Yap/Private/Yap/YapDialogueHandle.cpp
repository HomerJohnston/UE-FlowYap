// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "Yap/YapDialogueHandle.h"

#include "Yap/Interfaces/IYapHandleReactor.h"

#define LOCTEXT_NAMESPACE "Yap"

FYapDialogueHandle FYapDialogueHandle::_InvalidHandle;

FYapDialogueHandle::FYapDialogueHandle(UFlowNode_YapDialogue* InDialogueNode, uint8 InFragmentIndex)
{
	DialogueNode = InDialogueNode;
	FragmentIndex = InFragmentIndex;

	Guid = FGuid::NewGuid();
}

void FYapDialogueHandle::OnSpeakingEnds() const
{
	for (TWeakObjectPtr<UObject> Reactor : Reactors)
	{
		if (Reactor.IsValid())
		{
			IYapHandleReactor::Execute_K2_OnSpeakingEnds(Reactor.Get());
		}
	}
}

void FYapDialogueHandle::Invalidate()
{
	for (TWeakObjectPtr<UObject> Reactor : Reactors)
	{
		if (Reactor.IsValid())
		{
			IYapHandleReactor::Execute_K2_OnHandleInvalidated(Reactor.Get());
		}
	}
	
	DialogueNode = nullptr;
	FragmentIndex = INDEX_NONE;
	Guid.Invalidate();
	Reactors.Empty();
}

void FYapDialogueHandle::AddReactor(UObject* Reactor)
{
	if (Reactor->Implements<UYapHandleReactor>())
	{
		Reactors.Add(Reactor);
	}
	else
	{
		
	}
}

#undef LOCTEXT_NAMESPACE
