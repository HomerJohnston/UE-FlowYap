// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once

#include "Yap/IYapConversationListener.h"

#include "YapConversationBrokerBase.generated.h"

enum class EYapMaturitySetting : uint8;
class UYapCharacter;
struct FYapDialogueHandle;
struct FYapPromptHandle;

#define LOCTEXT_NAMESPACE "Yap"

/** Optional base class for brokering Yap to your game. Create a child class of this and the functions to create conversation panels and/or display floating text widgets in your game. Then set Yap's project settings to use your class. */
UCLASS(Abstract)
class UYapConversationBrokerBase : public UObject
{
	GENERATED_BODY()

#if WITH_EDITOR
public:
	bool ImplementsGetWorld() const override { return true; }

protected:
	bool bWarnedAboutMatureDialogue = false;
#endif
	
protected:
	/** Code to run when a conversation begins. Do NOT call Parent when overriding. */
	UFUNCTION(BlueprintNativeEvent)
	void K2_OnConversationBegins(const FGameplayTag& Conversation);

	/** Code to run when a conversation ends. Do NOT call Parent when overriding. */
	UFUNCTION(BlueprintNativeEvent)
	void K2_OnConversationEnds(const FGameplayTag& Conversation);

	/** Code to run when a piece of dialogue (speech) begins. Do NOT call Parent when overriding. */
	UFUNCTION(BlueprintNativeEvent)
	void K2_OnDialogueBegins(const FGameplayTag& Conversation, FYapDialogueHandle DialogueHandle, const UYapCharacter* Speaker, const FGameplayTag& MoodKey, const FText& DialogueText, float DialogueTime, const UObject* DialogueAudioAsset, const UYapCharacter* DirectedAt);

	/** Code to run when a piece of dialogue (speech) ends. Do NOT call Parent when overriding. */
	UFUNCTION(BlueprintNativeEvent)
	void K2_OnDialogueEnds(const FGameplayTag& Conversation, FYapDialogueHandle DialogueHandle);

	/** Code to run when a single player prompt entry is emitted (for example, to add a button/text widget to a list). Do NOT call Parent when overriding. */
	UFUNCTION(BlueprintNativeEvent)
	void K2_OnPromptOptionAdded(const FGameplayTag& Conversation, FYapPromptHandle Handle, const UYapCharacter* Speaker, const FGameplayTag& MoodKey, const FText& DialogueText, const FText& TitleText);

	/** Code to run after all player prompt entries have been emitted. Do NOT call Parent when overriding. */
	UFUNCTION(BlueprintNativeEvent)
	void K2_OnPromptOptionsAllAdded(const FGameplayTag& Conversation);

	/** Use this to read your game's settings and determine if mature language is permitted. Do NOT call Parent when overriding. */
	UFUNCTION(BlueprintNativeEvent)
	EYapMaturitySetting K2_UseMatureDialogue();
	
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
	virtual void OnDialogueBegins(const FGameplayTag& Conversation, FYapDialogueHandle DialogueHandle, const UYapCharacter* Speaker, const FGameplayTag& MoodKey, const FText& DialogueText, float DialogueTime, const UObject* DialogueAudioAsset, const UYapCharacter* DirectedAt)
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
	virtual EYapMaturitySetting UseMatureDialogue()
	{
		return K2_UseMatureDialogue();
	}

	
};

// NOTES:
//
// This class does NOT implement IYapConversationListenerInterface because it's very annoying that Unreal refuses to let you convert interface events to functions,
// but you can convert BIE/BNE's to functions. This class includes identical functions as IYapConversationListenerInterface. Templates are used in the Yap Subsystem
// to call the same functions on either a conversation broker or a IYapConversationListenerInterface implementer.
//
// The BNE's above pass many individual args instead of a struct to make it easier to refer to individual args inside of a blueprint graph.

#undef LOCTEXT_NAMESPACE