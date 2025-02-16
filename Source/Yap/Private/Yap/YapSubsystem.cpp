// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

// ================================================================================================

#include "Yap/YapSubsystem.h"

#include "Logging/StructuredLog.h"
#include "Yap/YapBroker.h"
#include "Yap/YapFragment.h"
#include "Yap/YapLog.h"
#include "Yap/Interfaces/IYapConversationHandler.h"
#include "Yap/YapDialogueHandle.h"
#include "Yap/YapProjectSettings.h"
#include "Yap/YapPromptHandle.h"
#include "Yap/Enums/YapLoadContext.h"
#include "Yap/Interfaces/IYapFreeSpeechHandler.h"
#include "Yap/Nodes/FlowNode_YapDialogue.h"

#define LOCTEXT_NAMESPACE "Yap"

#define YAP_BROADCAST_EVT_TARGS(NAME, CPPFUNC, K2FUNC) U##NAME, I##NAME, &I##NAME::CPPFUNC, &I##NAME::K2FUNC

TWeakObjectPtr<UWorld> UYapSubsystem::World = nullptr;
bool UYapSubsystem::bGetGameMaturitySettingWarningIssued = false;

// ------------------------------------------------------------------------------------------------

FYapActiveConversation::FYapActiveConversation()
{
	FlowAsset = nullptr;
	Conversation.Reset();
}

// ------------------------------------------------------------------------------------------------

bool FYapActiveConversation::OpenConversation(UFlowAsset* InOwningAsset, const FGameplayTag& InConversation)
{
	if (Conversation.IsSet())
	{
		UE_LOGFMT(LogYap, Warning, "Tried to open conversation {0} but conversation {1} was already ongoing. Ignoring request.", InConversation.ToString(), Conversation.GetValue().ToString());
		return false;
	}
	
	FlowAsset = InOwningAsset;
	Conversation = InConversation;

	(void)OnConversationOpens.ExecuteIfBound(Conversation.GetValue());
	
	return true;
}

// ------------------------------------------------------------------------------------------------

bool FYapActiveConversation::CloseConversation()
{
	if (Conversation.IsSet())
	{
		(void)OnConversationCloses.ExecuteIfBound(Conversation.GetValue());

		FlowAsset = nullptr;
		Conversation.Reset();

		return true;
	}

	return false;
}

// ------------------------------------------------------------------------------------------------

UYapSubsystem::UYapSubsystem()
{
}

// ------------------------------------------------------------------------------------------------

void UYapSubsystem::RegisterConversationHandler(UObject* NewHandler)
{
	if (NewHandler->Implements<UYapConversationHandler>())
	{
		Get()->ConversationHandlers.AddUnique(NewHandler);
	}
	else
	{
		UE_LOG(LogYap, Error, TEXT("Tried to register a conversation handler, but object does not implement the IYapConversationHandler interface! [%s]"), *NewHandler->GetName());
	}
}

// ------------------------------------------------------------------------------------------------

void UYapSubsystem::UnregisterConversationHandler(UObject* HandlerToRemove)
{
	Get()->ConversationHandlers.Remove(HandlerToRemove);
}

// ------------------------------------------------------------------------------------------------

void UYapSubsystem::RegisterFreeSpeechHandler(UObject* NewHandler)
{
	if (NewHandler->Implements<UYapFreeSpeechHandler>())
	{
		Get()->FreeSpeechHandlers.AddUnique(NewHandler);
	}
	else
	{
		UE_LOG(LogYap, Error, TEXT("Tried to register a free speech handler, but object does not implement the IYapFreeSpeechHandler interface! [%s]"), *NewHandler->GetName());
	}
}

// ------------------------------------------------------------------------------------------------

void UYapSubsystem::UnregisterFreeSpeechHandler(UObject* HandlerToRemove)
{
	Get()->FreeSpeechHandlers.Remove(HandlerToRemove);
}

// ------------------------------------------------------------------------------------------------

UYapCharacterComponent* UYapSubsystem::FindCharacterComponent(FGameplayTag CharacterTag)
{
	TWeakObjectPtr<UYapCharacterComponent>* CharacterComponentPtr = YapCharacterComponents.Find(CharacterTag);

	if (CharacterComponentPtr && CharacterComponentPtr->IsValid())
	{
		return CharacterComponentPtr->Get();
	}

	return nullptr;
}

// ------------------------------------------------------------------------------------------------

