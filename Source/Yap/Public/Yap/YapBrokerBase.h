// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once

#include "Yap/IYapConversationListener.h"

#include "YapBrokerBase.generated.h"

enum class EYapMaturitySetting : uint8;
class UYapCharacter;
struct FYapDialogueHandle;
struct FYapPromptHandle;

#define LOCTEXT_NAMESPACE "Yap"

/** Required class for brokering Yap to your game. Create a child class of this and implement the functions as needed. Then set Yap's project settings to use your class. */
UCLASS(Abstract, MinimalAPI)
class UYapBrokerBase : public UObject
{
	GENERATED_BODY()

#if WITH_EDITOR
protected:
	bool bWarnedAboutMatureDialogue = false;
#endif
	
public:
	bool ImplementsGetWorld() const override { return true; }
	
protected:
	/** Code to run when a conversation begins. Do NOT call Parent when overriding. */
	UFUNCTION(BlueprintNativeEvent, DisplayName = "On Conversation Begins")
	void K2_OnConversationBegins(const FGameplayTag& Conversation);

	/** Code to run when a conversation ends. Do NOT call Parent when overriding. */
	UFUNCTION(BlueprintNativeEvent, DisplayName = "On Conversation Ends")
	void K2_OnConversationEnds(const FGameplayTag& Conversation);

	// TODO reorder this - DirectedAt should go after Speaker.
	// TODO this should contain TitleText even though most people will not use it.
	/** Code to run when a piece of dialogue (speech) begins. Do NOT call Parent when overriding. */
	UFUNCTION(BlueprintNativeEvent, DisplayName = "On Dialogue Begins")
	void K2_OnDialogueBegins(const FGameplayTag& Conversation, FYapDialogueHandle DialogueHandle, const UYapCharacter* DirectedAt, const UYapCharacter* Speaker, const FGameplayTag& MoodKey, const FText& DialogueText, const FText& TitleText, float DialogueTime, const UObject* DialogueAudioAsset);

	/** Code to run when a piece of dialogue (speech) ends. Do NOT call Parent when overriding. */
	UFUNCTION(BlueprintNativeEvent, DisplayName = "On Dialogue Ends")
	void K2_OnDialogueEnds(const FGameplayTag& Conversation, FYapDialogueHandle DialogueHandle);

	/** Code to run when a single player prompt entry is emitted (for example, to add a button/text widget to a list). Do NOT call Parent when overriding. */
	UFUNCTION(BlueprintNativeEvent, DisplayName = "On Prompt Option Added")
	void K2_OnPromptOptionAdded(const FGameplayTag& Conversation, FYapPromptHandle Handle, const UYapCharacter* DirectedAt, const UYapCharacter* Speaker, const FGameplayTag& MoodKey, const FText& DialogueText, const FText& TitleText);

	/** Code to run after all player prompt entries have been emitted. Do NOT call Parent when overriding. */
	UFUNCTION(BlueprintNativeEvent, DisplayName = "On Prompt Options All Added")
	void K2_OnPromptOptionsAllAdded(const FGameplayTag& Conversation);

	/** Use this to read your game's settings and determine if mature language is permitted. Do NOT call Parent when overriding. */
	UFUNCTION(BlueprintNativeEvent, DisplayName = "Use Mature Dialogue")
	EYapMaturitySetting K2_UseMatureDialogue();

#if WITH_EDITOR
	/** Implement this to play audio assets from the graph editor. Cast to your project's audio type and play it. Do NOT call Parent when overriding. */
	UFUNCTION(BlueprintNativeEvent, DisplayName = "Play Dialogue Audio In Editor")
	bool K2_PreviewDialogueAudio(const UObject* AudioAsset) const;

	UFUNCTION(BlueprintNativeEvent, DisplayName = "Get Dialogue Audio Duration")
	float K2_GetDialogueAudioDuration(const UObject* AudioAsset) const;
#endif

// ================================================================================================

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
	virtual void OnDialogueBegins(const FGameplayTag& Conversation, FYapDialogueHandle DialogueHandle, const UYapCharacter* DirectedAt, const UYapCharacter* Speaker, const FGameplayTag& MoodKey, const FText& DialogueText, const FText& TitleText, float DialogueTime, const UObject* AudioAsset)
	{
		K2_OnDialogueBegins(Conversation, DialogueHandle, DirectedAt, Speaker, MoodKey, DialogueText, TitleText, DialogueTime, AudioAsset);
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

#if WITH_EDITOR
	YAP_API bool PreviewDialogueAudio_Internal(const UObject* AudioAsset) const;
	
	/** Implement this to play audio assets from the graph editor. Cast to your project's audio type and play it. Do NOT call Super when overriding. */
	virtual bool PreviewDialogueAudio(const UObject* AudioAsset) const
	{
		return K2_PreviewDialogueAudio(AudioAsset);
	}

	YAP_API bool ImplementsPreviewDialogueAudio() const;
	
	YAP_API float GetDialogueAudioDuration(const UObject* AudioAsset) const
	{
		return K2_GetDialogueAudioDuration(AudioAsset);
	}
#endif
};

// NOTES:
//
// This class does NOT implement IYapConversationListenerInterface because it's very annoying that Unreal refuses to let you convert interface events to functions,
// but you can convert BIE/BNE's to functions. This class includes identical functions as IYapConversationListenerInterface. Templates are used in the Yap Subsystem
// to call the same functions on either a conversation broker or a IYapConversationListenerInterface implementer.
//
// The BNE's above pass many individual args instead of a struct to make it easier to refer to individual args inside of a blueprint graph.

#undef LOCTEXT_NAMESPACE