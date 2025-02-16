// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "Yap/YapBlueprintFunctionLibrary.h"

#if WITH_EDITOR
#include "AssetTypeActions/AssetDefinition_SoundBase.h"
#endif

#include "Yap/YapCharacter.h"
#include "Yap/YapDialogueHandle.h"
#include "Yap/YapLog.h"
#include "Yap/YapPromptHandle.h"
#include "Yap/YapSubsystem.h"
#include "Yap/Nodes/FlowNode_YapDialogue.h"

#define LOCTEXT_NAMESPACE "Yap"

// ------------------------------------------------------------------------------------------------

#if WITH_EDITOR
void UYapBlueprintFunctionLibrary::PlaySoundInEditor(USoundBase* Sound)
{
	if (Sound)
	{
		GEditor->PlayPreviewSound(Sound);
	}
	else
	{
		UE_LOG(LogYap, Warning, TEXT("Sound was null"));
	}
}
#endif

// ------------------------------------------------------------------------------------------------

float UYapBlueprintFunctionLibrary::GetSoundLength(USoundBase* Sound)
{
	return Sound->Duration;
}

// ------------------------------------------------------------------------------------------------

bool UYapBlueprintFunctionLibrary::SkipDialogue(const FYapDialogueHandleRef& Handle)
{
	if (Handle.IsValid())
	{
		if (!UYapSubsystem::SkipDialogue(Handle))
		{
			UE_LOG(LogYap, Display, TEXT("Failed to skip dialogue!"))
		}
	}
	else
	{
		UE_LOG(LogYap, Warning, TEXT("Attempted to skip with invalid handle!"))
	}
	
	return false;
}

// ------------------------------------------------------------------------------------------------

bool UYapBlueprintFunctionLibrary::RunPrompt(const FYapPromptHandle& Handle)
{
	return UYapSubsystem::RunPrompt(Handle);
}

// ------------------------------------------------------------------------------------------------

void UYapBlueprintFunctionLibrary::AddReactor(FYapDialogueHandleRef& HandleRef, UObject* Reactor)
{
	FYapDialogueHandle& Handle = UYapSubsystem::GetDialogueHandle(HandleRef);

	if (Handle.IsValid())
	{
		Handle.AddReactor(Reactor);
	}
	else
	{
		UE_LOG(LogYap, Warning, TEXT("Could not find valid handle to add reactor to!"))
	}
}

const TArray<FInstancedStruct>& UYapBlueprintFunctionLibrary::GetFragmentData(const FYapDialogueHandleRef& HandleRef)
{
	const FYapDialogueHandle& Handle = UYapSubsystem::GetDialogueHandle(HandleRef);

	const UFlowNode_YapDialogue* DialogueNode = Handle.GetDialogueNode();

	const FYapFragment& Fragment = DialogueNode->GetFragments()[Handle.GetFragmentIndex()];
	
	return Fragment.GetData();
}

// ------------------------------------------------------------------------------------------------

void UYapBlueprintFunctionLibrary::RegisterConversationHandler(UObject* NewHandler)
{
	UYapSubsystem::RegisterConversationHandler(NewHandler);
}

// ------------------------------------------------------------------------------------------------

void UYapBlueprintFunctionLibrary::RegisterFreeSpeechHandler(UObject* NewHandler)
{
	UYapSubsystem::RegisterFreeSpeechHandler(NewHandler);
}

// ------------------------------------------------------------------------------------------------

void UYapBlueprintFunctionLibrary::UnregisterConversationHandler(UObject* HandlerToUnregister)
{
	UYapSubsystem::UnregisterConversationHandler(HandlerToUnregister);
}

// ------------------------------------------------------------------------------------------------

void UYapBlueprintFunctionLibrary::UnregisterFreeSpeechHandler(UObject* HandlerToUnregister)
{
	UYapSubsystem::UnregisterFreeSpeechHandler(HandlerToUnregister);
}

// ------------------------------------------------------------------------------------------------

AActor* UYapBlueprintFunctionLibrary::FindYapCharacterActor(const UYapCharacter* Character)
{
	if (!IsValid(Character))
	{
		return nullptr;
	}

	if (!Character->GetIdentityTag().IsValid())
	{
		return nullptr;
	}
	
	UYapCharacterComponent* Comp = UYapSubsystem::FindCharacterComponent(Character->GetIdentityTag());

	if (!IsValid(Comp))
	{
		return nullptr;
	}

	return Comp->GetOwner();
}

// ------------------------------------------------------------------------------------------------

void UYapBlueprintFunctionLibrary::TriggerConversationOpen()
{
	UYapSubsystem::Get()->ConversationOpenTrigger.Broadcast();
}

// ------------------------------------------------------------------------------------------------

#undef LOCTEXT_NAMESPACE
