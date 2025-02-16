// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once

#include "GameplayTagContainer.h"

class UYapCharacter;
struct FYapPromptHandle;
struct FYapDialogueHandle;
struct FYapBit;

#include "Yap/YapPromptHandle.h"
#include "Yap/YapDialogueHandle.h"

#include "IYapConversationHandler.generated.h"

#define LOCTEXT_NAMESPACE "Yap"

// We will pass data into the conversation handlers via structs.
// This makes it easier for users to (optionally) build blueprint functions which accept the whole chunk of data in one pin.

// ------------------------------------------------------------------------------------------------

/** Struct containing all the data for this event. */
USTRUCT(BlueprintType, DisplayName = "Yap Conv. Chat Opened")
struct FYapData_ConversationChatOpened
{
	GENERATED_BODY()

	/** Conversation name. */
	UPROPERTY(BlueprintReadOnly)
	FGameplayTag Conversation;
};

// ------------------------------------------------------------------------------------------------

/** Struct containing all the data for this event. */
USTRUCT(BlueprintType, DisplayName = "Yap Conv. Chat Closed")
struct FYapData_ConversationChatClosed
{
	GENERATED_BODY()

	/** Conversation name. */
	UPROPERTY(BlueprintReadOnly)
	FGameplayTag Conversation;
};

// ------------------------------------------------------------------------------------------------

/** Struct containing all the data for this event. */
USTRUCT(BlueprintType, DisplayName = "Yap Conv. Dialogue Node Entered")
struct FYapData_ConversationDialogueNodeEntered
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<const UFlowNode_YapDialogue> DialogueNode;

	UPROPERTY(BlueprintReadOnly)
	FGameplayTag DialogueTag;
};

// ------------------------------------------------------------------------------------------------

/** Struct containing all the data for this event. */
USTRUCT(BlueprintType, DisplayName = "Yap Conv. Dialogue Node Exited")
struct FYapData_ConversationDialogueNodeExited
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<const UFlowNode_YapDialogue> DialogueNode = nullptr;
	
	UPROPERTY(BlueprintReadOnly)
	FGameplayTag DialogueTag;
};

// ------------------------------------------------------------------------------------------------

/** Struct containing all the data for this event. */
USTRUCT(BlueprintType, DisplayName = "Yap Conv. Dialogue Node Bypassed")
struct FYapData_ConversationDialogueNodeBypassed
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<const UFlowNode_YapDialogue> DialogueNode = nullptr;
	
	UPROPERTY(BlueprintReadOnly)
	FGameplayTag DialogueTag;
};

// ------------------------------------------------------------------------------------------------

/** Struct containing all the data for this event. */
USTRUCT(BlueprintType, DisplayName = "Yap Conv. Speech Begins")
struct FYapData_ConversationSpeechBegins
{
	GENERATED_BODY()

	/** Conversation name. */
	UPROPERTY(BlueprintReadOnly)
	FGameplayTag Conversation;
	
	/** Dialogue handle, can be used for interrupting or identifying dialogue. */
	UPROPERTY(BlueprintReadOnly)
	FYapDialogueHandleRef DialogueHandleRef;

	/** Who is being speaked towards. */
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<const UYapCharacter> DirectedAt = nullptr;

	/** Who is speaking. */
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<const UYapCharacter> Speaker;

	/** Mood of the speaker. */
	UPROPERTY(BlueprintReadOnly)
	FGameplayTag MoodTag;

	/** Text being spoken. */
	UPROPERTY(BlueprintReadOnly)
	FText DialogueText;

	/** Optional title text representing the dialogue. */
	UPROPERTY(BlueprintReadOnly)
	FText TitleText;
	
	/** How long this dialogue is expected to play for. */
	UPROPERTY(BlueprintReadOnly)
	float DialogueTime = 0;

	/** Delay after this dialogue completes before carrying on. */
	UPROPERTY(BlueprintReadOnly)
	float PaddingTime = 0;

	/** Audio asset, you are responsible to cast to your proper type to use. */
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<const UObject> DialogueAudioAsset;

	/** Can this dialogue be skipped? */
	UPROPERTY(BlueprintReadOnly)
	bool bSkippable = false;
};

// ------------------------------------------------------------------------------------------------

/** Struct containing all the data for this event. */
USTRUCT(BlueprintType, DisplayName = "Yap Conv. Speech Ends")
struct FYapData_ConversationSpeechEnds
{
	GENERATED_BODY()

	/** Conversation name. */
	UPROPERTY(BlueprintReadOnly)
	FGameplayTag Conversation;

