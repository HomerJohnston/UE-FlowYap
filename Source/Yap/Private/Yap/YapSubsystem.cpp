

// ================================================================================================

#include "Yap/YapSubsystem.h"

#include "GameplayTagsManager.h"
#include "Logging/StructuredLog.h"
#include "Yap/YapFragment.h"
#include "Yap/YapLog.h"
#include "Yap/YapConversationHandler.h"
#include "Yap/YapPromptHandle.h"
#include "Yap/Nodes/FlowNode_YapDialogue.h"

FFlowYapActiveConversation::FFlowYapActiveConversation()
{
	FlowAsset = nullptr;
	Conversation = FGameplayTag::EmptyTag;
}

bool FFlowYapActiveConversation::StartConversation(UFlowAsset* InOwningAsset, const FGameplayTag& InConversation)
{
	if (Conversation != FGameplayTag::EmptyTag)
	{
		UE_LOGFMT(FlowYap, Warning, "Tried to start conversation {0} but conversation {1} was already ongoing. Ignoring start request.", InConversation.ToString(), Conversation.ToString());
		return false;
	}
	
	if (InConversation == FGameplayTag::EmptyTag)
	{
		UE_LOG(FlowYap, Error, TEXT("Tried to start conversation named NONE! Did you forgot to name the Start Conversation node? Ignoring start request."));
		return false;
	}

	FlowAsset = InOwningAsset;
	Conversation = InConversation;

	OnConversationStarts.ExecuteIfBound(Conversation);
	
	return true;
}

bool FFlowYapActiveConversation::EndConversation()
{
	if (Conversation != FGameplayTag::EmptyTag)
	{
		OnConversationEnds.ExecuteIfBound(Conversation);

		FlowAsset = nullptr;
		Conversation = FGameplayTag::EmptyTag;

		return true;
	}

	return false;
}

// ================================================================================================

UYapSubsystem::UYapSubsystem()
{
}

void UYapSubsystem::AddConversationHandler(UObject* NewListener)
{
	if (NewListener->Implements<UYapConversationHandler>())
	{
		Listeners.AddUnique(NewListener);
	}
	else
	{
		UE_LOGFMT(FlowYap, Warning, "Tried to register a conversation handler, {0} but it does not implement the FlowYapConversationHandler interface!");
	}
}

void UYapSubsystem::RemoveConversationHandler(UObject* RemovedListener)
{
	Listeners.Remove(RemovedListener);
}

void UYapSubsystem::RegisterTaggedFragment(const FGameplayTag& FragmentTag, UFlowNode_YapDialogue* DialogueNode)
{
	if (TaggedFragments.Contains(FragmentTag))
	{
		UE_LOGFMT(FlowYap, Warning, "Tried to register tagged fragment with tag [{0}] but this tag was already registered! Find and fix the duplicate tag usage.", FragmentTag.ToString()); // TODO if I pass in the full fragment I could log the dialogue text to make this easier for designers?
		return;
	}
	
	TaggedFragments.Add(FragmentTag, DialogueNode);
}

FYapFragment* UYapSubsystem::FindTaggedFragment(const FGameplayTag& FragmentTag)
{
	UFlowNode_YapDialogue** DialoguePtr = TaggedFragments.Find(FragmentTag);

	if (DialoguePtr)
	{
		return (*DialoguePtr)->FindTaggedFragment(FragmentTag);
	}

	return nullptr;
}

bool UYapSubsystem::StartConversation(UFlowAsset* OwningAsset, const FGameplayTag& Conversation)
{
	return ActiveConversation.StartConversation(OwningAsset, Conversation);
}

void UYapSubsystem::EndCurrentConversation()
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

void UYapSubsystem::BroadcastPrompt(UFlowNode_YapDialogue* Dialogue, uint8 FragmentIndex)
{
	FGuid DialogueGUID = Dialogue->GetGuid();
	FYapFragment& Fragment = Dialogue->GetFragmentByIndexMutable(FragmentIndex);
	const FYapBit& Bit = Fragment.GetBit();

	FGameplayTag ConversationName;

	if (ActiveConversation.FlowAsset == Dialogue->GetFlowAsset())
	{
		ConversationName = ActiveConversation.Conversation;
	}

	FYapPromptHandle Handle(Dialogue, FragmentIndex);
	
	for (int i = 0; i < Listeners.Num(); ++i)
	{
		UObject* Listener = Listeners[i];
		IYapConversationHandler::Execute_AddPrompt(Listener, ConversationName, Bit, Handle);
	}
}

void UYapSubsystem::BroadcastDialogueStart(UFlowNode_YapDialogue* Dialogue, uint8 FragmentIndex)
{
	FGuid DialogueGUID = Dialogue->GetGuid();
	FYapFragment& Fragment = Dialogue->GetFragmentByIndexMutable(FragmentIndex);
	const FYapBit& Bit = Fragment.GetBit();

	FGameplayTag ConversationName;

	if (ActiveConversation.FlowAsset == Dialogue->GetFlowAsset())
	{
		ConversationName = ActiveConversation.Conversation;
	}
	
	for (int i = 0; i < Listeners.Num(); ++i)
	{
		UObject* Listener = Listeners[i];
		IYapConversationHandler::Execute_OnDialogueStart(Listener, ConversationName, Bit);
	}
}

void UYapSubsystem::BroadcastDialogueEnd(const UFlowNode_YapDialogue* OwnerDialogue, uint8 FragmentIndex)
{
	const FYapBit& Bit = OwnerDialogue->GetFragmentByIndex(FragmentIndex).GetBit();

	FGameplayTag ConversationName;

	if (ActiveConversation.FlowAsset == OwnerDialogue->GetFlowAsset())
	{
		ConversationName = ActiveConversation.Conversation;
	}
	
	for (int i = 0; i < Listeners.Num(); ++i)
	{
		UObject* Listener = Listeners[i];
		IYapConversationHandler::Execute_OnDialogueEnd(Listener, ConversationName, Bit);
	}
}

void UYapSubsystem::RunPrompt(FYapPromptHandle& Handle)
{
	//BroadcastDialogueStart(Handle.DialogueNode, Handle.FragmentIndex);

	Handle.DialogueNode->RunPrompt(Handle.FragmentIndex);
}

// =====================================================================================================

void UYapSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	ActiveConversation.OnConversationStarts.BindUObject(this, &UYapSubsystem::OnConversationStarts_Internal);
	ActiveConversation.OnConversationEnds.BindUObject(this, &UYapSubsystem::OnConversationEnds_Internal);
}

void UYapSubsystem::OnConversationStarts_Internal(const FGameplayTag& Name)
{
	for (int i = 0; i < Listeners.Num(); ++i)
	{
		UObject* Listener = Listeners[i];
		IYapConversationHandler::Execute_OnConversationStarts(Listener, Name);
	}
}

void UYapSubsystem::OnConversationEnds_Internal(const FGameplayTag& Name)
{
	for (int i = 0; i < Listeners.Num(); ++i)
	{
		UObject* Listener = Listeners[i];
		IYapConversationHandler::Execute_OnConversationEnds(Listener, Name);
	}
}

bool UYapSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::GamePreview || WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}
