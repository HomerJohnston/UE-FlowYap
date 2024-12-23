#pragma once

struct FGameplayTag;
struct FYapPromptHandle;
struct FYapDialogueHandle;
struct FYapBit;

#include "Yap/YapPromptHandle.h"
#include "Yap/YapDialogueHandle.h"

#include "YapConversationListenerInterface.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UYapConversationListenerInterface : public UInterface
{
	GENERATED_BODY()
};

class IYapConversationListenerInterface
{
	GENERATED_BODY()

public:
	/**  */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void OnConversationBegins(const FGameplayTag& Conversation);

	/**  */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void OnConversationEnds(const FGameplayTag& Conversation);

	/**  */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void OnDialogueBegins(const FGameplayTag& Conversation, FYapDialogueHandle DialogueHandle, const UYapCharacter* Character, const FGameplayTag& MoodKey, const FText& DialogueText, double DialogueTime, const UObject* DialogueAudioAsset);
	
	/**  */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void OnDialogueEnds(const FGameplayTag& Conversation, FYapDialogueHandle DialogueHandle);

	/**  */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void OnPromptOptionAdded(const FGameplayTag& Conversation, const FYapBit& DialogueInfo, FYapPromptHandle Handle);

	/**  */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void OnPromptOptionsAllAdded(const FGameplayTag& Conversation);
};