	/** Dialogue handle, can be used for interrupting or identifying dialogue. */
	UPROPERTY(BlueprintReadOnly)
	FYapDialogueHandleRef DialogueHandleRef;

	/** How long it is expected to wait before moving on to the next fragment or Flow Graph node. */
	UPROPERTY(BlueprintReadOnly)
	float PaddingTime = 0;
};

// ------------------------------------------------------------------------------------------------

/** Struct containing all the data for this event. */
USTRUCT(BlueprintType, DisplayName = "Yap Conv. Speech Padding Ends")
struct FYapData_ConversationSpeechPaddingEnds
{
	GENERATED_BODY()

	/** Conversation name. */
	UPROPERTY(BlueprintReadOnly)
	FGameplayTag Conversation;
	
	/** Dialogue handle, can be used for interrupting or identifying dialogue. */
	UPROPERTY(BlueprintReadOnly)
	FYapDialogueHandleRef DialogueHandleRef;

	/** Will manual advancement be required to progress? */
	UPROPERTY(BlueprintReadOnly)
	bool bManualAdvance = false;
};

// ------------------------------------------------------------------------------------------------

/** Struct containing all the data for this event. */
USTRUCT(BlueprintType, DisplayName = "Yap Conv. Player Prompt Created")
struct FYapData_ConversationPlayerPromptCreated
{
	GENERATED_BODY()

	/** Conversation name. */
	UPROPERTY(BlueprintReadOnly)
	FGameplayTag Conversation;

	/** Dialogue handle, can be used for interrupting or identifying dialogue. */
	UPROPERTY(BlueprintReadOnly)
	FYapPromptHandle Handle;

	/** Who will be spoken to. */
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<const UYapCharacter> DirectedAt;

	/** Who is going to speak. */
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<const UYapCharacter> Speaker;

	/** Mood of the speaker. */
	UPROPERTY(BlueprintReadOnly)
	FGameplayTag MoodTag;
	 
	/** Text that will be spoken. */
	UPROPERTY(BlueprintReadOnly)
	FText DialogueText;

	/** Optional title text representing the dialogue. */
	UPROPERTY(BlueprintReadOnly)
	FText TitleText;
};

// ------------------------------------------------------------------------------------------------

/** Struct containing all the data for this event. */
USTRUCT(BlueprintType, DisplayName = "Yap Conv. Player Prompts Created")
struct FYapData_ConversationPlayerPromptsReady
{
	GENERATED_BODY()

	/** Conversation name. */
	UPROPERTY(BlueprintReadOnly)
	FGameplayTag Conversation;
};

// ------------------------------------------------------------------------------------------------

/** Struct containing all the data for this event. */
USTRUCT(BlueprintType, DisplayName = "Yap Conv. Player Prompt Selected")
struct FYapData_ConversationPlayerPromptChosen
{
	GENERATED_BODY()

	/** Conversation name. */
	UPROPERTY(BlueprintReadOnly)
	FGameplayTag Conversation;
};
// ================================================================================================

UINTERFACE(MinimalAPI, Blueprintable)
class UYapConversationHandler : public UInterface
{
	GENERATED_BODY()
};

/** A conversation handler is an interface you can apply to anything to help it respond to Yap dialogue.
 * Use UYapSubsystem::RegisterConversationHandler(...) to register your class for events. 
 */
