// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "Yap/YapBrokerBase.h"

#include "Yap/YapDialogueHandle.h"
#include "Yap/YapLog.h"
#include "Yap/YapProjectSettings.h"
#include "Yap/YapPromptHandle.h"

#define LOCTEXT_NAMESPACE "Yap"

// ------------------------------------------------------------------------------------------------

void UYapBrokerBase::K2_OnConversationBegins_Implementation(const FGameplayTag& Conversation)
{
	UE_LOG(LogYap, Warning, TEXT("Unimplemented broker function: %s"), *FString(__func__));
}

// ------------------------------------------------------------------------------------------------

void UYapBrokerBase::K2_OnConversationEnds_Implementation(const FGameplayTag& Conversation)
{
	UE_LOG(LogYap, Warning, TEXT("Unimplemented broker function: %s"), *FString(__func__));
}

// ------------------------------------------------------------------------------------------------

void UYapBrokerBase::K2_OnDialogueBegins_Implementation(const FGameplayTag& Conversation, FYapDialogueHandle DialogueHandle, const UYapCharacter* Speaker, const FGameplayTag& MoodKey, const FText& DialogueText, float DialogueTime, const UObject* DialogueAudioAsset, const UYapCharacter* DirectedAt)
{
	UE_LOG(LogYap, Warning, TEXT("Unimplemented broker function: %s"), *FString(__func__));
}

// ------------------------------------------------------------------------------------------------

void UYapBrokerBase::K2_OnDialogueEnds_Implementation(const FGameplayTag& Conversation, FYapDialogueHandle DialogueHandle)
{
	UE_LOG(LogYap, Warning, TEXT("Unimplemented broker function: %s"), *FString(__func__));
}

// ------------------------------------------------------------------------------------------------

void UYapBrokerBase::K2_OnPromptOptionAdded_Implementation(const FGameplayTag& Conversation, FYapPromptHandle Handle, const UYapCharacter* Speaker, const FGameplayTag& MoodKey, const FText& DialogueText, const FText& TitleText)
{
	UE_LOG(LogYap, Warning, TEXT("Unimplemented broker function: %s"), *FString(__func__));
}

// ------------------------------------------------------------------------------------------------

void UYapBrokerBase::K2_OnPromptOptionsAllAdded_Implementation(const FGameplayTag& Conversation)
{
	UE_LOG(LogYap, Warning, TEXT("Unimplemented broker function: %s"), *FString(__func__));
}

// ------------------------------------------------------------------------------------------------

EYapMaturitySetting UYapBrokerBase::K2_UseMatureDialogue_Implementation()
{
#if WITH_EDITOR
	if (!bWarnedAboutMatureDialogue)
	{
		if (!UYapProjectSettings::GetSuppressMatureWarning())
		{
			UE_LOG(LogYap, Warning, TEXT("Unimplemented broker function: %s\nDefaulting to use mature language; child-safe language will never be displayed.\nYou can suppress this warning in project settings."), *FString(__func__));
		}

		bWarnedAboutMatureDialogue = true;
	}
#endif
	return UYapProjectSettings::GetDefaultMaturitySetting();
}

// ================================================================================================
#if WITH_EDITOR

// Used to check to see if a derived class actually implemented PlayDialogueAudioAsset_Editor()
thread_local bool bPreviewDialogueAudioOverridden = false;
thread_local bool bSuppressPreviewDialogueAudioWarning = false;

bool UYapBrokerBase::K2_PreviewDialogueAudio_Implementation(const UObject* AudioAsset) const
{
	bPreviewDialogueAudioOverridden = false;

	if (!bSuppressPreviewDialogueAudioWarning)
	{
		UE_LOG(LogYap, Warning, TEXT("Unimplemented broker function: %s"), *FString(__func__));
	}
	
	return false;
}

// ------------------------------------------------------------------------------------------------

float UYapBrokerBase::K2_GetDialogueAudioDuration_Implementation(const UObject* AudioAsset) const
{
	UE_LOG(LogYap, Warning, TEXT("Unimplemented broker function: %s"), *FString(__func__));

	return -1;
}

bool UYapBrokerBase::PreviewDialogueAudio_Internal(const UObject* AudioAsset) const
{
	bPreviewDialogueAudioOverridden = true;

	return PreviewDialogueAudio(AudioAsset);
}

bool UYapBrokerBase::ImplementsPreviewDialogueAudio() const
{
	bSuppressPreviewDialogueAudioWarning = true;
	(void)PreviewDialogueAudio_Internal(nullptr);
	bSuppressPreviewDialogueAudioWarning = false;

	return bPreviewDialogueAudioOverridden;
}

#endif // WITH_EDITOR

#undef LOCTEXT_NAMESPACE