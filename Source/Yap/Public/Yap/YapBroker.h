// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once

#include "YapLog.h"

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

#if WITH_EDITOR
public:
	bool ImplementsGetWorld() const override { return true; }
#endif
	
	// ================================================================================================
	// STATE DATA
	// ================================================================================================
private:
	// These will be initialized during BeginPlay and keep track of whether there is a K2 function
	// implementation to call, if the C++ implementation is not overridden. If the C++ implementation
	// is overridden, it will supersede. This is "backwards" compared to normal BNE's but if you're
	// overriding this class in C++ you won't want to override further in BP.
	static TOptional<bool> bImplemented_Initialize;
	static TOptional<bool> bImplemented_UseMatureDialogue;
	static TOptional<bool> bImplemented_GetPlaybackSpeed;
	static TOptional<bool> bImplemented_CalculateWordCount;
	static TOptional<bool> bImplemented_GetAudioAssetDuration;
#if WITH_EDITOR
	static TOptional<bool> bImplemented_PreviewAudioAsset;
#endif
	
	// Some of these functions may be ran on tick by the editor or during play.
	// We want to log errors, but not spam the log on tick, only on the first occurrence.
	// I also want to make sure the errors log each time PIE runs, not just once.
	static bool bWarned_Initialize;
	static bool bWarned_UseMatureDialogue;
	static bool bWarned_GetPlaybackSpeed;
	static bool bWarned_CalculateWordCount;
	static bool bWarned_GetAudioAssetDuration;
#if WITH_EDITOR
	static bool bWarned_PreviewAudioAsset;
#endif
	
	// ================================================================================================
	// BLUEPRINT
	// ================================================================================================

protected:
	
	// - - - - - GENERAL UTILITY FUNCTIONS - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

	/** OPTIONAL FUNCTION - Do NOT call Parent when overriding.
	* Use this to do any desired initialization, such as creating a Dialogue UI instance if you aren't creating one already elsewhere. */
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "Initialize")
	EYapMaturitySetting K2_Initialize() const;
	
	/** OPTIONAL FUNCTION - Do NOT call Parent when overriding.
	 * Use this to read your game's user settings (e.g. "Enable Mature Content") and determine if mature language is permitted. */
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "Use Mature Dialogue")
	EYapMaturitySetting K2_UseMatureDialogue() const;

	/** OPTIONAL FUNCTION - Do NOT call Parent when overriding.
	 * Use this to modify speaking time by a scalar multiplier. Does NOT affect anything when fragments are using audio duration! */
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "Get Playback Speed")
	float K2_GetPlaybackSpeed() const;
	
	// - - - - - TEXT ASSET MANAGEMENT - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	/** OPTIONAL FUNCTION - Do NOT call Parent when overriding.
	 * Provides a word count estimate of a given piece of FText. A default implementation of this function exists. */
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "Calculate Word Count")
	int32 K2_CalculateWordCount(const FText& Text) const;

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

	// - - - - - GENERAL UTILITY FUNCTIONS - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

	/** OPTIONAL FUNCTION - Do NOT call Super when overriding.
	 * Use this to do any desired initialization, such as creating a Dialogue UI instance if you aren't creating one already elsewhere. */
	virtual void Initialize();
	
	/** OPTIONAL FUNCTION - Do NOT call Super when overriding.
	 * Use this to read your game's user settings (e.g. "Enable Mature Content") and determine if mature language is permitted. */
	virtual EYapMaturitySetting UseMatureDialogue() const;

	/** OPTIONAL FUNCTION - Do NOT call Super when overriding.
	 * Use this to modify speaking time by a scalar multiplier. Does NOT affect anything when fragments are using audio duration! */
	virtual float GetPlaybackSpeed() const;
	
	// - - - - - TEXT ASSET MANAGEMENT - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	/** OPTIONAL FUNCTION - Do NOT call Super when overriding.
	 * Provides a word count estimate of a given piece of FText. A default implementation of this function exists. */
	virtual int32 CalculateWordCount(const FText& Text) const;

	/** OPTIONAL FUNCTION - Do NOT call Super when overriding - rarely needed, overridable through C++ only.
	 * Use this to read your game's settings (e.g. text playback speed) and determine the duration a dialogue should run for.
	 * The default implementation of this function will use your project setting TextWordsPerMinute multiplied by GetPlaybackSpeed. */
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
		
	void Initialize_Internal();
	
#if WITH_EDITOR
	bool PreviewAudioAsset_Internal(const UObject* AudioAsset) const;
	
	bool ImplementsPreviewAudioAsset_Internal() const;
#endif

	// Thank you to Blue Man for this thing
	template<typename TFunction, typename... TArgs>
	struct TResolveFunctionReturn
	{
		using Type = std::invoke_result_t<TFunction, UYapBroker, TArgs...>;
	};

	// non-const variant
	template<auto TFunction, typename ...TArgs>
	auto CallK2Function(FString FunctionName, TOptional<bool>& bImplemented, bool& bWarned, bool bLogWarnings, TArgs&&... Args) -> typename TResolveFunctionReturn<decltype(TFunction), TArgs...>::Type
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

	// const variant, exactly the same
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

#undef LOCTEXT_NAMESPACE