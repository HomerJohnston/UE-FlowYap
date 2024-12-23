#include "Yap/YapConversationBrokerBase.h"

#include "Yap/YapDialogueHandle.h"
#include "Yap/YapLog.h"

void UYapConversationBrokerBase::OnConversationBegins_Implementation(const FGameplayTag& Conversation)
{
	UE_LOG(LogYap, Warning, TEXT("Unimplemented broker function: %s"), *FString(__func__));
}

void UYapConversationBrokerBase::OnConversationEnds_Implementation(const FGameplayTag& Conversation)
{
	UE_LOG(LogYap, Warning, TEXT("Unimplemented broker function: %s"), *FString(__func__));
}

void UYapConversationBrokerBase::OnDialogueBegins_Implementation(const FGameplayTag& Conversation, FYapDialogueHandle DialogueHandle, const UYapCharacter* Character, const FGameplayTag& MoodKey, const FText& DialogueText, double DialogueTime, const UObject* DialogueAudioAsset)
{
	UE_LOG(LogYap, Warning, TEXT("Unimplemented broker function: %s"), *FString(__func__));
}

void UYapConversationBrokerBase::OnDialogueEnds_Implementation(const FGameplayTag& Conversation, FYapDialogueHandle DialogueHandle)
{
	UE_LOG(LogYap, Warning, TEXT("Unimplemented broker function: %s"), *FString(__func__));
}

void UYapConversationBrokerBase::OnPromptOptionAdded_Implementation(const FGameplayTag& Conversation, const FYapBit& DialogueInfo, FYapPromptHandle Handle)
{
	UE_LOG(LogYap, Warning, TEXT("Unimplemented broker function: %s"), *FString(__func__));
}

void UYapConversationBrokerBase::OnPromptOptionsAllAdded_Implementation(const FGameplayTag& Conversation)
{
	UE_LOG(LogYap, Warning, TEXT("Unimplemented broker function: %s"), *FString(__func__));
}
