// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once

#include "YapLog.h"
#include "Yap/IYapConversationListener.h"

#include "YapBroker.generated.h"

enum class EYapMaturitySetting : uint8;
class UYapCharacter;
struct FYapDialogueHandle;
struct FYapPromptHandle;

#define LOCTEXT_NAMESPACE "Yap"

/** Required class for brokering Yap to your game. Create a child class of this and implement the functions as needed. Then set Yap's project settings to use your class.
 *
 * Do ***NOT*** call Super or Parent function implementations when overriding any functions in this class. */
UCLASS(Abstract, Blueprintable)
class YAP_API UYapBroker : public UObject
{
	GENERATED_BODY()

	// Some of these functions may be ran on tick by the editor or during play. I want to log errors, but not spam the log. I also want to keep logging it every time PIE runs, so static isn't a convenient option.	
public:
	bool ImplementsGetWorld() const override { return true; }
	
	// ================================================================================================
	// BLUEPRINT - Do NOT override these using C++. Override the raw non-K2 functions below.
	// ================================================================================================

protected:
	
	// - - - - - DIALOGUE PLAYBACK - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	
	/** REQUIRED FUNCTION - Do NOT call Parent when overriding.
	 * Executes when a conversation begins. */
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "Conversation Opened")
	void K2_OnConversationOpened(const FGameplayTag& Conversation) const;

	/** REQUIRED FUNCTION - Do NOT call Parent when overriding.
	 * Executes when a conversation ends. */
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "Conversation Closed")
	void K2_OnConversationClosed(const FGameplayTag& Conversation) const;

	/** REQUIRED FUNCTION - Do NOT call Parent when overriding.
	 * Executes when a piece of dialogue (speech) begins. */
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "Dialogue Begins")
	void K2_OnDialogueBegins(const FGameplayTag& Conversation, FYapDialogueHandle DialogueHandle, const UYapCharacter* DirectedAt, const UYapCharacter* Speaker, const FGameplayTag& MoodKey, const FText& DialogueText, const FText& TitleText, float DialogueTime, const UObject* DialogueAudioAsset) const;

	/** REQUIRED FUNCTION - Do NOT call Parent when overriding.
	 * Executes when a piece of dialogue (speech) ends. */
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "Dialogue Ends")
	void K2_OnDialogueEnds(const FGameplayTag& Conversation, FYapDialogueHandle DialogueHandle) const;

	/** REQUIRED FUNCTION - Do NOT call Parent when overriding.
	 * Executes when a single player prompt entry is emitted (for example, so you can add a button/text widget to a list). */
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "Add Player Prompt")
	void K2_AddPlayerPrompt(const FGameplayTag& Conversation, FYapPromptHandle Handle, const UYapCharacter* DirectedAt, const UYapCharacter* Speaker, const FGameplayTag& MoodKey, const FText& DialogueText, const FText& TitleText) const;

	/** REQUIRED FUNCTION - Do NOT call Parent when overriding.
	 * Executes after all player prompt entries have been emitted. */
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "After Player Prompts Added")
	void K2_AfterPlayerPromptsAdded(const FGameplayTag& Conversation) const;

	// - - - - - GENERAL UTILITY FUNCTIONS - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

	/** OPTIONAL FUNCTION - Do NOT call Parent when overriding.
	 * Use this to read your game's user settings (e.g. "Enable Mature Content") and determine if mature language is permitted. */
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "Use Mature Dialogue")
	EYapMaturitySetting K2_UseMatureDialogue() const;

	// - - - - - TEXT ASSET MANAGEMENT - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	/** OPTIONAL FUNCTION - Do NOT call Parent when overriding.
	 * Provides a word count estimate of a given piece of FText. A default implementation of this function exists. */
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "Calculate Word Count")
	int32 K2_CalculateWordCount(const FText& Text) const;

	/** OPTIONAL FUNCTION - Do NOT call Parent when overriding.
	 * Use this to read your game's settings (e.g. text playback speed) and determine the duration a dialogue should run for. The default implementation of this function will use the project setting TextWordsPerMinute. */
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "Get Dialogue Audio Duration")
	float K2_CalculateTextTime(int32 WordCount, int32 CharCount) const;
	
	// - - - - - AUDIO ASSET MANAGEMENT - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	/** Overriding this is required if you use 3rd party audio (Wwise, FMOD, etc.) - Do NOT call Parent when overriding.
	 * Use this to cast to your project's audio type(s) and return their duration length in seconds. */
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "Get Dialogue Audio Duration")
	float K2_GetAudioAssetDuration(const UObject* AudioAsset) const;

#if WITH_EDITOR
	/** Overriding this is required if you use 3rd party audio (Wwise, FMOD, etc.) - Do NOT call Parent when overriding.
	 * Use this to cast to your project's audio type(s) and initiate playback in editor. */
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "Play Dialogue Audio In Editor")
	bool K2_PreviewAudioAsset(const UObject* AudioAsset) const;
#endif
	
	// ============================================================================================
	// C++ OVERRIDES
	// ============================================================================================
	
