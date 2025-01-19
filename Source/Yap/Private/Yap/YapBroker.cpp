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
TOptional<bool> UYapBroker::bImplemented_GetPlaybackSpeed = false;
TOptional<bool> UYapBroker::bImplemented_CalculateWordCount = false;
TOptional<bool> UYapBroker::bImplemented_CalculateTextTime = false;
TOptional<bool> UYapBroker::bImplemented_GetAudioAssetDuration = false;
#if WITH_EDITOR
TOptional<bool> UYapBroker::bImplemented_PreviewAudioAsset = false;
#endif

bool UYapBroker::bWarned_OnConversationOpened = false;
bool UYapBroker::bWarned_OnConversationClosed = false;
bool UYapBroker::bWarned_OnDialogueBegins = false;
bool UYapBroker::bWarned_OnDialogueEnds = false;
bool UYapBroker::bWarned_AddPlayerPrompt = false;
bool UYapBroker::bWarned_AfterPlayerPromptsAdded = false;
bool UYapBroker::bWarned_UseMatureDialogue = false;
bool UYapBroker::bWarned_GetPlaybackSpeed = false;
bool UYapBroker::bWarned_CalculateWordCount = false;
bool UYapBroker::bWarned_CalculateTextTime = false;
bool UYapBroker::bWarned_GetAudioAssetDuration = false;
#if WITH_EDITOR
bool UYapBroker::bWarned_PreviewAudioAsset = false;
#endif

// ============================================================================================
// C++ OVERRIDES
// ============================================================================================

#define YAP_QUOTE(X) #X

#define YAP_CALL_K2(FUNCTION, SHOW_UNIMPLEMENTED_WARNING, ...) CallK2Function<&UYapBroker::K2_##FUNCTION>(YAP_QUOTE(FUNCTION), bImplemented_##FUNCTION, bWarned_##FUNCTION, SHOW_UNIMPLEMENTED_WARNING __VA_OPT__(,) __VA_ARGS__)

void UYapBroker::OnConversationOpened(const FGameplayTag& Conversation) const
{
	YAP_CALL_K2(OnConversationOpened, true, Conversation);
}

void UYapBroker::OnConversationClosed(const FGameplayTag& Conversation) const
{
	YAP_CALL_K2(OnConversationClosed, true, Conversation);
}

void UYapBroker::OnDialogueBegins(const FGameplayTag& Conversation, FYapDialogueHandle DialogueHandle, const UYapCharacter* DirectedAt, const UYapCharacter* Speaker, const FGameplayTag& MoodKey, const FText& DialogueText, const FText& TitleText, float DialogueTime, const UObject* AudioAsset) const
{
	YAP_CALL_K2(OnDialogueBegins, true, Conversation, DialogueHandle, DirectedAt, Speaker, MoodKey, DialogueText, TitleText, DialogueTime, AudioAsset);
}

void UYapBroker::OnDialogueEnds(const FGameplayTag& Conversation, FYapDialogueHandle DialogueHandle) const
{
	YAP_CALL_K2(OnDialogueEnds, true, Conversation, DialogueHandle);
}

void UYapBroker::AddPlayerPrompt(const FGameplayTag& Conversation, FYapPromptHandle Handle, const UYapCharacter* DirectedAt, const UYapCharacter* Speaker, const FGameplayTag& MoodKey, const FText& DialogueText, const FText& TitleText) const
{
	YAP_CALL_K2(AddPlayerPrompt, true, Conversation, Handle, DirectedAt, Speaker, MoodKey, DialogueText, TitleText);
}

void UYapBroker::AfterPlayerPromptsAdded(const FGameplayTag& Conversation) const
{
	YAP_CALL_K2(AfterPlayerPromptsAdded, true, Conversation);
}

EYapMaturitySetting UYapBroker::UseMatureDialogue() const
{
	bool bSuppressDefaultMatureWarning = !UYapProjectSettings::GetSuppressDefaultMatureWarning();

	return YAP_CALL_K2(UseMatureDialogue, bSuppressDefaultMatureWarning);
}

float UYapBroker::GetPlaybackSpeed() const
{
	return YAP_CALL_K2(GetPlaybackSpeed, false);
}

