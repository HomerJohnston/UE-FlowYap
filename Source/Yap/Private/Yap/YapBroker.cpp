// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "Yap/YapBroker.h"

#include "Internationalization/BreakIterator.h"
#include "Yap/YapDialogueHandle.h"
#include "Yap/YapLog.h"
#include "Yap/YapProjectSettings.h"
#include "Yap/YapPromptHandle.h"
#include "Yap/Enums/YapMaturitySetting.h"

#define LOCTEXT_NAMESPACE "Yap"

TOptional<bool> UYapBroker::bImplemented_OnConversationOpened = false;
TOptional<bool> UYapBroker::bImplemented_OnConversationClosed = false;
TOptional<bool> UYapBroker::bImplemented_OnDialogueBegins = false;
TOptional<bool> UYapBroker::bImplemented_OnDialogueEnds = false;
TOptional<bool> UYapBroker::bImplemented_AddPlayerPrompt = false;
TOptional<bool> UYapBroker::bImplemented_AfterPlayerPromptsAdded = false;
TOptional<bool> UYapBroker::bImplemented_UseMatureDialogue = false;
TOptional<bool> UYapBroker::bImplemented_CalculateWordCount = false;
TOptional<bool> UYapBroker::bImplemented_CalculateTextTime = false;
TOptional<bool> UYapBroker::bImplemented_GetAudioAssetDuration = false;
#if WITH_EDITOR
TOptional<bool> UYapBroker::bImplemented_PreviewAudioAsset = false;
#endif

#if WITH_EDITOR
bool UYapBroker::bWarned_OnConversationOpened = false;
bool UYapBroker::bWarned_OnConversationClosed = false;
bool UYapBroker::bWarned_OnDialogueBegins = false;
bool UYapBroker::bWarned_OnDialogueEnds = false;
bool UYapBroker::bWarned_AddPlayerPrompt = false;
bool UYapBroker::bWarned_AfterPlayerPromptsAdded = false;
bool UYapBroker::bWarned_UseMatureDialogue = false;
bool UYapBroker::bWarned_CalculateWordCount = false;
bool UYapBroker::bWarned_CalculateTextTime = false;
bool UYapBroker::bWarned_GetAudioAssetDuration = false;
bool UYapBroker::bWarned_PreviewAudioAsset = false;
#endif
// ================================================================================================
#if WITH_EDITOR

// Used to check to see if a derived class actually implemented PlayDialogueAudioAsset_Editor()
thread_local bool bPreviewAudioAssetOverridden = false;
thread_local bool bSuppressPreviewAudioAssetWarning = false;

/*
bool UYapBroker::K2_PreviewAudioAsset_Implementation(const UObject* AudioAsset) const
{
	bPreviewAudioAssetOverridden = false;

	if (!bSuppressPreviewAudioAssetWarning)
	{
		UE_LOG(LogYap, Warning, TEXT("Unimplemented broker function: %s"), *FString(__func__));
	}
	
	return false;
}
*/
#endif

// ------------------------------------------------------------------------------------------------

/*
float UYapBroker::K2_GetAudioAssetDuration_Implementation(const UObject* AudioAsset) const
{
	UE_LOG(LogYap, Warning, TEXT("Unimplemented broker function: %s"), *FString(__func__));

	return -1;
}
*/

/*
int32 UYapBroker::K2_CalculateWordCount_Implementation(const FText& Text)
{
	// Utility to count the number of words within a string (we use a line-break iterator to avoid counting the whitespace between the words)
	TSharedRef<IBreakIterator> LineBreakIterator = FBreakIterator::CreateLineBreakIterator();
	auto CountWords = [&LineBreakIterator](const FString& InTextToCount) -> int32
	{
		int32 NumWords = 0;
		LineBreakIterator->SetString(InTextToCount);

		int32 PreviousBreak = 0;
		int32 CurrentBreak;

		while ((CurrentBreak = LineBreakIterator->MoveToNext()) != INDEX_NONE)
		{
			if (CurrentBreak > PreviousBreak)
			{
				++NumWords;
			}
			PreviousBreak = CurrentBreak;
		}

		LineBreakIterator->ClearString();
		return NumWords;
	};

	return CountWords(Text.ToString());
}
*/

/*
float UYapBroker::K2_CalculateTextTime_Implementation(int32 WordCount, int32 CharCount)
{
	int32 TWPM = UYapProjectSettings::GetTextWordsPerMinute();
	double Min = UYapProjectSettings::GetMinimumAutoTextTimeLength();
	double SecondsPerWord = 60.0 / (double)TWPM;

	return FMath::Max(WordCount * SecondsPerWord, Min);
}
*/

#if WITH_EDITOR

// ============================================================================================
// C++ OVERRIDES
// ============================================================================================

