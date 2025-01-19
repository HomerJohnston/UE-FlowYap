// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

// ================================================================================================

#include "Yap/YapSubsystem.h"

#include "Logging/StructuredLog.h"
#include "Yap/YapBroker.h"
#include "Yap/YapFragment.h"
#include "Yap/YapLog.h"
#include "Yap/IYapConversationListener.h"
#include "Yap/YapDialogueHandle.h"
#include "Yap/YapProjectSettings.h"
#include "Yap/YapPromptHandle.h"
#include "Yap/Nodes/FlowNode_YapDialogue.h"

#define LOCTEXT_NAMESPACE "Yap"

TWeakObjectPtr<UWorld> UYapSubsystem::World = nullptr;
bool UYapSubsystem::bGetGameMaturitySettingWarningIssued = false;

FYapActiveConversation::FYapActiveConversation()
{
	FlowAsset = nullptr;
	Conversation.Reset();
}

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

// ================================================================================================

UYapSubsystem::UYapSubsystem()
{
}

void UYapSubsystem::RegisterConversationListener(UObject* NewListener)
{
	if (NewListener->Implements<UYapConversationListener>())
	{
		Listeners.AddUnique(NewListener);
	}
	else
	{
		UE_LOG(LogYap, Error, TEXT("Tried to register a conversation listener, but object does not implement the YapConversationListener interface! [%s]"), *NewListener->GetName());
	}
}

void UYapSubsystem::UnregisterConversationListener(UObject* RemovedListener)
{
	Listeners.Remove(RemovedListener);
}

UYapCharacterComponent* UYapSubsystem::GetYapCharacter(const FGameplayTag& CharacterTag)
{
	TWeakObjectPtr<UYapCharacterComponent>* CharacterComponentPtr = YapCharacterComponents.Find(CharacterTag);

	if (CharacterComponentPtr && CharacterComponentPtr->IsValid())
	{
		return CharacterComponentPtr->Get();
	}

	return nullptr;
}

UYapBroker* UYapSubsystem::GetBroker()
{
	UYapBroker* Broker = Get()->Broker;

#if WITH_EDITOR
	ensureMsgf(IsValid(Broker), TEXT("Conversation Broker is invalid. Did you create one and assign it in project settings? Docs: https://github.com/HomerJohnston/UE-FlowYap/wiki/Conversation-Broker"));
#endif
	
	return Broker;		
}

EYapMaturitySetting UYapSubsystem::GetGameMaturitySetting()
{
	EYapMaturitySetting MaturitySetting;
	
	if (!ensureMsgf(World.IsValid(), TEXT("World was invalid in UYapSubsystem::GetGameMaturitySetting(). This should never happen! Using default project maturity setting.")))
	{
		MaturitySetting = UYapProjectSettings::GetDefaultMaturitySetting(); 
	}
	else
	{
		UYapBroker* Broker = GetBroker();

		if (ensureMsgf(IsValid(Broker), TEXT("No broker set in project settings! Using default project maturity setting.")))
		{
			MaturitySetting = Broker->UseMatureDialogue();
		}
		else
		{
			MaturitySetting = UYapProjectSettings::GetDefaultMaturitySetting();
		}	
	}

	// Something went wrong... we will try to grab it from project settings with error logging. If even that fails, we will hard-code default to mature.
	if (MaturitySetting == EYapMaturitySetting::Unspecified)
	{
		bool bSetWarningIssued = false;

		if (!bGetGameMaturitySettingWarningIssued)
		{
			UE_LOG(LogYap, Error, TEXT("UYapSubsystem::GetGameMaturitySetting failed to get a valid game maturity setting! Using default project maturity setting. This could be caused by a faulty Broker implementation."));
			bSetWarningIssued = true;
		}
		
		MaturitySetting = UYapProjectSettings::GetDefaultMaturitySetting();

		if (MaturitySetting == EYapMaturitySetting::Unspecified)
		{
			if (!bGetGameMaturitySettingWarningIssued)
			{
				UE_LOG(LogYap, Error, TEXT("UYapSubsystem::GetGameMaturitySetting failed to get a valid game maturity setting! Defaulting to mature. This could be caused by corrupt project settings."));
				bSetWarningIssued = true;
			}
		
			MaturitySetting = EYapMaturitySetting::Mature;
		}
		
		if (bSetWarningIssued)
		{
			bGetGameMaturitySettingWarningIssued = true;
		}
	}
	
	return MaturitySetting;
}

