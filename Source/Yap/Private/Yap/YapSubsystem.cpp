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

bool FYapActiveConversation::StartConversation(UFlowAsset* InOwningAsset, const FGameplayTag& InConversation)
{
	if (Conversation.IsSet())
	{
		UE_LOGFMT(LogYap, Warning, "Tried to start conversation {0} but conversation {1} was already ongoing. Ignoring start request.", InConversation.ToString(), Conversation.GetValue().ToString());
		return false;
	}
	
	FlowAsset = InOwningAsset;
	Conversation = InConversation;

	(void)OnConversationStarts.ExecuteIfBound(Conversation.GetValue());
	
	return true;
}

bool FYapActiveConversation::EndConversation()
{
	if (Conversation.IsSet())
	{
		(void)OnConversationEnds.ExecuteIfBound(Conversation.GetValue());

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
		UE_LOG(LogYap, Error, TEXT("Tried to register a conversation handler, but it does not implement the FlowYapConversationHandler interface!"));
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

UYapBroker* UYapSubsystem::GetConversationBroker()
{
	UYapBroker* Broker = Get()->ConversationBroker;

#if WITH_EDITOR
	ensureMsgf(IsValid(Broker), TEXT("Conversation Broker is invalid. Did you create one and assign it in project settings? Docs: https://github.com/HomerJohnston/UE-FlowYap/wiki/Conversation-Broker"));
#endif
	
	return Broker;		
}

EYapMaturitySetting UYapSubsystem::GetGameMaturitySetting()
{
	EYapMaturitySetting MaturitySetting = EYapMaturitySetting::Unspecified;
	
	if (!ensureMsgf(World.IsValid(), TEXT("World was invalid in UYapSubsystem::GetGameMaturitySetting(); this should not happen! Returning default project setting.")))
	{
		MaturitySetting = UYapProjectSettings::GetDefaultMaturitySetting(); 
	}
	else
	{
		UYapBroker* Broker = GetConversationBroker();

		if (ensureMsgf(IsValid(Broker), TEXT("No Conversation Broker in UYapSubsystem::GetGameMaturitySetting(); returning default project setting.")))
		{
			MaturitySetting = Broker->UseMatureDialogue();
		}
		else
		{
			MaturitySetting = UYapProjectSettings::GetDefaultMaturitySetting();
		}	
	}

	if (MaturitySetting == EYapMaturitySetting::Unspecified)
	{
		if (!bGetGameMaturitySettingWarningIssued)
		{
			UE_LOG(LogYap, Error, TEXT("UYapSubsystem::GetGameMaturitySetting failed to get a valid game maturity setting! Defaulting to mature. This could be caused by a faulty Broker implementation, or corrupt project settings?"));
			bGetGameMaturitySettingWarningIssued = true;			
		}
		MaturitySetting = EYapMaturitySetting::Mature;
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
	const FYapFragment& Fragment = Dialogue->GetFragmentByIndex(FragmentIndex);
	const FYapBit& Bit = Fragment.GetBit();

	FGameplayTag ConversationName;

	if (ActiveConversation.FlowAsset == Dialogue->GetFlowAsset())
	{
		ConversationName = ActiveConversation.Conversation.GetValue();
	}

	FYapPromptHandle Handle(Dialogue, FragmentIndex);

	EYapMaturitySetting MaturitySetting = GetGameMaturitySetting();
	
	BroadcastBrokerListenerFuncs<&UYapBroker::AddPlayerPrompt, &IYapConversationListener::Execute_K2_OnPromptOptionAdded>
		(ConversationName, Handle, Bit.GetDirectedAt(), Bit.GetSpeaker(), Bit.GetMoodKey(), Bit.GetDialogueText(MaturitySetting), Bit.GetTitleText(MaturitySetting));
}

void UYapSubsystem::OnFinishedBroadcastingPrompts()
{
	FGameplayTag ConversationName = ActiveConversation.IsConversationInProgress() ? ActiveConversation.Conversation.GetValue() : FGameplayTag::EmptyTag;

	BroadcastBrokerListenerFuncs<&UYapBroker::AfterPlayerPromptsAdded, &IYapConversationListener::Execute_K2_OnPromptOptionsAllAdded>
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
	ActiveConversation.OnConversationStarts.BindUObject(this, &UYapSubsystem::OnConversationStarts_Internal);
	ActiveConversation.OnConversationEnds.BindUObject(this, &UYapSubsystem::OnConversationEnds_Internal);

	// TODO handle null unset values
	// TODO deprecate the text calculator class? everything done in the broker now
	ConversationBrokerClass = UYapProjectSettings::GetConversationBrokerClass().LoadSynchronous();

	if (ConversationBrokerClass)
	{
		ConversationBroker = NewObject<UYapBroker>(this, ConversationBrokerClass);
		ConversationBroker->Initialize();
	}

	TArray<TSoftClassPtr<UObject>> DialogueAudioAssetClassesSoft = UYapProjectSettings::GetDialogueAssetClasses();

	for (const TSoftClassPtr<UObject>& Class : DialogueAudioAssetClassesSoft)
	{
		DialogueAudioAssetClasses.Add(Class.LoadSynchronous()); // TODO async loading
	}

	bGetGameMaturitySettingWarningIssued = false;
}

void UYapSubsystem::Deinitialize()
{
	World = nullptr;
}


void UYapSubsystem::OnConversationStarts_Internal(const FGameplayTag& ConversationName)
{
	BroadcastBrokerListenerFuncs<&UYapBroker::OnConversationOpened, &IYapConversationListener::Execute_K2_OnConversationBegins>
		(ConversationName);
}

void UYapSubsystem::OnConversationEnds_Internal(const FGameplayTag& ConversationName)
{
	BroadcastBrokerListenerFuncs<&UYapBroker::OnConversationClosed, &IYapConversationListener::Execute_K2_OnConversationEnds>
		(ConversationName);
}

bool UYapSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::GamePreview || WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

#undef LOCTEXT_NAMESPACE