class IYapConversationHandler
{
	GENERATED_BODY()

#if WITH_EDITOR
	bool bWarnedAboutMatureDialogue = false;
#endif
	
protected:
	/** Code to run when a conversation begins. Do NOT call Parent when overriding. */
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "Conv. Chat Opened")
	void K2_ConversationChatOpened(FYapData_ConversationChatOpened Data);
	
	/** Code to run when a conversation closes. Do NOT call Parent when overriding. */
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "Conv. Chat Closed")
	void K2_ConversationChatClosed(FYapData_ConversationChatClosed Data);

	/** Code to run when a dialogue node is first entered. Do NOT call Parent when overriding. */
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "Conv. Dialogue Node Entered")
	void K2_ConversationDialogueNodeEntered(FYapData_ConversationDialogueNodeEntered Data);
	
	/** Code to run when a dialogue node is finally exited. Do NOT call Parent when overriding. */
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "Conv. Dialogue Node Exited")
	void K2_ConversationDialogueNodeExited(FYapData_ConversationDialogueNodeExited Data);

	/** Code to run when a dialogue node runs its bypass output. Do NOT call Parent when overriding. */
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "Conv. Dialogue Node Bypassed")
	void K2_ConversationDialogueNodeBypassed(FYapData_ConversationDialogueNodeBypassed Data);
	
	/** Code to run when a piece of dialogue (speech) begins. Do NOT call Parent when overriding. */
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "Conv. Speech Begins")
	void K2_ConversationSpeechBegins(FYapData_ConversationSpeechBegins Data);

	/** Code to run when a piece of dialogue (speech) ends. Do NOT call Parent when overriding. */
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "Conv. Speech Ends")
	void K2_ConversationSpeechEnds(FYapData_ConversationSpeechEnds Data);

	/** Code to run after the padding time finishes (after dialogue has ended). Do NOT call Parent when overriding. */
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "Conv. Speech Padding Ends")
	void K2_ConversationSpeechPaddingEnds(FYapData_ConversationSpeechPaddingEnds Data);

	/** Code to run when a single player prompt entry is emitted (for example, to add a button/text widget to a list). Do NOT call Parent when overriding. */
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "Conv. Player Prompt Created")
	void K2_ConversationPlayerPromptCreated(FYapData_ConversationPlayerPromptCreated Data);

	/** Code to run after all player prompt entries have been emitted. Do NOT call Parent when overriding. */
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "Conv. Player Prompts Ready")
	void K2_ConversationPlayerPromptsReady(FYapData_ConversationPlayerPromptsReady Data);

	/** Code to run when a player prompt is ran. Do NOT call Parent when overriding. */
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "Conv. Player Prompt Chosen")
	void K2_ConversationPlayerPromptChosen(FYapData_ConversationPlayerPromptChosen Data);
	
public:
	/** Code to run when a conversation begins. Do NOT call Super when overriding. */
	virtual void OnConversationChatOpened(FYapData_ConversationChatOpened Data)
	{
		K2_ConversationChatOpened(Data);
	};
	
	/** Code to run when a conversation ends. Do NOT call Super when overriding. */
	virtual void OnConversationChatClosed(FYapData_ConversationChatClosed Data)
	{
		K2_ConversationChatClosed(Data);
	};

	/** Code to run when first entering a dialogue node, before any speaking begins. Do NOT call Super when overriding. */
	virtual void OnConversationDialogueNodeEntered(FYapData_ConversationDialogueNodeEntered Data)
	{
		K2_ConversationDialogueNodeEntered(Data);
	}

	/** Code to run when finishing a dialogue node. Do NOT call Super when overriding. */
	virtual void OnConversationDialogueNodeExited(FYapData_ConversationDialogueNodeExited Data)
	{
		K2_ConversationDialogueNodeExited(Data);
	}

	/** Code to run when exiting a dialogue node through the bypass pin. Do NOT call Super when overriding. */
	virtual void OnConversationDialogueNodeBypassed(FYapData_ConversationDialogueNodeBypassed Data)
	{
		K2_ConversationDialogueNodeBypassed(Data);
	}	
	
	/** Code to run when a piece of dialogue (speech) begins. Do NOT call Super when overriding. */
	virtual void OnConversationSpeechBegins(FYapData_ConversationSpeechBegins Data)
	{
		K2_ConversationSpeechBegins(Data);
	}
	
	/** Code to run when a piece of dialogue (speech) ends. Do NOT call Super when overriding. */
	virtual void OnConversationSpeechEnds(FYapData_ConversationSpeechEnds Data)
	{
		K2_ConversationSpeechEnds(Data);
	}
	
	/** Code to run after the padding time finishes (after dialogue has ended). Do NOT call Super when overriding. */
	virtual void OnConversationSpeechPaddingEnds(FYapData_ConversationSpeechPaddingEnds Data)
	{
		K2_ConversationSpeechPaddingEnds(Data);
	}
	
	/** Code to run when a single player prompt entry is emitted (for example, to add a button/text widget to a list). Do NOT call Super when overriding. */
	virtual void OnConversationPlayerPromptCreated(FYapData_ConversationPlayerPromptCreated Data)
	{
		K2_ConversationPlayerPromptCreated(Data);
	}
	
	/** Code to run after all player prompt entries have been emitted. Do NOT call Super when overriding. */
	virtual void OnConversationPlayerPromptsReady(FYapData_ConversationPlayerPromptsReady Data)
	{
		K2_ConversationPlayerPromptsReady(Data);
	}

	/** Code to run when a player prompt is ran. Do NOT call Super when overriding. */
	virtual void OnConversationPlayerPromptChosen(FYapData_ConversationPlayerPromptChosen Data)
	{
		K2_ConversationPlayerPromptChosen(Data);
	}
};

#undef LOCTEXT_NAMESPACE