UYapBroker* UYapSubsystem::GetBroker()
{
	UYapBroker* Broker = Get()->Broker;

#if WITH_EDITOR
	ensureMsgf(IsValid(Broker), TEXT("Conversation Broker is invalid. Did you create one and assign it in project settings? Docs: https://github.com/HomerJohnston/UE-FlowYap/wiki/Conversation-Broker"));
#endif
	
	return Broker;		
}

// ------------------------------------------------------------------------------------------------

EYapMaturitySetting UYapSubsystem::GetCurrentMaturitySetting()
{
	EYapMaturitySetting MaturitySetting;
	
	if (!ensureMsgf(World.IsValid(), TEXT("World was invalid in UYapSubsystem::GetGameMaturitySetting(). This should never happen! Defaulting to mature.")))
	{
		MaturitySetting = EYapMaturitySetting::Mature; 
	}
	else
	{
		UYapBroker* Broker = GetBroker();

		if (ensureMsgf(IsValid(Broker), TEXT("No broker set in project settings! Defaulting to mature.")))
		{
			MaturitySetting = Broker->GetMaturitySetting();
		}
		else
		{
			MaturitySetting = EYapMaturitySetting::Mature;
		}	
	}

	// Something went wrong... we will hard-code default to mature.
	if (MaturitySetting == EYapMaturitySetting::Unspecified)
	{
		bool bSetWarningIssued = false;

		if (!bGetGameMaturitySettingWarningIssued)
		{
			UE_LOG(LogYap, Error, TEXT("UYapSubsystem::GetGameMaturitySetting failed to get a valid game maturity setting! Using default project maturity setting. This could be caused by a faulty Broker implementation."));
			bSetWarningIssued = true;
		}
		
		MaturitySetting = EYapMaturitySetting::Mature;
		
		if (bSetWarningIssued)
		{
			bGetGameMaturitySettingWarningIssued = true;
		}
	}
	
	return MaturitySetting;
}

// ------------------------------------------------------------------------------------------------

void UYapSubsystem::RegisterTaggedFragment(const FGameplayTag& FragmentTag, UFlowNode_YapDialogue* DialogueNode)
{
	if (TaggedFragments.Contains(FragmentTag))
	{
		UE_LOG(LogYap, Warning, TEXT("Tried to register tagged fragment with tag [%s] but this tag was already registered! Find and fix the duplicate tag usage."), *FragmentTag.ToString()); // TODO if I pass in the full fragment I could log the dialogue text to make this easier for designers?
		return;
	}
	
	TaggedFragments.Add(FragmentTag, DialogueNode);
}

// ------------------------------------------------------------------------------------------------

FYapFragment* UYapSubsystem::FindTaggedFragment(const FGameplayTag& FragmentTag)
{
	UFlowNode_YapDialogue** DialoguePtr = TaggedFragments.Find(FragmentTag);

	if (DialoguePtr)
	{
		return (*DialoguePtr)->FindTaggedFragment(FragmentTag);
	}

	return nullptr;
}

// ------------------------------------------------------------------------------------------------

bool UYapSubsystem::OpenConversation(UFlowAsset* OwningAsset, const FGameplayTag& Conversation)
{
	// TODO - Queue or nest conversations

	return ActiveConversation.OpenConversation(OwningAsset, Conversation);
}

// ------------------------------------------------------------------------------------------------

void UYapSubsystem::CloseConversation()
{
	if (!ActiveConversation.CloseConversation())
	{
		return;
	}

	// TODO - Queue or nest conversations
	/*
	if (ConversationQueue.Num() > 0)
	{
		FName NextConversation = ConversationQueue[0];
		ConversationQueue.RemoveAt(0);
		OpenConversation(NextConversation);
	}
	*/
}

// ------------------------------------------------------------------------------------------------

FYapPromptHandle UYapSubsystem::BroadcastPrompt(UFlowNode_YapDialogue* Dialogue, uint8 FragmentIndex)
{
	// TODO not sure if there's a clean way to avoid const_cast. The problem is that GetDirectedAt and GetSpeaker (used below) are mutable, because they forcefully load assets.
	FYapFragment& Fragment = const_cast<FYapFragment&>(Dialogue->GetFragmentByIndex(FragmentIndex));
	FYapBit& Bit = const_cast<FYapBit&>(Fragment.GetBit()); 

	FGameplayTag ConversationName;

	if (ActiveConversation.FlowAsset == Dialogue->GetFlowAsset())
	{
		ConversationName = ActiveConversation.Conversation.GetValue();
	}

	FYapPromptHandle Handle(Dialogue, FragmentIndex);

	if (ConversationName.IsValid())
	{
		FYapData_ConversationPlayerPromptCreated Data;
		Data.Conversation = ConversationName;
		Data.Handle = Handle;
		Data.DirectedAt = Fragment.GetDirectedAt(EYapLoadContext::Sync);
		Data.Speaker = Fragment.GetSpeaker(EYapLoadContext::Sync);
		Data.MoodTag = Fragment.GetMoodTag();
		Data.DialogueText = Bit.GetDialogueText();
		Data.TitleText = Bit.GetTitleText();

		BroadcastEventHandlerFunc<YAP_BROADCAST_EVT_TARGS(YapConversationHandler, OnConversationPlayerPromptCreated, Execute_K2_ConversationPlayerPromptCreated)>(ConversationHandlers, Data);
	}
	else
	{
		UE_LOG(LogYap, Error, TEXT("Tried to create a player prompt, but there is no active conversation!"));
	}

	return Handle;
}

