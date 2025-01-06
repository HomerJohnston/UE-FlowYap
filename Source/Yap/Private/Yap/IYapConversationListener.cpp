﻿// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license.

#include "Yap/IYapConversationListener.h"

#include "Yap/YapLog.h"
#include "Yap/YapProjectSettings.h"

void IYapConversationListener::K2_OnConversationBegins_Implementation(const FGameplayTag& Conversation)
{
	UE_LOG(LogYap, Warning, TEXT("Unimplemented listener function: %s"), *FString(__func__));
}

void IYapConversationListener::K2_OnConversationEnds_Implementation(const FGameplayTag& Conversation)
{
	UE_LOG(LogYap, Warning, TEXT("Unimplemented listener function: %s"), *FString(__func__));
}

void IYapConversationListener::K2_OnDialogueBegins_Implementation(const FGameplayTag& Conversation, FYapDialogueHandle DialogueHandle, const UYapCharacter* Speaker, const FGameplayTag& MoodKey, const FText& DialogueText, float DialogueTime, const UObject* DialogueAudioAsset, const UYapCharacter* DirectedAt)
{
	UE_LOG(LogYap, Warning, TEXT("Unimplemented listener function: %s"), *FString(__func__));
}

void IYapConversationListener::K2_OnDialogueEnds_Implementation(const FGameplayTag& Conversation, FYapDialogueHandle DialogueHandle)
{
	UE_LOG(LogYap, Warning, TEXT("Unimplemented listener function: %s"), *FString(__func__));
}

void IYapConversationListener::K2_OnPromptOptionAdded_Implementation(const FGameplayTag& Conversation, FYapPromptHandle Handle, const UYapCharacter* Speaker, const FGameplayTag& MoodKey, const FText& DialogueText, const FText& TitleText)
{
	UE_LOG(LogYap, Warning, TEXT("Unimplemented listener function: %s"), *FString(__func__));
}

void IYapConversationListener::K2_OnPromptOptionsAllAdded_Implementation(const FGameplayTag& Conversation)
{
	UE_LOG(LogYap, Warning, TEXT("Unimplemented listener function: %s"), *FString(__func__));
}

EYapMaturitySetting IYapConversationListener::K2_UseMatureDialogue_Implementation()
{
#if WITH_EDITOR
	if (!bWarnedAboutMatureDialogue)
	{
		if (!UYapProjectSettings::Get()->GetSuppressMatureWarning())
		{
			UE_LOG(LogYap, Warning, TEXT("Unimplemented listener function: %s\nDefaulting to use mature language; child-safe language will never be displayed.\nYou can suppress this warning in project settings."), *FString(__func__));
		}

		bWarnedAboutMatureDialogue = true;
	}
#endif
	return UYapProjectSettings::GetDefaultMaturitySetting();
}
