#pragma once

struct FGameplayTag;
struct FYapPromptHandle;
struct FYapDialogueHandle;
struct FYapBit;

#include "Yap/YapPromptHandle.h"
#include "Yap/YapDialogueHandle.h"

#include "IYapConversationListener.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UYapConversationListener : public UInterface
{
	GENERATED_BODY()
};

class IYapConversationListener
{
	GENERATED_BODY()

#if WITH_EDITOR
	bool bWarnedAboutMatureDialogue = false;
#endif
	
protected:
	/** Code to run when a conversation begins. Do NOT call Parent when overriding. */
	UFUNCTION(BlueprintNativeEvent)
	void K2_OnConversationBegins(const FGameplayTag& Conversation);
	virtual void K2_OnConversationBegins_Implementation(const FGameplayTag& Conversation);
	
	/** Code to run when a conversation ends. Do NOT call Parent when overriding. */
	UFUNCTION(BlueprintNativeEvent)
	void K2_OnConversationEnds(const FGameplayTag& Conversation);
	virtual void K2_OnConversationEnds_Implementation(const FGameplayTag& Conversation);

	/** Code to run when a piece of dialogue (speech) begins. Do NOT call Parent when overriding. */
	UFUNCTION(BlueprintNativeEvent)
	void K2_OnDialogueBegins(const FGameplayTag& Conversation, FYapDialogueHandle DialogueHandle, const UYapCharacter* Speaker, const FGameplayTag& MoodKey, const FText& DialogueText, double DialogueTime, const UObject* DialogueAudioAsset, const UYapCharacter* DirectedAt);
	virtual void K2_OnDialogueBegins_Implementation(const FGameplayTag& Conversation, FYapDialogueHandle DialogueHandle, const UYapCharacter* Speaker, const FGameplayTag& MoodKey, const FText& DialogueText, double DialogueTime, const UObject* DialogueAudioAsset, const UYapCharacter* DirectedAt);

	/** Code to run when a piece of dialogue (speech) ends. Do NOT call Parent when overriding. */
	UFUNCTION(BlueprintNativeEvent)
	void K2_OnDialogueEnds(const FGameplayTag& Conversation, FYapDialogueHandle DialogueHandle);
	virtual void K2_OnDialogueEnds_Implementation(const FGameplayTag& Conversation, FYapDialogueHandle DialogueHandle);

	/** Code to run when a single player prompt entry is emitted (for example, to add a button/text widget to a list). Do NOT call Parent when overriding. */
	UFUNCTION(BlueprintNativeEvent)
	void K2_OnPromptOptionAdded(const FGameplayTag& Conversation, FYapPromptHandle Handle, const UYapCharacter* Speaker, const FGameplayTag& MoodKey, const FText& DialogueText, const FText& TitleText);
	virtual void K2_OnPromptOptionAdded_Implementation(const FGameplayTag& Conversation, FYapPromptHandle Handle, const UYapCharacter* Speaker, const FGameplayTag& MoodKey, const FText& DialogueText, const FText& TitleText);

	/** Code to run after all player prompt entries have been emitted. Do NOT call Parent when overriding. */
	UFUNCTION(BlueprintNativeEvent)
	void K2_OnPromptOptionsAllAdded(const FGameplayTag& Conversation);
	virtual void K2_OnPromptOptionsAllAdded_Implementation(const FGameplayTag& Conversation);

	/** Use this to read your game's settings and determine if mature language is permitted. Do NOT call Parent when overriding. */
	UFUNCTION(BlueprintNativeEvent)
	bool K2_UseMatureDialogue();
	virtual bool K2_UseMatureDialogue_Implementation();

public:
	/** Code to run when a conversation begins. Do NOT call Super when overriding. */
	virtual void OnConversationBegins(const FGameplayTag& Conversation)
	{
		K2_OnConversationBegins(Conversation);
	};
	
	/** Code to run when a conversation ends. Do NOT call Super when overriding. */
	virtual void OnConversationEnds(const FGameplayTag& Conversation)
	{
		K2_OnConversationEnds(Conversation);
	};
	
	/** Code to run when a piece of dialogue (speech) begins. Do NOT call Super when overriding. */
	virtual void OnDialogueBegins(const FGameplayTag& Conversation, FYapDialogueHandle DialogueHandle, const UYapCharacter* Speaker, const FGameplayTag& MoodKey, const FText& DialogueText, double DialogueTime, const UObject* DialogueAudioAsset, const UYapCharacter* DirectedAt)
	{
		K2_OnDialogueBegins(Conversation, DialogueHandle, Speaker, MoodKey, DialogueText, DialogueTime, DialogueAudioAsset, DirectedAt);
	}
	
	/** Code to run when a piece of dialogue (speech) ends. Do NOT call Super when overriding. */
	virtual void OnDialogueEnds(const FGameplayTag& Conversation, FYapDialogueHandle DialogueHandle)
	{
		K2_OnDialogueEnds(Conversation, DialogueHandle);
	}
	
	/** Code to run when a single player prompt entry is emitted (for example, to add a button/text widget to a list). Do NOT call Super when overriding. */
	virtual void OnPromptOptionAdded(const FGameplayTag& Conversation, FYapPromptHandle Handle, const UYapCharacter* Speaker, const FGameplayTag& MoodKey, const FText& DialogueText, const FText& TitleText)
	{
		K2_OnPromptOptionAdded(Conversation, Handle, Speaker, MoodKey, DialogueText, TitleText);
	}
	
	/** Code to run after all player prompt entries have been emitted. Do NOT call Super when overriding. */
	virtual void OnPromptOptionsAllAdded(const FGameplayTag& Conversation)
	{
		K2_OnPromptOptionsAllAdded(Conversation);
	}
	
	/** Use this to read your game's settings and determine if mature language is permitted. Do NOT call Super when overriding. */
	virtual bool UseMatureDialogue()
	{
		return K2_UseMatureDialogue();
	}
};