// ------------------------------------------------------------------------------------------------

void UYapSubsystem::OnFinishedBroadcastingPrompts()
{
	FGameplayTag ConversationName = ActiveConversation.IsConversationInProgress() ? ActiveConversation.Conversation.GetValue() : FGameplayTag::EmptyTag;

	if (ConversationName.IsValid())
	{
		FYapData_ConversationPlayerPromptsReady Data;
		Data.Conversation = ConversationName;
	
		BroadcastEventHandlerFunc<YAP_BROADCAST_EVT_TARGS(YapConversationHandler, OnConversationPlayerPromptsReady, Execute_K2_ConversationPlayerPromptsReady)>(ConversationHandlers, Data);
	}
	else
	{
		UE_LOG(LogYap, Error, TEXT("Tried to broadcast player prompts created, but there is no active conversation!"));
	}
}

// ------------------------------------------------------------------------------------------------

void UYapSubsystem::BroadcastDialogueStart(UFlowNode_YapDialogue* Dialogue, uint8 FragmentIndex)
{
	// TODO not sure if there's a clean way to avoid const_cast. The problem is that GetDirectedAt and GetSpeaker (used below) are mutable, because they forcefully load assets.
	FYapFragment& Fragment = const_cast<FYapFragment&>(Dialogue->GetFragmentByIndex(FragmentIndex));
	FYapBit& Bit = const_cast<FYapBit&>(Fragment.GetBit());
	FYapDialogueHandleRef DialogueHandleRef(Dialogue->DialogueHandle.GetGuid());
	GuidDialogueMap.Add(DialogueHandleRef, Dialogue);
	FGameplayTag ConversationName;

	if (ActiveConversation.FlowAsset == Dialogue->GetFlowAsset())
	{
		ConversationName = ActiveConversation.Conversation.GetValue();
	}
	
	TOptional<float> Time = Fragment.GetTime();

	float EffectiveTime;
	
	if (Time.IsSet())
	{
		EffectiveTime = Time.GetValue();
	}
	else
	{
		EffectiveTime = UYapProjectSettings::GetMinimumFragmentTime();
	}

	
	if (ConversationName.IsValid())
	{
		FYapData_ConversationSpeechBegins Data;
		Data.Conversation = ConversationName;
		Data.DialogueHandleRef = DialogueHandleRef;
		Data.DirectedAt = Fragment.GetDirectedAt(EYapLoadContext::Sync);
		Data.Speaker = Fragment.GetSpeaker(EYapLoadContext::Sync);
		Data.MoodTag = Fragment.GetMoodTag();
		Data.DialogueText = Bit.GetDialogueText();
		Data.TitleText = Bit.GetTitleText();
		Data.DialogueTime = EffectiveTime;
		Data.PaddingTime = Fragment.GetPaddingToNextFragment(); 
		Data.DialogueAudioAsset = Bit.GetAudioAsset<UObject>();
		Data.bSkippable = Fragment.GetSkippable(Dialogue->GetSkippable());
	
		BroadcastEventHandlerFunc<YAP_BROADCAST_EVT_TARGS(YapConversationHandler, OnConversationSpeechBegins, Execute_K2_ConversationSpeechBegins)>(ConversationHandlers, Data);
	}
	else
	{
		FYapData_TalkSpeechBegins Data;
		Data.DialogueHandleRef = DialogueHandleRef;
		Data.DirectedAt = Fragment.GetDirectedAt(EYapLoadContext::Sync);
		Data.Speaker = Fragment.GetSpeaker(EYapLoadContext::Sync);
		Data.MoodTag = Fragment.GetMoodTag();
		Data.DialogueText = Bit.GetDialogueText();
		Data.TitleText = Bit.GetTitleText();
		Data.DialogueTime = EffectiveTime;
		Data.PaddingTime = Fragment.GetPaddingToNextFragment(); 
		Data.DialogueAudioAsset = Bit.GetAudioAsset<UObject>();
		Data.bSkippable = Fragment.GetSkippable(Dialogue->GetSkippable());
	
		BroadcastEventHandlerFunc<YAP_BROADCAST_EVT_TARGS(YapFreeSpeechHandler, OnTalkSpeechBegins, Execute_K2_TalkSpeechBegins)>(FreeSpeechHandlers, Data);
	}
}

