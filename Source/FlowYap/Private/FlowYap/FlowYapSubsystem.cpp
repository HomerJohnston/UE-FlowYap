#include "FlowYap/FlowYapSubsystem.h"

#include "FlowYap/FlowYapLog.h"
#include "FlowYap/IFlowYapConversationListener.h"

// ================================================================================================

FFlowYapActiveConversation::FFlowYapActiveConversation()
{
	ConversationName = NAME_None;
}

bool FFlowYapActiveConversation::TryStartConversation(FName InName)
{
	if (InName == NAME_None)
	{
		UE_LOG(FlowYap, Error, TEXT("Tried to start conversation named NONE! Did you forgot to name the Start Conversation node?"));
		return false;
	}
	
	if (ConversationName == NAME_None)
	{
		ConversationName = InName;
		OnConversationStarts.ExecuteIfBound(ConversationName);
		return true;
	}

	return false;
}

bool FFlowYapActiveConversation::EndConversation()
{
	if (ConversationName != NAME_None)
	{
		OnConversationEnds.ExecuteIfBound(ConversationName);
		ConversationName = NAME_None;

		return true;
	}

	return false;
}

// ================================================================================================

UFlowYapSubsystem::UFlowYapSubsystem()
{
}

void UFlowYapSubsystem::AddConversationListener(UObject* NewListener)
{
	if (NewListener->Implements<UFlowYapConversationListener>())
	{
		Listeners.AddUnique(NewListener);
	}
}

void UFlowYapSubsystem::RemoveConversationListener(UObject* RemovedListener)
{
	Listeners.Remove(RemovedListener);
}

void UFlowYapSubsystem::StartConversation(FName ConversationName)
{
	if (Conversation.TryStartConversation(ConversationName))
	{
		return;
	}
	
	ConversationQueue.Add(ConversationName); // It is possible to add multiple of the same conversation name, should this be a warned behaviour? Probably? TODO?
}

void UFlowYapSubsystem::EndCurrentConversation()
{
	if (!Conversation.EndConversation())
	{
		return;
	}

	if (ConversationQueue.Num() > 0)
	{
		FName NextConversation = ConversationQueue[0];
		ConversationQueue.RemoveAt(0);
		StartConversation(NextConversation);
	}
}

void UFlowYapSubsystem::DialogueStart(FName ConversationName, const FFlowYapBit& DialogueBit)
{
	for (int i = 0; i < Listeners.Num(); ++i)
	{
		UObject* Listener = Listeners[i];
		IFlowYapConversationListener::Execute_OnDialogueStart(Listener, ConversationName, DialogueBit);
	}
}

void UFlowYapSubsystem::DialogueEnd(FName ConversationName, const FFlowYapBit& DialogueBit)
{
	for (int i = 0; i < Listeners.Num(); ++i)
	{
		UObject* Listener = Listeners[i];
		IFlowYapConversationListener::Execute_OnDialogueEnd(Listener, ConversationName, DialogueBit);
	}
}

void UFlowYapSubsystem::DialogueInterrupt(FName ConversationName, const FFlowYapBit& DialogueBit)
{
	for (int i = 0; i < Listeners.Num(); ++i)
	{
		UObject* Listener = Listeners[i];
		IFlowYapConversationListener::Execute_OnDialogueInterrupt(Listener, ConversationName, DialogueBit);
	}
}

void UFlowYapSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Conversation.OnConversationStarts.BindUObject(this, &UFlowYapSubsystem::OnConversationStarts_Internal);
	Conversation.OnConversationEnds.BindUObject(this, &UFlowYapSubsystem::OnConversationEnds_Internal);
}

void UFlowYapSubsystem::OnConversationStarts_Internal(FName Name)
{
	for (int i = 0; i < Listeners.Num(); ++i)
	{
		UObject* Listener = Listeners[i];
		IFlowYapConversationListener::Execute_OnConversationStarts(Listener, Name);
	}
}

void UFlowYapSubsystem::OnConversationEnds_Internal(FName Name)
{
	for (int i = 0; i < Listeners.Num(); ++i)
	{
		UObject* Listener = Listeners[i];
		IFlowYapConversationListener::Execute_OnConversationEnds(Listener, Name);
	}
}