// If the C++ function is not overridden, we attempt to use the blueprint function. If the blueprint function is not implemented, we log a warning once per PIE.
#define YAP_BROKER_CPP_CALL_K2(FUNC, LOG_CONDITION, DEFAULTRETURN, ...)\
	/* The Initialize function should have been called to set all of these variables. */\
	check(bImplemented_##FUNC.IsSet());\
	\
	if (bImplemented_##FUNC.GetValue())\
	{\
		return K2_##FUNC(__VA_ARGS__);\
	}\
	else\
	{\
		if (!bWarned_##FUNC)\
		{\
			if (LOG_CONDITION)\
			{\
				UE_LOG(LogYap, Warning, TEXT("Unimplemented broker function: %s"), *FString(__func__));\
			}\
			\
			bWarned_##FUNC = true;\
		}\
	}\
	return DEFAULTRETURN

template<typename TFunction>
struct TResolveFunctionReturn
{
	using Type = std::invoke_result_t<TFunction, UYapBroker>;
};

#define YAP_QUOTE(X) #X
#define YAP_CALL_K2(FUNCTION, ...) CallK2Function<&UYapBroker::K2_##FUNCTION>(YAP_QUOTE(FUNCTION), bImplemented_##FUNCTION, bWarned_##FUNCTION, __VA_ARGS__)

void UYapBroker::OnConversationOpened(const FGameplayTag& Conversation) const
{
	bool bShowUnimplementedWarning = true;

	YAP_CALL_K2(OnConversationOpened, bShowUnimplementedWarning, /*Args*/ Conversation);
}

void UYapBroker::OnConversationClosed(const FGameplayTag& Conversation) const
{
	bool bShowUnimplementedWarning = true;
	
	YAP_CALL_K2(OnConversationClosed, bShowUnimplementedWarning, /*Args*/ Conversation);
}

void UYapBroker::OnDialogueBegins(const FGameplayTag& Conversation, FYapDialogueHandle DialogueHandle, const UYapCharacter* DirectedAt, const UYapCharacter* Speaker, const FGameplayTag& MoodKey, const FText& DialogueText, const FText& TitleText, float DialogueTime, const UObject* AudioAsset) const
{
	bool bShowUnimplementedWarning = true;
	
	YAP_CALL_K2(OnDialogueBegins, bShowUnimplementedWarning, /*Args*/ Conversation, DialogueHandle, DirectedAt, Speaker, MoodKey, DialogueText, TitleText, DialogueTime, AudioAsset);
}

void UYapBroker::OnDialogueEnds(const FGameplayTag& Conversation, FYapDialogueHandle DialogueHandle) const
{
	bool bShowUnimplementedWarning = true;
	
	YAP_CALL_K2(OnDialogueEnds, bShowUnimplementedWarning, /*Args*/ Conversation, DialogueHandle);
}

void UYapBroker::AddPlayerPrompt(const FGameplayTag& Conversation, FYapPromptHandle Handle, const UYapCharacter* DirectedAt, const UYapCharacter* Speaker, const FGameplayTag& MoodKey, const FText& DialogueText, const FText& TitleText) const
{
	bool bShowUnimplementedWarning = true;
	
	YAP_CALL_K2(AddPlayerPrompt, bShowUnimplementedWarning, /*Args*/ Conversation, Handle, DirectedAt, Speaker, MoodKey, DialogueText, TitleText);
}

void UYapBroker::AfterPlayerPromptsAdded(const FGameplayTag& Conversation) const
{
	bool bShowUnimplementedWarning = true;

	YAP_CALL_K2(AfterPlayerPromptsAdded, bShowUnimplementedWarning, /*Args*/ Conversation);
}

EYapMaturitySetting UYapBroker::UseMatureDialogue() const
{
	bool bShowUnimplementedWarning = !UYapProjectSettings::GetSuppressDefaultMatureWarning();
	
	return YAP_CALL_K2(UseMatureDialogue, bShowUnimplementedWarning);
}

int32 UYapBroker::CalculateWordCount(const FText& Text) const
{
	check(bImplemented_CalculateWordCount.IsSet());
	
	if (bImplemented_CalculateWordCount)
	{
		return K2_CalculateWordCount(Text);
	}

	// Default Implementation
	
	// Utility to count the number of words within a string (we use a line-break iterator to avoid counting the whitespace between the words)
	TSharedRef<IBreakIterator> LineBreakIterator = FBreakIterator::CreateLineBreakIterator();
	auto CountWords = [&LineBreakIterator](const FString& InTextToCount) -> int32
	{
		int32 NumWords = 0;
		LineBreakIterator->SetString(InTextToCount);

		int32 PreviousBreak = 0;
		int32 CurrentBreak;

		while ((CurrentBreak = LineBreakIterator->MoveToNext()) != INDEX_NONE)
		{
			if (CurrentBreak > PreviousBreak)
			{
				++NumWords;
			}
			PreviousBreak = CurrentBreak;
		}

		LineBreakIterator->ClearString();
		return NumWords;
	};

	return CountWords(Text.ToString());
}

float UYapBroker::CalculateTextTime(int32 WordCount, int32 CharCount) const
{
	check(bImplemented_CalculateTextTime.IsSet());

	if (bImplemented_CalculateTextTime)
	{
		return K2_CalculateTextTime(WordCount, CharCount);
	}

	// Default Implementation
	
	int32 TWPM = UYapProjectSettings::GetTextWordsPerMinute();
	double Min = UYapProjectSettings::GetMinimumAutoTextTimeLength();
	double SecondsPerWord = 60.0 / (double)TWPM;

	return FMath::Max(WordCount * SecondsPerWord, Min);
}

float UYapBroker::GetAudioAssetDuration(const UObject* AudioAsset) const
{
	bool bShowUnimplementedWarning = true; // TODO true if audio classes aren't set to default unreal classes, false otherwise?
	
	return YAP_CALL_K2(GetAudioAssetDuration, bShowUnimplementedWarning, AudioAsset);
}

bool UYapBroker::PreviewAudioAsset(const UObject* AudioAsset) const
{
	bool bShowUnimplementedWarning = true; // TODO true if audio classes aren't set to default unreal classes, false otherwise?
	
	return YAP_CALL_K2(PreviewAudioAsset, bShowUnimplementedWarning, AudioAsset);
}

void UYapBroker::Initialize()
{
#if WITH_EDITOR
	bWarned_OnConversationOpened = false;
	bWarned_OnConversationClosed = false;
	bWarned_OnDialogueBegins = false;
	bWarned_OnDialogueEnds = false;
	bWarned_AddPlayerPrompt = false;
	bWarned_AfterPlayerPromptsAdded = false;
	bWarned_UseMatureDialogue = false;
	bWarned_CalculateWordCount = false;
	bWarned_CalculateTextTime = false;
	bWarned_GetAudioAssetDuration = false;
	bWarned_PreviewAudioAsset = false;
#endif
	
	bImplemented_OnConversationOpened = GetClass()->IsFunctionImplementedInScript(GET_FUNCTION_NAME_CHECKED(UYapBroker, K2_OnConversationOpened));
	bImplemented_OnConversationClosed = GetClass()->IsFunctionImplementedInScript(GET_FUNCTION_NAME_CHECKED(UYapBroker, K2_OnConversationClosed));
	bImplemented_OnDialogueBegins = GetClass()->IsFunctionImplementedInScript(GET_FUNCTION_NAME_CHECKED(UYapBroker, K2_OnDialogueBegins));
	bImplemented_OnDialogueEnds = GetClass()->IsFunctionImplementedInScript(GET_FUNCTION_NAME_CHECKED(UYapBroker, K2_OnDialogueEnds));
	bImplemented_AddPlayerPrompt = GetClass()->IsFunctionImplementedInScript(GET_FUNCTION_NAME_CHECKED(UYapBroker, K2_AddPlayerPrompt));
	bImplemented_AfterPlayerPromptsAdded = GetClass()->IsFunctionImplementedInScript(GET_FUNCTION_NAME_CHECKED(UYapBroker, K2_AfterPlayerPromptsAdded));
	bImplemented_UseMatureDialogue = GetClass()->IsFunctionImplementedInScript(GET_FUNCTION_NAME_CHECKED(UYapBroker, K2_UseMatureDialogue));
	bImplemented_CalculateWordCount = GetClass()->IsFunctionImplementedInScript(GET_FUNCTION_NAME_CHECKED(UYapBroker, K2_CalculateWordCount));
	bImplemented_CalculateTextTime = GetClass()->IsFunctionImplementedInScript(GET_FUNCTION_NAME_CHECKED(UYapBroker, K2_CalculateTextTime));
	bImplemented_GetAudioAssetDuration = GetClass()->IsFunctionImplementedInScript(GET_FUNCTION_NAME_CHECKED(UYapBroker, K2_GetAudioAssetDuration));
#if WITH_EDITOR
	bImplemented_PreviewAudioAsset = GetClass()->IsFunctionImplementedInScript(GET_FUNCTION_NAME_CHECKED(UYapBroker, K2_PreviewAudioAsset));
#endif
}

bool UYapBroker::PreviewAudioAsset_Internal(const UObject* AudioAsset) const
{
	bPreviewAudioAssetOverridden = true;

	return PreviewAudioAsset(AudioAsset);
}

bool UYapBroker::ImplementsPreviewAudioAsset_Internal() const
{
	bSuppressPreviewAudioAssetWarning = true;
	(void)PreviewAudioAsset_Internal(nullptr);
	bSuppressPreviewAudioAssetWarning = false;

	return bPreviewAudioAssetOverridden;
}

#endif // WITH_EDITOR

#undef LOCTEXT_NAMESPACE