// ------------------------------------------------------------------------------------------------

void UYapSubsystem::BroadcastDialogueEnd(const UFlowNode_YapDialogue* Dialogue, uint8 FragmentIndex)
{
	FGameplayTag ConversationName;
	const FYapFragment& Fragment = Dialogue->GetFragmentByIndex(FragmentIndex);
	FYapDialogueHandleRef DialogueHandleRef(Dialogue->DialogueHandle.GetGuid());

	if (ActiveConversation.FlowAsset == Dialogue->GetFlowAsset())
	{
		ConversationName = ActiveConversation.Conversation.GetValue();
	}

	if (ConversationName.IsValid())
	{
		FYapData_ConversationSpeechEnds Data;
		Data.Conversation = ConversationName;
		Data.DialogueHandleRef = DialogueHandleRef;
		Data.PaddingTime = Fragment.GetPaddingToNextFragment();
	
		BroadcastEventHandlerFunc<YAP_BROADCAST_EVT_TARGS(YapConversationHandler, OnConversationSpeechEnds, Execute_K2_ConversationSpeechEnds)>(ConversationHandlers, Data);
	}
	else
	{
		FYapData_TalkSpeechEnds Data;
		Data.DialogueHandleRef = DialogueHandleRef;
		Data.PaddingTime = Fragment.GetPaddingToNextFragment();
		
		BroadcastEventHandlerFunc<YAP_BROADCAST_EVT_TARGS(YapFreeSpeechHandler, OnTalkSpeechEnds, Execute_K2_TalkSpeechEnds)>(FreeSpeechHandlers, Data);
	}	
}

// ------------------------------------------------------------------------------------------------

void UYapSubsystem::BroadcastPaddingTimeOver(const UFlowNode_YapDialogue* Dialogue, uint8 FragmentIndex)
{
	FGameplayTag ConversationName;
	const FYapFragment& Fragment = Dialogue->GetFragmentByIndex(FragmentIndex);
	FYapDialogueHandleRef DialogueHandleRef(Dialogue->DialogueHandle.GetGuid());

	if (ActiveConversation.FlowAsset == Dialogue->GetFlowAsset())
	{
		ConversationName = ActiveConversation.Conversation.GetValue();
	}

	if (ConversationName.IsValid())
	{
		FYapData_ConversationSpeechPaddingEnds Data;
		Data.Conversation = ConversationName;
		Data.DialogueHandleRef = FYapDialogueHandleRef(Dialogue->DialogueHandle.GetGuid());
		Data.bManualAdvance = !Fragment.GetAutoAdvance(Dialogue->GetAutoAdvance());
	
		BroadcastEventHandlerFunc<YAP_BROADCAST_EVT_TARGS(YapConversationHandler, OnConversationSpeechPaddingEnds, Execute_K2_ConversationSpeechPaddingEnds)>(ConversationHandlers, Data);
	}
	else
	{
		FYapData_TalkSpeechPaddingEnds Data;
		Data.DialogueHandleRef = FYapDialogueHandleRef(Dialogue->DialogueHandle.GetGuid());
		Data.bManualAdvance = !Fragment.GetAutoAdvance(Dialogue->GetAutoAdvance());
		
		BroadcastEventHandlerFunc<YAP_BROADCAST_EVT_TARGS(YapFreeSpeechHandler, OnTalkSpeechPaddingEnds, Execute_K2_TalkSpeechPaddingEnds)>(FreeSpeechHandlers, Data);
	}

	GuidDialogueMap.Remove(DialogueHandleRef);
}

// ------------------------------------------------------------------------------------------------

