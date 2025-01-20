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

// We will pass data into the conversation handlers via structs.
// This makes it easier for users to (optionally) build blueprint functions which accept the whole chunk of data in one pin.

// ------------------------------------------------------------------------------------------------

/** This could be passed directly. We're passing it within a struct so that if we ever decide to add more data to the event, existing blueprints remain working. */
USTRUCT(BlueprintType, DisplayName = "Yap: Conversation Opened")
struct FYapData_OnConversationOpened
{
	GENERATED_BODY()

	/** Conversation name. */
	UPROPERTY(BlueprintReadOnly)
	FGameplayTag Conversation;
};

// ------------------------------------------------------------------------------------------------

/** This could be passed directly. We're passing it within a struct so that if we ever decide to add more data to the event, existing blueprints remain working. */
USTRUCT(BlueprintType, DisplayName = "Yap: Conversation Closed")
struct FYapData_OnConversationClosed
{
	GENERATED_BODY()

	/** Conversation name. */
	UPROPERTY(BlueprintReadOnly)
	FGameplayTag Conversation;
};

// ------------------------------------------------------------------------------------------------

/** Struct containing all the data for this event. */
USTRUCT(BlueprintType, DisplayName = "Yap: Dialogue Begins")
struct FYapData_OnDialogueBegins
{
	GENERATED_BODY()

	/** Conversation name. */
	UPROPERTY(BlueprintReadOnly)
	FGameplayTag Conversation;
	
	/** Dialogue handle, can be used for interrupting or identifying dialogue. */
	UPROPERTY(BlueprintReadOnly)
	FYapDialogueHandle DialogueHandle;

	/** Who is being speaked towards. */
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<const UYapCharacter> DirectedAt;

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
	float DialogueTime;

	/** Audio asset, you are responsible to cast to your proper type to use. */
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<const UObject> DialogueAudioAsset;
};

// ------------------------------------------------------------------------------------------------

/** Struct containing all the data for this event. */
USTRUCT(BlueprintType, DisplayName = "Yap: Dialogue Ends")
struct FYapData_OnDialogueEnds
{
	GENERATED_BODY()

	/** Conversation name. */
	UPROPERTY(BlueprintReadOnly)
	FGameplayTag Conversation;

	/** Dialogue handle, can be used for interrupting or identifying dialogue. */
	UPROPERTY(BlueprintReadOnly)
	FYapDialogueHandle DialogueHandle;

	/** How long it is expected to wait before moving on to the next fragment or Flow Graph node. */
	UPROPERTY(BlueprintReadOnly)
	float PaddingTime;
};

// ------------------------------------------------------------------------------------------------

/** Struct containing all the data for this event. */
USTRUCT(BlueprintType, DisplayName = "Yap: Padding Time Over")
struct FYapData_OnPaddingTimeOver
{
	GENERATED_BODY()

	/** Conversation name. */
	UPROPERTY(BlueprintReadOnly)
	FGameplayTag Conversation;
	
	/** Dialogue handle, can be used for interrupting or identifying dialogue. */
	UPROPERTY(BlueprintReadOnly)FYapDialogueHandle DialogueHandle;
};

// ------------------------------------------------------------------------------------------------

/** Struct containing all the data for this event. */
USTRUCT(BlueprintType, DisplayName = "Yap: Add Player Prompt")
struct FYapData_AddPlayerPrompt
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
USTRUCT(BlueprintType, DisplayName = "Yap: After Player Prompts Added")
struct FYapData_AfterPlayerPromptsAdded
{
	GENERATED_BODY()

	/** Conversation name. */
	UPROPERTY(BlueprintReadOnly)
	FGameplayTag Conversation;
};

// ------------------------------------------------------------------------------------------------

/** Struct containing all the data for this event. */
USTRUCT(BlueprintType, DisplayName = "Yap: Player Prompt Selected")
struct FYapData_OnPlayerPromptSelected
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
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, DisplayName = "On Conversation Opened")
	void K2_OnConversationOpened(FYapData_OnConversationOpened Data);
	
	/** Code to run when a conversation closes. Do NOT call Parent when overriding. */
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, DisplayName = "On Conversation Closed")
	void K2_OnConversationClosed(FYapData_OnConversationClosed Data);

	/** Code to run when a piece of dialogue (speech) begins. Do NOT call Parent when overriding. */
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, DisplayName = "On Dialogue Begins")
	void K2_OnDialogueBegins(FYapData_OnDialogueBegins Data);

	/** Code to run when a piece of dialogue (speech) ends. Do NOT call Parent when overriding. */
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, DisplayName = "On Dialogue Ends")
	void K2_OnDialogueEnds(FYapData_OnDialogueEnds Data);

	/** Code to run after the padding time finishes (after dialogue has ended). Do NOT call Parent when overriding. */
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, DisplayName = "On Padding Time Over")
	void K2_OnPaddingTimeOver(FYapData_OnPaddingTimeOver Data);

	/** Code to run when a single player prompt entry is emitted (for example, to add a button/text widget to a list). Do NOT call Parent when overriding. */
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, DisplayName = "Add Player Prompt")
	void K2_AddPlayerPrompt(FYapData_AddPlayerPrompt Data);

	/** Code to run after all player prompt entries have been emitted. Do NOT call Parent when overriding. */
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, DisplayName = "After Player Prompts Added")
	void K2_AfterPlayerPromptsAdded(FYapData_AfterPlayerPromptsAdded Data);

	/** Code to run when a player prompt is ran. Do NOT call Parent when overriding. */
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, DisplayName = "On Player Prompt Selected")
	void K2_OnPlayerPromptSelected(FYapData_OnPlayerPromptSelected Data);
	
public:
	/** Code to run when a conversation begins. Do NOT call Super when overriding. */
	virtual void OnConversationOpened(FYapData_OnConversationOpened Data)
	{
		K2_OnConversationOpened(Data);
	};
	
	/** Code to run when a conversation ends. Do NOT call Super when overriding. */
	virtual void OnConversationClosed(FYapData_OnConversationClosed Data)
	{
		K2_OnConversationClosed(Data);
	};
	
	/** Code to run when a piece of dialogue (speech) begins. Do NOT call Super when overriding. */
	virtual void OnDialogueBegins(FYapData_OnDialogueBegins Data)
	{
		K2_OnDialogueBegins(Data);
	}
	
	/** Code to run when a piece of dialogue (speech) ends. Do NOT call Super when overriding. */
	virtual void OnDialogueEnds(FYapData_OnDialogueEnds Data)
	{
		K2_OnDialogueEnds(Data);
	}
	
	/** Code to run after the padding time finishes (after dialogue has ended). Do NOT call Super when overriding. */
	virtual void OnPaddingTimeOver(FYapData_OnPaddingTimeOver Data)
	{
		K2_OnPaddingTimeOver(Data);
	}
	
	/** Code to run when a single player prompt entry is emitted (for example, to add a button/text widget to a list). Do NOT call Super when overriding. */
	virtual void AddPlayerPrompt(FYapData_AddPlayerPrompt Data)
	{
		K2_AddPlayerPrompt(Data);
	}
	
	/** Code to run after all player prompt entries have been emitted. Do NOT call Super when overriding. */
	virtual void AfterPlayerPromptsAdded(FYapData_AfterPlayerPromptsAdded Data)
	{
		K2_AfterPlayerPromptsAdded(Data);
	}

	/** Code to run when a player prompt is ran. Do NOT call Super when overriding. */
	virtual void OnPlayerPromptSelected(FYapData_OnPlayerPromptSelected Data)
	{
		K2_OnPlayerPromptSelected(Data);
	}
};

