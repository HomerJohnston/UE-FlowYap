// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

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
	UFUNCTION(BlueprintNativeEvent, DisplayName = "On Conversation Begins")
	void K2_OnConversationBegins(const FGameplayTag& Conversation);
	virtual void K2_OnConversationBegins_Implementation(const FGameplayTag& Conversation);
	
	/** Code to run when a conversation ends. Do NOT call Parent when overriding. */
	UFUNCTION(BlueprintNativeEvent, DisplayName = "On Conversation Ends")
	void K2_OnConversationEnds(const FGameplayTag& Conversation);
	virtual void K2_OnConversationEnds_Implementation(const FGameplayTag& Conversation);

	/** Code to run when a piece of dialogue (speech) begins. Do NOT call Parent when overriding. */
	UFUNCTION(BlueprintNativeEvent, DisplayName = "On Dialogue Begins")
	void K2_OnDialogueBegins(const FGameplayTag& Conversation, FYapDialogueHandle DialogueHandle, const UYapCharacter* DirectedAt, const UYapCharacter* Speaker, const FGameplayTag& MoodKey, const FText& DialogueText, const FText& TitleText, float DialogueTime, const UObject* DialogueAudioAsset);
	virtual void K2_OnDialogueBegins_Implementation(const FGameplayTag& Conversation, FYapDialogueHandle DialogueHandle, const UYapCharacter* DirectedAt, const UYapCharacter* Speaker, const FGameplayTag& MoodKey, const FText& DialogueText, const FText& TitleText, float DialogueTime, const UObject* DialogueAudioAsset);

	/** Code to run when a piece of dialogue (speech) ends. Do NOT call Parent when overriding. */
	UFUNCTION(BlueprintNativeEvent, DisplayName = "On Dialogue Ends")
	void K2_OnDialogueEnds(const FGameplayTag& Conversation, FYapDialogueHandle DialogueHandle);
	virtual void K2_OnDialogueEnds_Implementation(const FGameplayTag& Conversation, FYapDialogueHandle DialogueHandle);

	/** Code to run when a single player prompt entry is emitted (for example, to add a button/text widget to a list). Do NOT call Parent when overriding. */
	UFUNCTION(BlueprintNativeEvent, DisplayName = "On Prompt Option Added")
	void K2_OnPromptOptionAdded(const FGameplayTag& Conversation, FYapPromptHandle Handle, const UYapCharacter* DirectedAt, const UYapCharacter* Speaker, const FGameplayTag& MoodKey, const FText& DialogueText, const FText& TitleText);
	virtual void K2_OnPromptOptionAdded_Implementation(const FGameplayTag& Conversation, FYapPromptHandle Handle, const UYapCharacter* DirectedAt, const UYapCharacter* Speaker, const FGameplayTag& MoodKey, const FText& DialogueText, const FText& TitleText);

	/** Code to run after all player prompt entries have been emitted. Do NOT call Parent when overriding. */
	UFUNCTION(BlueprintNativeEvent, DisplayName = "On Prompt Options All Added")
	void K2_OnPromptOptionsAllAdded(const FGameplayTag& Conversation);
	virtual void K2_OnPromptOptionsAllAdded_Implementation(const FGameplayTag& Conversation);

	/** Use this to read your game's settings and determine if mature language is permitted. Do NOT call Parent when overriding. */
	UFUNCTION(BlueprintNativeEvent, DisplayName = "Use Mature Dialogue")
	EYapMaturitySetting K2_UseMatureDialogue();
	virtual EYapMaturitySetting K2_UseMatureDialogue_Implementation();

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
	virtual void OnDialogueBegins(const FGameplayTag& Conversation, FYapDialogueHandle DialogueHandle, const UYapCharacter* DirectedAt, const UYapCharacter* Speaker, const FGameplayTag& MoodKey, const FText& DialogueText, const FText& TitleText, float DialogueTime, const UObject* DialogueAudioAsset)
	{
		K2_OnDialogueBegins(Conversation, DialogueHandle, DirectedAt, Speaker, MoodKey, DialogueText, TitleText, DialogueTime, DialogueAudioAsset);
	}
	
	/** Code to run when a piece of dialogue (speech) ends. Do NOT call Super when overriding. */
	virtual void OnDialogueEnds(const FGameplayTag& Conversation, FYapDialogueHandle DialogueHandle)
	{
		K2_OnDialogueEnds(Conversation, DialogueHandle);
	}
	
	/** Code to run when a single player prompt entry is emitted (for example, to add a button/text widget to a list). Do NOT call Super when overriding. */
	virtual void OnPromptOptionAdded(const FGameplayTag& Conversation, FYapPromptHandle Handle, const UYapCharacter* DirectedAt, const UYapCharacter* Speaker, const FGameplayTag& MoodKey, const FText& DialogueText, const FText& TitleText)
	{
		K2_OnPromptOptionAdded(Conversation, Handle, DirectedAt, Speaker, MoodKey, DialogueText, TitleText);
	}
	
	/** Code to run after all player prompt entries have been emitted. Do NOT call Super when overriding. */
	virtual void OnPromptOptionsAllAdded(const FGameplayTag& Conversation)
	{
		K2_OnPromptOptionsAllAdded(Conversation);
	}
	
	/** Use this to read your game's settings and determine if mature language is permitted. Do NOT call Super when overriding. */
	virtual EYapMaturitySetting UseMatureDialogue()
	{
		return K2_UseMatureDialogue();
	}
};