int32 UYapBroker::CalculateWordCount(const FText& Text) const
{
	check(bImplemented_CalculateWordCount.IsSet());
	
	if (bImplemented_CalculateWordCount)
	{
		return K2_CalculateWordCount(Text);
	}

	// ------------------------------------------
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

	// ------------------------------------------
	// Default Implementation
	
	int32 TWPM = UYapProjectSettings::GetTextWordsPerMinute();
	double Min = UYapProjectSettings::GetMinimumAutoTextTimeLength();
	double SecondsPerWord = 60.0 / (double)TWPM;

	return FMath::Max(WordCount * SecondsPerWord, Min);
}

float UYapBroker::GetAudioAssetDuration(const UObject* AudioAsset) const
{
	float Time = -1;
	
#if WITH_EDITOR
	const TArray<TSoftClassPtr<UObject>>& AudioAssetClasses = UYapProjectSettings::GetAudioAssetClasses();

	bool bFoundClassMatch = false;
	
	for (const TSoftClassPtr<UObject>& Class : AudioAssetClasses)
	{
		if (Class.IsPending())
		{
			UE_LOG(LogYap, Warning, TEXT("Async loading audio class asset - this should not happen!"));
		}
		
		if (AudioAsset->IsA(Class.LoadSynchronous()))
		{
			bFoundClassMatch = true;
			break;
		}
	}

	if (!bFoundClassMatch)
	{
		FString ProjectAudioClassesString;
		
		for (int32 i = 0; i < AudioAssetClasses.Num(); ++i)
		{
			const TSoftClassPtr<UObject>& Class = AudioAssetClasses[i];
			
			ProjectAudioClassesString += Class->GetName();

			if (i < AudioAssetClasses.Num() - 1)
			{
				ProjectAudioClassesString += ", ";
			}
		}
		
		UE_LOG(LogYap, Error, TEXT("Failed to match [%s] to a valid audio asset class! Asset type: [%s], project asset types: [%s]"), *AudioAsset->GetPathName(), *AudioAsset->GetClass()->GetName(), *ProjectAudioClassesString);
	}
#endif
	
	if (UYapProjectSettings::HasCustomAudioAssetClasses())
	{
		bool bShowUnimplementedWarning = true; // TODO true if audio classes aren't set to default unreal classes, false otherwise?
		Time = YAP_CALL_K2(GetAudioAssetDuration, bShowUnimplementedWarning, AudioAsset);
	}
	else
	{
		// ------------------------------------------
		// Default Implementation
		const USoundBase* AudioAssetAsSoundBase = Cast<USoundBase>(AudioAsset);

		if (AudioAssetAsSoundBase)
		{
			Time = AudioAssetAsSoundBase->GetDuration();
		}
	}
	
	if (Time < 0)
	{
		UE_LOG(LogYap, Error, TEXT("Failed to determine audio asset duration, unknown error!"));
	}

	return Time;
}

#if WITH_EDITOR
bool UYapBroker::PreviewAudioAsset(const UObject* AudioAsset) const
{
	bool bShowUnimplementedWarning = true; // TODO true if audio classes aren't set to default unreal classes, false otherwise?
	
	return YAP_CALL_K2(PreviewAudioAsset, bShowUnimplementedWarning, AudioAsset);
}
#endif

void UYapBroker::Initialize()
{
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
#if WITH_EDITOR
	bWarned_PreviewAudioAsset = false;
#endif // WITH_EDITOR
	
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
#endif // WITH_EDITOR
}

#if WITH_EDITOR
// Used to check to see if a derived class actually implemented PlayDialogueAudioAsset_Editor()
thread_local bool bPreviewAudioAssetOverridden = false;
thread_local bool bSuppressPreviewAudioAssetWarning = false;
#endif // WITH_EDITOR

#if WITH_EDITOR
bool UYapBroker::PreviewAudioAsset_Internal(const UObject* AudioAsset) const
{
	bPreviewAudioAssetOverridden = true;

	return PreviewAudioAsset(AudioAsset);
}
#endif // WITH_EDITOR

#if WITH_EDITOR
bool UYapBroker::ImplementsPreviewAudioAsset_Internal() const
{
	bSuppressPreviewAudioAssetWarning = true;
	(void)PreviewAudioAsset_Internal(nullptr);
	bSuppressPreviewAudioAssetWarning = false;

	return bPreviewAudioAssetOverridden;
}
#endif // WITH_EDITOR

#undef LOCTEXT_NAMESPACE