bool UYapSubsystem::RunPrompt(const FYapPromptHandle& Handle)
{
	UYapSubsystem* This = Get();
	check(This);
	
	// TODO handle invalid handles gracefully
	Handle.GetDialogueNode()->RunPrompt(Handle.GetFragmentIndex());

	FGameplayTag ConversationName;

	if (This->ActiveConversation.FlowAsset == Handle.GetDialogueNode()->GetFlowAsset())
	{
		ConversationName = This->ActiveConversation.Conversation.GetValue();
	}
	
	FYapData_ConversationPlayerPromptChosen Data;
	Data.Conversation = ConversationName;
	
	This->BroadcastEventHandlerFunc<YAP_BROADCAST_EVT_TARGS(YapConversationHandler, OnConversationPlayerPromptChosen, Execute_K2_ConversationPlayerPromptChosen)>(This->ConversationHandlers, Data);

	return true;
}

// ------------------------------------------------------------------------------------------------

bool UYapSubsystem::SkipDialogue(const FYapDialogueHandleRef& Handle)
{
	// TODO handle invalid handles gracefully
	TWeakObjectPtr<UFlowNode_YapDialogue>* DialogueWeak = Get()->GuidDialogueMap.Find(Handle);

	if (DialogueWeak)
	{
		return (*DialogueWeak)->SkipCurrent();
	}

	return false;
}

// ------------------------------------------------------------------------------------------------

FYapDialogueHandle& UYapSubsystem::GetDialogueHandle(FYapDialogueHandleRef HandleRef)
{
	TWeakObjectPtr<UFlowNode_YapDialogue>* DialoguePtr = Get()->GuidDialogueMap.Find(HandleRef);
	
	if (DialoguePtr)
	{
		return (*DialoguePtr)->GetDialogueHandle();
	}

	return FYapDialogueHandle::InvalidHandle();
}

// ------------------------------------------------------------------------------------------------

void UYapSubsystem::RegisterCharacterComponent(UYapCharacterComponent* YapCharacterComponent)
{
	AActor* Actor = YapCharacterComponent->GetOwner();

	if (RegisteredYapCharacterActors.Contains(Actor))
	{
		UE_LOG(LogYap, Error, TEXT("Multiple character components on actor, ignoring! Actor: %s"), *Actor->GetName());
		return;
	}

	YapCharacterComponents.Add(YapCharacterComponent->GetCharacterTag(), YapCharacterComponent);
	
	RegisteredYapCharacterActors.Add(Actor);
}

void UYapSubsystem::UnregisterCharacterComponent(UYapCharacterComponent* YapCharacterComponent)
{
	AActor* Actor = YapCharacterComponent->GetOwner();

	YapCharacterComponents.Remove(YapCharacterComponent->GetCharacterTag());
	RegisteredYapCharacterActors.Remove(Actor);
}

// ------------------------------------------------------------------------------------------------

void UYapSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	World = Cast<UWorld>(GetOuter());

	// TODO async load these
	ActiveConversation.OnConversationOpens.BindUObject(this, &UYapSubsystem::OnConversationOpens_Internal);
	ActiveConversation.OnConversationCloses.BindUObject(this, &UYapSubsystem::OnConversationCloses_Internal);

	// TODO handle null unset values
	TSubclassOf<UYapBroker> BrokerClass = UYapProjectSettings::GetBrokerClass().LoadSynchronous();

	if (BrokerClass)
	{
		Broker = NewObject<UYapBroker>(this, BrokerClass);
	}

	bGetGameMaturitySettingWarningIssued = false;
}

// ------------------------------------------------------------------------------------------------

void UYapSubsystem::Deinitialize()
{
	World = nullptr;
}

// ------------------------------------------------------------------------------------------------

void UYapSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	if (IsValid(Broker))
	{
		Broker->Initialize_Internal();
	}
}

// ------------------------------------------------------------------------------------------------

void UYapSubsystem::OnConversationOpens_Internal(const FGameplayTag& ConversationName)
{
	FYapData_ConversationChatOpened Data;
	Data.Conversation = ConversationName;
	
	BroadcastEventHandlerFunc<YAP_BROADCAST_EVT_TARGS(YapConversationHandler, OnConversationChatOpened, Execute_K2_ConversationChatOpened)>(ConversationHandlers, Data);
}

// ------------------------------------------------------------------------------------------------

void UYapSubsystem::OnConversationCloses_Internal(const FGameplayTag& ConversationName)
{
	FYapData_ConversationChatClosed Data;
	Data.Conversation = ConversationName;
	
	BroadcastEventHandlerFunc<YAP_BROADCAST_EVT_TARGS(YapConversationHandler, OnConversationChatClosed, Execute_K2_ConversationChatClosed)>(ConversationHandlers, Data);
}

// ------------------------------------------------------------------------------------------------

bool UYapSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::GamePreview || WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

#undef LOCTEXT_NAMESPACE