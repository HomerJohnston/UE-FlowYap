// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "Yap/YapDialogueHandle.h"

#include "Yap/YapBlueprintFunctionLibrary.h"
#include "Yap/Interfaces/IYapHandleReactor.h"

#define LOCTEXT_NAMESPACE "Yap"

// ------------------------------------------------------------------------------------------------

FYapDialogueHandle FYapDialogueHandle::_InvalidHandle;

// ================================================================================================
// FYapDialogueHandleRef
// ================================================================================================

bool FYapDialogueHandleRef::SkipDialogue()
{
	return UYapBlueprintFunctionLibrary::SkipDialogue(*this);
}

// ------------------------------------------------------------------------------------------------

void FYapDialogueHandleRef::AddReactor(UObject* Reactor)
{
	UYapBlueprintFunctionLibrary::AddReactor(*this, Reactor);
}

// ------------------------------------------------------------------------------------------------

const TArray<FInstancedStruct>& FYapDialogueHandleRef::GetFragmentData()
{
	return UYapBlueprintFunctionLibrary::GetFragmentData(*this);
}

// ------------------------------------------------------------------------------------------------

bool FYapDialogueHandleRef::operator==(const FYapDialogueHandleRef& Other) const
{
	return Guid == Other.Guid;
}

// ================================================================================================
// FYapDialogueHandle
// ================================================================================================

FYapDialogueHandle::FYapDialogueHandle(UFlowNode_YapDialogue* InDialogueNode, uint8 InFragmentIndex)
{
	DialogueNode = InDialogueNode;
	FragmentIndex = InFragmentIndex;

	Guid = FGuid::NewGuid();
}

// ------------------------------------------------------------------------------------------------

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

// ------------------------------------------------------------------------------------------------

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

// ------------------------------------------------------------------------------------------------

void FYapDialogueHandle::AddReactor(UObject* Reactor)
{
	if (ensureMsgf(Reactor->Implements<UYapHandleReactor>(), TEXT("FYapDialogueHandle::AddReactor(...) failed: object does not implement IYapHandleReactor! [%s]"), *Reactor->GetName()))
	{
		Reactors.Add(Reactor);
	}
}

// ------------------------------------------------------------------------------------------------

bool FYapDialogueHandle::operator==(const FYapDialogueHandle& Other) const
{
	return Guid == Other.Guid;
}

// ------------------------------------------------------------------------------------------------

#undef LOCTEXT_NAMESPACE
