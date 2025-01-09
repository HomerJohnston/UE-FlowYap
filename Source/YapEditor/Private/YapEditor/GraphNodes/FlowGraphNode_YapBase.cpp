// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "YapEditor/GraphNodes/FlowGraphNode_YapBase.h"

#include "YapEditor/YapLogEditor.h"

#define LOCTEXT_NAMESPACE "YapEditor"

UFlowGraphNode_YapBase::UFlowGraphNode_YapBase()
{
}

void UFlowGraphNode_YapBase::Broadcast(FName Event)
{
	TMulticastDelegate<void()>* Evt = EventBus.Events.Find(Event);

	if (Evt)
	{
		Evt->Broadcast();
	}
	else
	{
		UE_LOG(LogYapEditor, Error, TEXT("Tried to broadcast editor event: [%s] but this event was not registered on class [%s]"), *Event.ToString(), *GetClass()->GetName());
	}
}

#undef LOCTEXT_NAMESPACE