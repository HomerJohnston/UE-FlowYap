// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license.

#include "Yap/IYapConversationHandler.h"

#include "Yap/YapLog.h"
#include "Yap/YapProjectSettings.h"

class UYapCharacter;

#define LOCTEXT_NAMESPACE "Yap"

void IYapConversationHandler::K2_OnConversationOpened_Implementation(const FGameplayTag& Conversation)
{
	UE_LOG(LogYap, Warning, TEXT("Unimplemented conversation handler function: %s"), *FString(__func__));
}

void IYapConversationHandler::K2_OnConversationClosed_Implementation(const FGameplayTag& Conversation)
{
	UE_LOG(LogYap, Warning, TEXT("Unimplemented conversation handler function: %s"), *FString(__func__));
}

void IYapConversationHandler::K2_OnDialogueBegins_Implementation(const FGameplayTag& Conversation, FYapDialogueHandle DialogueHandle, const UYapCharacter* DirectedAt, const UYapCharacter* Speaker, const FGameplayTag& MoodKey, const FText& DialogueText, const FText& TitleText, float DialogueTime, const UObject* DialogueAudioAsset)
{
	UE_LOG(LogYap, Warning, TEXT("Unimplemented conversation handler function: %s"), *FString(__func__));
}

void IYapConversationHandler::K2_OnDialogueEnds_Implementation(const FGameplayTag& Conversation, FYapDialogueHandle DialogueHandle, float PaddingTime)
{
	UE_LOG(LogYap, Warning, TEXT("Unimplemented conversation handler function: %s"), *FString(__func__));
}

void IYapConversationHandler::K2_AddPlayerPrompt_Implementation(const FGameplayTag& Conversation, FYapPromptHandle Handle, const UYapCharacter* DirectedAt, const UYapCharacter* Speaker, const FGameplayTag& MoodKey, const FText& DialogueText, const FText& TitleText)
{
	UE_LOG(LogYap, Warning, TEXT("Unimplemented conversation handler function: %s"), *FString(__func__));
}

void IYapConversationHandler::K2_AfterPlayerPromptsAdded_Implementation(const FGameplayTag& Conversation)
{
	UE_LOG(LogYap, Warning, TEXT("Unimplemented conversation handler function: %s"), *FString(__func__));
}

#undef LOCTEXT_NAMESPACE