

// ================================================================================================

#include "Yap/FlowYapSubsystem.h"

#include "Logging/StructuredLog.h"
#include "Yap/FlowYapFragment.h"
#include "Yap/FlowYapLog.h"
#include "Yap/IFlowYapConversationListener.h"
#include "Yap/Nodes/FlowNode_YapDialogue.h"

FFlowYapActiveConversation::FFlowYapActiveConversation()
{
	FlowAsset = nullptr;
	Name = NAME_None;
}

bool FFlowYapActiveConversation::StartConversation(UFlowAsset* InOwningAsset, FName InName)
{
	if (Name != NAME_None)
	{
		UE_LOGFMT(FlowYap, Warning, "Tried to start conversation {0} but conversation {1} was already ongoing. Ignoring start request.", InName, Name);
		return false;
	}
	
	if (InName == NAME_None)
	{
		UE_LOG(FlowYap, Error, TEXT("Tried to start conversation named NONE! Did you forgot to name the Start Conversation node? Ignoring start request."));
		return false;
	}

	FlowAsset = InOwningAsset;
	Name = InName;

	OnConversationStarts.ExecuteIfBound(Name);
	
	return true;
}

bool FFlowYapActiveConversation::EndConversation()
{
	if (Name != NAME_None)
	{
		OnConversationEnds.ExecuteIfBound(Name);

		FlowAsset = nullptr;
		Name = NAME_None;

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

bool UFlowYapSubsystem::StartConversation(UFlowAsset* OwningAsset, FName ConversationName)
{
	return ActiveConversation.StartConversation(OwningAsset, ConversationName);
}

void UFlowYapSubsystem::EndCurrentConversation()
{
	if (!ActiveConversation.EndConversation())
	{
		return;
	}

	// TODO
	/*
	if (ConversationQueue.Num() > 0)
	{
		FName NextConversation = ConversationQueue[0];
		ConversationQueue.RemoveAt(0);
		StartConversation(NextConversation);
	}
	*/
}

void UFlowYapSubsystem::BroadcastFragmentStart(UFlowNode_YapDialogue* Dialogue, uint8 FragmentIndex)
{
	FGuid DialogueGUID = Dialogue->GetGuid();
	FFlowYapFragment* Fragment = Dialogue->GetFragmentByIndexMutable(FragmentIndex);
	const FFlowYapBit& Bit = Fragment->GetBit();

	FYapFragmentActivationCount& FragmentActivationCount = GlobalFragmentActivationCounts.FindOrAdd(DialogueGUID);

	int32& Count = FragmentActivationCount.Counts.FindOrAdd(FragmentIndex);
	Count += 1;

	FName ConversationName = NAME_None;

	if (ActiveConversation.FlowAsset == Dialogue->GetFlowAsset())
	{
		ConversationName = ActiveConversation.Name;
	}
	
	for (int i = 0; i < Listeners.Num(); ++i)
	{
		UObject* Listener = Listeners[i];
		IFlowYapConversationListener::Execute_OnDialogueStart(Listener, ConversationName, Bit);
	}
}

void UFlowYapSubsystem::BroadcastFragmentEnd(const UFlowNode_YapDialogue* OwnerDialogue, uint8 FragmentIndex)
{
	const FFlowYapBit& Bit = OwnerDialogue->GetFragmentByIndex(FragmentIndex)->GetBit();

	FName ConversationName = NAME_None;

	if (ActiveConversation.FlowAsset == OwnerDialogue->GetFlowAsset())
	{
		ConversationName = ActiveConversation.Name;
	}
	
	for (int i = 0; i < Listeners.Num(); ++i)
	{
		UObject* Listener = Listeners[i];
		IFlowYapConversationListener::Execute_OnDialogueEnd(Listener, ConversationName, Bit);
	}
}

int32 UFlowYapSubsystem::GetGlobalActivationCount(UFlowNode_YapDialogue* OwnerDialogue, uint8 FragmentIndex)
{
	FGuid DialogueGUID = OwnerDialogue->GetGuid();

	FYapFragmentActivationCount* FragmentActivationCount = GlobalFragmentActivationCounts.Find(DialogueGUID);

	if (FragmentActivationCount)
	{
		int32* Count = FragmentActivationCount->Counts.Find(FragmentIndex);

		if (Count)
		{
			return *Count;
		}
	}

	return 0;
}

bool UFlowYapSubsystem::FragmentGlobalActivationLimitMet(UFlowNode_YapDialogue* Dialogue, uint8 FragmentIndex) const
{
	const FYapFragmentActivationCount* DialogueCounts = GlobalFragmentActivationCounts.Find(Dialogue->GetGuid());

	if (DialogueCounts)
	{
		const int32* Count = DialogueCounts->Counts.Find(FragmentIndex);

		if (Count)
		{
			return *Count < Dialogue->GetFragmentByIndex(FragmentIndex)->GetGlobalActivationLimit();
		}
	}
	
	return false;
}

void UFlowYapSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	ActiveConversation.OnConversationStarts.BindUObject(this, &UFlowYapSubsystem::OnConversationStarts_Internal);
	ActiveConversation.OnConversationEnds.BindUObject(this, &UFlowYapSubsystem::OnConversationEnds_Internal);
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

bool UFlowYapSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::GamePreview || WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}
