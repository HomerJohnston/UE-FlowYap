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
	UFUNCTION(BlueprintNativeEvent)
	void OnConversationBegins(const FGameplayTag& Conversation);

	/**  */
	UFUNCTION(BlueprintNativeEvent)
	void OnConversationEnds(const FGameplayTag& Conversation);

	/**  */
	UFUNCTION(BlueprintNativeEvent)
	void OnDialogueBegins(const FGameplayTag& Conversation, FYapDialogueHandle DialogueHandle, const UYapCharacter* Speaker, const FGameplayTag& MoodKey, const FText& DialogueText, double DialogueTime, const UObject* DialogueAudioAsset, bool bSkippable);
	
	/**  */
	UFUNCTION(BlueprintNativeEvent)
	void OnDialogueEnds(const FGameplayTag& Conversation, FYapDialogueHandle DialogueHandle);

	/**  */
	UFUNCTION(BlueprintNativeEvent)
	void OnPromptOptionAdded(const FGameplayTag& Conversation, FYapPromptHandle Handle, const UYapCharacter* Speaker, const FGameplayTag& MoodKey, const FText& DialogueText, const FText& TitleText);

	/**  */
	UFUNCTION(BlueprintNativeEvent)
	void OnPromptOptionsAllAdded(const FGameplayTag& Conversation);
};