void UYapSubsystem::RegisterTaggedFragment(const FGameplayTag& FragmentTag, UFlowNode_YapDialogue* DialogueNode)
{
	if (TaggedFragments.Contains(FragmentTag))
	{
		UE_LOG(LogYap, Warning, TEXT("Tried to register tagged fragment with tag [%s] but this tag was already registered! Find and fix the duplicate tag usage."), *FragmentTag.ToString()); // TODO if I pass in the full fragment I could log the dialogue text to make this easier for designers?
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

bool UYapSubsystem::OpenConversation(UFlowAsset* OwningAsset, const FGameplayTag& Conversation)
{
	return ActiveConversation.OpenConversation(OwningAsset, Conversation);
}

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

void UYapSubsystem::BroadcastPrompt(UFlowNode_YapDialogue* Dialogue, uint8 FragmentIndex)
{
	const FYapFragment& Fragment = Dialogue->GetFragmentByIndex(FragmentIndex);
	const FYapBit& Bit = Fragment.GetBit();

	FGameplayTag ConversationName;

	if (ActiveConversation.FlowAsset == Dialogue->GetFlowAsset())
	{
		ConversationName = ActiveConversation.Conversation.GetValue();
	}

	FYapPromptHandle Handle(Dialogue, FragmentIndex);

	EYapMaturitySetting MaturitySetting = GetGameMaturitySetting();
	
	BroadcastBrokerListenerFuncs<&UYapBroker::AddPlayerPrompt, &IYapConversationListener::Execute_K2_AddPlayerPrompt>
		(ConversationName, Handle, Bit.GetDirectedAt(), Bit.GetSpeaker(), Bit.GetMoodKey(), Bit.GetDialogueText(MaturitySetting), Bit.GetTitleText(MaturitySetting));
}

void UYapSubsystem::OnFinishedBroadcastingPrompts()
{
	FGameplayTag ConversationName = ActiveConversation.IsConversationInProgress() ? ActiveConversation.Conversation.GetValue() : FGameplayTag::EmptyTag;

	BroadcastBrokerListenerFuncs<&UYapBroker::AfterPlayerPromptsAdded, &IYapConversationListener::Execute_K2_AfterPlayerPromptAdded>
		(ConversationName);
}

void UYapSubsystem::BroadcastDialogueStart(UFlowNode_YapDialogue* DialogueNode, uint8 FragmentIndex)
{
	const FYapFragment& Fragment = DialogueNode->GetFragmentByIndex(FragmentIndex);
	const FYapBit& Bit = Fragment.GetBit();

	FGameplayTag ConversationName;

	if (ActiveConversation.FlowAsset == DialogueNode->GetFlowAsset())
	{
		ConversationName = ActiveConversation.Conversation.GetValue();
	}

	bool bSkippable = Bit.GetSkippable(DialogueNode);

	FYapDialogueHandle DialogueHandle(DialogueNode, FragmentIndex, bSkippable);

	EYapMaturitySetting MaturitySetting = GetGameMaturitySetting();

	TOptional<float> Time = Bit.GetTime();

	float EffectiveTime;
	
	if (Time.IsSet())
	{
		EffectiveTime = Time.GetValue();
	}
	else
	{
		EffectiveTime = UYapProjectSettings::GetMinimumFragmentTime();
		UE_LOG(LogYap, Error, TEXT("Fragment failed to return a valid time! Dialogue: %s"), *Bit.GetDialogueText(MaturitySetting).ToString());
	}
	
	BroadcastBrokerListenerFuncs<&UYapBroker::OnDialogueBegins, &IYapConversationListener::Execute_K2_OnDialogueBegins>
		(ConversationName, DialogueHandle, Bit.GetDirectedAt(), Bit.GetSpeaker(), Bit.GetMoodKey(), Bit.GetDialogueText(MaturitySetting), Bit.GetTitleText(MaturitySetting), EffectiveTime, Bit.GetAudioAsset<UObject>(MaturitySetting));
}

void UYapSubsystem::BroadcastDialogueEnd(const UFlowNode_YapDialogue* OwnerDialogue, uint8 FragmentIndex)
{
	FGameplayTag ConversationName;

	if (ActiveConversation.FlowAsset == OwnerDialogue->GetFlowAsset())
	{
		ConversationName = ActiveConversation.Conversation.GetValue();
	}

	FYapDialogueHandle DialogueHandle(OwnerDialogue, FragmentIndex, false);

	BroadcastBrokerListenerFuncs<&UYapBroker::OnDialogueEnds, &IYapConversationListener::Execute_K2_OnDialogueEnds>
		(ConversationName, DialogueHandle);
}

void UYapSubsystem::RunPrompt(const FYapPromptHandle& Handle)
{
	Handle.DialogueNode->RunPrompt(Handle.FragmentIndex);
}

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

// =====================================================================================================

void UYapSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	World = Cast<UWorld>(GetOuter());

	// TODO async load these
	ActiveConversation.OnConversationOpens.BindUObject(this, &UYapSubsystem::OnConversationOpens_Internal);
	ActiveConversation.OnConversationCloses.BindUObject(this, &UYapSubsystem::OnConversationCloses_Internal);

	// TODO handle null unset values
	// TODO deprecate the text calculator class? everything done in the broker now
	BrokerClass = UYapProjectSettings::GetBrokerClass().LoadSynchronous();

	if (BrokerClass)
	{
		Broker = NewObject<UYapBroker>(this, BrokerClass);
		Broker->Initialize();
	}

	bGetGameMaturitySettingWarningIssued = false;
}

void UYapSubsystem::Deinitialize()
{
	World = nullptr;
}


void UYapSubsystem::OnConversationOpens_Internal(const FGameplayTag& ConversationName)
{
	BroadcastBrokerListenerFuncs<&UYapBroker::OnConversationOpened, &IYapConversationListener::Execute_K2_OnConversationOpened>
		(ConversationName);
}

void UYapSubsystem::OnConversationCloses_Internal(const FGameplayTag& ConversationName)
{
	BroadcastBrokerListenerFuncs<&UYapBroker::OnConversationClosed, &IYapConversationListener::Execute_K2_OnConversationClosed>
		(ConversationName);
}

bool UYapSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::GamePreview || WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

#undef LOCTEXT_NAMESPACE