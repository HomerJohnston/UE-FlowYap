#pragma once

#include "YapConversationListenerInterface.h"

#include "YapConversationBrokerBase.generated.h"

/** Optional base class for brokering Yap to your game. Create a child class of this and the functions to create conversation panels and/or display floating text widgets in your game. Then set Yap's project settings to use your class. */
UCLASS(Abstract)
class UYapConversationBrokerBase : public UObject
{
	GENERATED_BODY()

#if WITH_EDITOR
public:
	bool ImplementsGetWorld() const override { return true; }
#endif
	
public:

	UFUNCTION(BlueprintNativeEvent)
	void OnConversationBegins(const FGameplayTag& Conversation);

	UFUNCTION(BlueprintNativeEvent)
	void OnConversationEnds(const FGameplayTag& Conversation);

	UFUNCTION(BlueprintNativeEvent)
	void OnDialogueBegins(const FGameplayTag& Conversation, FYapDialogueHandle DialogueHandle, const UYapCharacter* Speaker, const FGameplayTag& MoodKey, const FText& DialogueText, double DialogueTime, const UObject* DialogueAudioAsset, bool bSkippable);

	UFUNCTION(BlueprintNativeEvent)
	void OnDialogueEnds(const FGameplayTag& Conversation, FYapDialogueHandle DialogueHandle);

	UFUNCTION(BlueprintNativeEvent)
	void OnPromptOptionAdded(const FGameplayTag& Conversation, FYapPromptHandle Handle, const UYapCharacter* Speaker, const FGameplayTag& MoodKey, const FText& DialogueText, const FText& TitleText);

	UFUNCTION(BlueprintNativeEvent)
	void OnPromptOptionsAllAdded(const FGameplayTag& Conversation);
};

// NOTES:
//
// This class does NOT implement IYapConversationListenerInterface because it's very annoying that Unreal refuses to let you convert interface events to functions.
// Instead, this class includes identical functions as IYapConversationListenerInterface.
// Templates are used to call the same functions on either a conversation broker or a IYapConversationListenerInterface implementer.
//
// The BNE's above pass many individual args instead of a struct to make it easier to refer to individual args inside of a blueprint graph.