public:
	
	// - - - - - DIALOGUE PLAYBACK - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	
	/** REQUIRED FUNCTION - Do NOT call Super when overriding.
	 * Executes when a conversation begins. */
	virtual void OnConversationOpened(const FGameplayTag& Conversation) const;

	/** REQUIRED FUNCTION - Do NOT call Super when overriding.
	 * Executes when a conversation ends. */
	virtual void OnConversationClosed(const FGameplayTag& Conversation) const;

	/** REQUIRED FUNCTION - Do NOT call Super when overriding.
	 * Executes when a piece of dialogue (speech) begins. */
	virtual void OnDialogueBegins(const FGameplayTag& Conversation, FYapDialogueHandle DialogueHandle, const UYapCharacter* DirectedAt, const UYapCharacter* Speaker, const FGameplayTag& MoodKey, const FText& DialogueText, const FText& TitleText, float DialogueTime, const UObject* AudioAsset) const;

	/** REQUIRED FUNCTION - Do NOT call Super when overriding.
	 * Executes when a piece of dialogue (speech) ends. */
	virtual void OnDialogueEnds(const FGameplayTag& Conversation, FYapDialogueHandle DialogueHandle) const;

	/** REQUIRED FUNCTION - Do NOT call Super when overriding.
	 * Executes when a single player prompt entry is emitted (for example, to add a button/text widget to a list). */
	virtual void AddPlayerPrompt(const FGameplayTag& Conversation, FYapPromptHandle Handle, const UYapCharacter* DirectedAt, const UYapCharacter* Speaker, const FGameplayTag& MoodKey, const FText& DialogueText, const FText& TitleText) const;

	/** REQUIRED FUNCTION - Do NOT call Super when overriding.
	 * Executes after all player prompt entries have been emitted. */
	virtual void AfterPlayerPromptsAdded(const FGameplayTag& Conversation) const;

	// - - - - - GENERAL UTILITY FUNCTIONS - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
	
	/** OPTIONAL FUNCTION - Do NOT call Super when overriding.
	 * Use this to read your game's user settings (e.g. "Enable Mature Content") and determine if mature language is permitted. */
	virtual EYapMaturitySetting UseMatureDialogue() const;

	// - - - - - TEXT ASSET MANAGEMENT - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	/** OPTIONAL FUNCTION - Do NOT call Super when overriding.
	 * Provides a word count estimate of a given piece of FText. A default implementation of this function exists. */
	virtual int32 CalculateWordCount(const FText& Text) const;

	/** OPTIONAL FUNCTION - Do NOT call Super when overriding.
	 * Use this to read your game's settings (e.g. text playback speed) and determine the duration a dialogue should run for. The default implementation of this function will use the project setting TextWordsPerMinute. */
	virtual float CalculateTextTime(int32 WordCount, int32 CharCount) const;

	// - - - - - AUDIO ASSET MANAGEMENT - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	
	/** Overriding this is required for 3rd party audio (Wwise, FMOD, etc.) - Do NOT call Super when overriding.
	 * Use this to cast to your project's audio type(s) and return their duration length in seconds. */
	virtual float GetAudioAssetDuration(const UObject* AudioAsset) const;

#if WITH_EDITOR
	/** Overriding this is required for 3rd party audio (Wwise, FMOD, etc.) - Do NOT call Super when overriding.
	 * Use this to cast to your project's audio type(s) and initiate playback in editor. */
	virtual bool PreviewAudioAsset(const UObject* AudioAsset) const;
#endif

	// ============================================================================================
	// INTERNAL FUNCTIONS (USED BY YAP)
	// ============================================================================================

	static TOptional<bool> bImplemented_OnConversationOpened;
	static TOptional<bool> bImplemented_OnConversationClosed;
	static TOptional<bool> bImplemented_OnDialogueBegins;
	static TOptional<bool> bImplemented_OnDialogueEnds;
	static TOptional<bool> bImplemented_AddPlayerPrompt;
	static TOptional<bool> bImplemented_AfterPlayerPromptsAdded;
	static TOptional<bool> bImplemented_UseMatureDialogue;
	static TOptional<bool> bImplemented_CalculateWordCount;
	static TOptional<bool> bImplemented_CalculateTextTime;
	static TOptional<bool> bImplemented_GetAudioAssetDuration;
#if WITH_EDITOR
	static TOptional<bool> bImplemented_PreviewAudioAsset;
#endif
	
#if WITH_EDITOR
	static bool bWarned_OnConversationOpened;
	static bool bWarned_OnConversationClosed;
	static bool bWarned_OnDialogueBegins;
	static bool bWarned_OnDialogueEnds;
	static bool bWarned_AddPlayerPrompt;
	static bool bWarned_AfterPlayerPromptsAdded;
	static bool bWarned_UseMatureDialogue;
	static bool bWarned_CalculateWordCount;
	static bool bWarned_CalculateTextTime;
	static bool bWarned_GetAudioAssetDuration;
	static bool bWarned_PreviewAudioAsset;
#endif
		
	void Initialize();
	
#if WITH_EDITOR
	bool PreviewAudioAsset_Internal(const UObject* AudioAsset) const;
	
	bool ImplementsPreviewAudioAsset_Internal() const;
#endif

	// Thank you to Blue Man for this... thing
	template<typename TFunction, typename... TArgs>
	struct TResolveFunctionReturn
	{
		using Type = std::invoke_result_t<TFunction, UYapBroker, TArgs...>;
	};
	
	template<auto TFunction, typename ...TArgs>
	auto CallK2Function(FString FunctionName, TOptional<bool>& bImplemented, bool& bWarned, bool bLogWarnings, TArgs&&... Args) const -> typename TResolveFunctionReturn<decltype(TFunction), TArgs...>::Type
	{
		using TReturn = typename TResolveFunctionReturn<decltype(TFunction), TArgs...>::Type;
		
		check(bImplemented.IsSet());

		if (bImplemented.GetValue())
		{
			return (this->*TFunction)(std::forward<TArgs>(Args)...);
		}

		if (!bWarned)
		{
			if (bLogWarnings)
			{
				UE_LOG(LogYap, Error, TEXT("Unimplemented broker function: %s"), *FunctionName);
			}

			bWarned = true;
		}

		return TReturn{};
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