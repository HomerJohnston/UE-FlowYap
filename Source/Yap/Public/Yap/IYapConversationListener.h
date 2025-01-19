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

/** A conversation listener is an interface you can apply to *any* class to help it respond to Yap dialogue.
 * This is essentially a virtual copy of part of the UYapBroker, but as an interface.
 *
 * You can optionally use UYapSubsystem::RegisterConversationListener(...) to register your class to automatically receive these events. 
 */
class IYapConversationListener
{
	GENERATED_BODY()

#if WITH_EDITOR
	bool bWarnedAboutMatureDialogue = false;
#endif
	
protected:
	/** Code to run when a conversation begins. Do NOT call Parent when overriding. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, DisplayName = "On Conversation Opened")
	void K2_OnConversationOpened(const FGameplayTag& Conversation);
	virtual void K2_OnConversationOpened_Implementation(const FGameplayTag& Conversation);
	
	/** Code to run when a conversation closes. Do NOT call Parent when overriding. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, DisplayName = "On Conversation Closed")
	void K2_OnConversationClosed(const FGameplayTag& Conversation);
	virtual void K2_OnConversationClosed_Implementation(const FGameplayTag& Conversation);

	/** Code to run when a piece of dialogue (speech) begins. Do NOT call Parent when overriding. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, DisplayName = "On Dialogue Begins")
	void K2_OnDialogueBegins(const FGameplayTag& Conversation, FYapDialogueHandle DialogueHandle, const UYapCharacter* DirectedAt, const UYapCharacter* Speaker, const FGameplayTag& MoodKey, const FText& DialogueText, const FText& TitleText, float DialogueTime, const UObject* DialogueAudioAsset);
	virtual void K2_OnDialogueBegins_Implementation(const FGameplayTag& Conversation, FYapDialogueHandle DialogueHandle, const UYapCharacter* DirectedAt, const UYapCharacter* Speaker, const FGameplayTag& MoodKey, const FText& DialogueText, const FText& TitleText, float DialogueTime, const UObject* DialogueAudioAsset);

	/** Code to run when a piece of dialogue (speech) ends. Do NOT call Parent when overriding. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, DisplayName = "On Dialogue Ends")
	void K2_OnDialogueEnds(const FGameplayTag& Conversation, FYapDialogueHandle DialogueHandle);
	virtual void K2_OnDialogueEnds_Implementation(const FGameplayTag& Conversation, FYapDialogueHandle DialogueHandle);

	/** Code to run when a single player prompt entry is emitted (for example, to add a button/text widget to a list). Do NOT call Parent when overriding. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, DisplayName = "On Prompt Option Added")
	void K2_AddPlayerPrompt(const FGameplayTag& Conversation, FYapPromptHandle Handle, const UYapCharacter* DirectedAt, const UYapCharacter* Speaker, const FGameplayTag& MoodKey, const FText& DialogueText, const FText& TitleText);
	virtual void K2_AddPlayerPrompt_Implementation(const FGameplayTag& Conversation, FYapPromptHandle Handle, const UYapCharacter* DirectedAt, const UYapCharacter* Speaker, const FGameplayTag& MoodKey, const FText& DialogueText, const FText& TitleText);

	/** Code to run after all player prompt entries have been emitted. Do NOT call Parent when overriding. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, DisplayName = "On Prompt Options All Added")
	void K2_AfterPlayerPromptAdded(const FGameplayTag& Conversation);
	virtual void K2_AfterPlayerPromptAdded_Implementation(const FGameplayTag& Conversation);

public:
	/** Code to run when a conversation begins. Do NOT call Super when overriding. */
	virtual void OnConversationBegins(const FGameplayTag& Conversation)
	{
		K2_OnConversationOpened(Conversation);
	};
	
	/** Code to run when a conversation ends. Do NOT call Super when overriding. */
	virtual void OnConversationCloses(const FGameplayTag& Conversation)
	{
		K2_OnConversationClosed(Conversation);
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
		K2_AddPlayerPrompt(Conversation, Handle, DirectedAt, Speaker, MoodKey, DialogueText, TitleText);
	}
	
	/** Code to run after all player prompt entries have been emitted. Do NOT call Super when overriding. */
	virtual void OnPromptOptionsAllAdded(const FGameplayTag& Conversation)
	{
		K2_AfterPlayerPromptAdded(Conversation);
	}

	// TODO should I have an "on player prompt selected" event?
};
