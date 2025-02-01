// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "Yap/YapBroker.h"

#include "Components/AudioComponent.h"
#include "Internationalization/BreakIterator.h"
#include "Yap/YapDialogueHandle.h"
#include "Yap/YapLog.h"
#include "Yap/YapProjectSettings.h"
#include "Yap/YapPromptHandle.h"
#include "Yap/Enums/YapMaturitySetting.h"

#define LOCTEXT_NAMESPACE "Yap"

TOptional<bool> UYapBroker::bImplemented_Initialize = false;
TOptional<bool> UYapBroker::bImplemented_GetMaturitySetting = false;
TOptional<bool> UYapBroker::bImplemented_GetPlaybackSpeed = false;
TOptional<bool> UYapBroker::bImplemented_GetAudioAssetDuration = false;
#if WITH_EDITOR
TOptional<bool> UYapBroker::bImplemented_PreviewAudioAsset = false;
#endif

bool UYapBroker::bWarned_Initialize = false;
bool UYapBroker::bWarned_GetMaturitySetting = false;
bool UYapBroker::bWarned_GetPlaybackSpeed = false;
bool UYapBroker::bWarned_GetAudioAssetDuration = false;
#if WITH_EDITOR
bool UYapBroker::bWarned_PreviewAudioAsset = false;
#endif

// ============================================================================================
// C++ OVERRIDES
// ============================================================================================

#define YAP_QUOTE(X) #X

#define YAP_CALL_K2(FUNCTION, SHOW_UNIMPLEMENTED_WARNING, ...) CallK2Function<&UYapBroker::K2_##FUNCTION>(YAP_QUOTE(FUNCTION), bImplemented_##FUNCTION, bWarned_##FUNCTION, SHOW_UNIMPLEMENTED_WARNING __VA_OPT__(,) __VA_ARGS__)

void UYapBroker::Initialize()
{
	YAP_CALL_K2(Initialize, false);
}

EYapMaturitySetting UYapBroker::GetMaturitySetting() const
{
	bool bSuppressDefaultMatureWarning = !UYapProjectSettings::GetSuppressBrokerWarnings();

	EYapMaturitySetting MaturitySetting = YAP_CALL_K2(GetMaturitySetting, bSuppressDefaultMatureWarning);

	if (MaturitySetting == EYapMaturitySetting::Unspecified)
	{
		return EYapMaturitySetting::Mature;
	}

	return MaturitySetting;
}

float UYapBroker::GetPlaybackSpeed() const
{
	return YAP_CALL_K2(GetPlaybackSpeed, false);
}

int32 UYapBroker::CalculateWordCount(const FText& Text) const
{
	if (GetClass()->IsFunctionImplementedInScript(GET_FUNCTION_NAME_CHECKED(UYapBroker, K2_CalculateWordCount))) // TODO cache this? Switch To YAP_CALL_K2?
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
	if (GetClass()->IsFunctionImplementedInScript(GET_FUNCTION_NAME_CHECKED(UYapBroker, K2_CalculateTextTime))) // TODO cache this? Switch To YAP_CALL_K2?
	{
		return K2_CalculateTextTime(WordCount, CharCount);
	}
	
	int32 TWPM = UYapProjectSettings::GetTextWordsPerMinute();
	float SecondsPerWord = 60.0 / (float)TWPM;
	float TalkTime = WordCount * SecondsPerWord * GetPlaybackSpeed();

	float Min = UYapProjectSettings::GetMinimumAutoTextTimeLength();
		
	return FMath::Max(TalkTime, Min);
}

float UYapBroker::GetAudioAssetDuration(const UObject* AudioAsset) const
{
	float Time = -1;

	if (!AudioAsset)
	{
		return Time;
	}
	
#if WITH_EDITOR
	const TArray<TSoftClassPtr<UObject>>& AudioAssetClasses = UYapProjectSettings::GetAudioAssetClasses();

	bool bFoundClassMatch = false;
	
	for (const TSoftClassPtr<UObject>& Class : AudioAssetClasses)
	{
		if (Class.IsPending())
		{
			UE_LOG(LogYap, Warning, TEXT("Synchronously loading audio class asset - this should not happen!"));
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

// Used to check to see if a derived class actually implemented PlayDialogueAudioAsset_Editor()
thread_local bool bPreviewAudioAssetOverridden = false;
thread_local bool bSuppressPreviewAudioAssetWarning = false;

bool UYapBroker::PreviewAudioAsset(const UObject* AudioAsset) const
{
	if (UYapProjectSettings::HasCustomAudioAssetClasses())
	{
		bool bShowUnimplementedWarning = true; // TODO true if audio classes aren't set to default unreal classes, false otherwise?
		return YAP_CALL_K2(PreviewAudioAsset, bShowUnimplementedWarning, AudioAsset);
	}
	else
	{
		// ------------------------------------------
		// Default Implementation
		static TWeakObjectPtr<UAudioComponent> PreviewAudioComponent; 
		
		if (const USoundBase* AudioAssetAsSoundBase = Cast<USoundBase>(AudioAsset))
		{
			if (PreviewAudioComponent.IsValid())
			{
				PreviewAudioComponent->Stop();
			}
			
			PreviewAudioComponent = GEditor->PlayPreviewSound(const_cast<USoundBase*>(AudioAssetAsSoundBase));
			return true;
		}
		else
		{
			if (!bSuppressPreviewAudioAssetWarning)
			{
				UE_LOG(LogYap, Warning, TEXT("Sound was null"));
			}
			return false;
		}
	}
}
#endif

#if WITH_EDITOR
FName UYapBroker::GenerateDialogueNodeTag(const UFlowNode_YapDialogue* InNode) const
{
	UFlowAsset* FlowAsset = InNode->GetFlowAsset();

	// Find all existing tags
	TSet<FName> ExistingTags;
	
	for (auto&[GUID, Node] : FlowAsset->GetNodes())
	{
		if (UFlowNode_YapDialogue* DialogueNode = Cast<UFlowNode_YapDialogue>(Node))
		{
			if (DialogueNode->GetDialogueTag().IsValid())
			{
				ExistingTags.Add(DialogueNode->GetDialogueTag().GetTagName());
			}
		}
	}

	if (!UYapProjectSettings::GetDialogueTagsParent().IsValid()) // TODO i should completely prevent this system from working if the parent tag isn't set
	{
		return NAME_None;
	}
	
	FString Path = FPackageName::GetLongPackagePath(FlowAsset->GetPackage()->GetName());
	Path.RemoveFromStart("/Game/");
	FString AssetName = FlowAsset->GetName();
	
	FString ProjectParentTag = UYapProjectSettings::GetDialogueTagsParent().GetTagName().ToString();
			
	// Generate a new tag, making sure it isn't already in use in the graph by another dialogue node
	FName NewGUID = NAME_None;
	int32 Safety = 0;
	do 
	{
		if (Safety > 1000)
		{
			UE_LOG(LogYap, Error, TEXT("Failed to generate a unique dialogue tag after 1000 iterations!"));
			NewGUID = NAME_None;
			break;
		}
		
		NewGUID = GenerateRandomDialogueNodeTag();

		NewGUID = FName(ProjectParentTag + "." + Path / AssetName + "." + NewGUID.ToString());
		
	} while (ExistingTags.Contains(NewGUID));

	return NewGUID;
}

FName UYapBroker::GenerateRandomDialogueNodeTag() const
{
	// TArray<char> AlphaNumerics {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F','G','H',/*'I',*/'J','K','L','M','N',/*'O',*/'P','Q','R','S','T','U','V','W','X','Y','Z'};
	TArray<char> AlphaNumerics {'A','B','C','D','E','F','G','H',/*'I',*/'J','K','L','M','N',/*'O',*/'P','Q','R','S','T','U','V','W','X','Y','Z'};

	const uint8 Size = 3;

	FString String;
	String.Reserve(Size);
	
	for (uint8 i = 0; i < Size; ++i)
	{
		uint8 RandIndex = FMath::RandHelper(AlphaNumerics.Num());
		String += AlphaNumerics[RandIndex];
	}

	return FName(String);
}

void GetFragmentID(const UFlowNode_YapDialogue* Node, const FString& DialogueTagString, int32& ID, int32 Index)
{
	FString PreviousTagString = Node->GetFragments()[Index].GetFragmentTag().ToString();

	if (PreviousTagString.StartsWith(DialogueTagString))
	{
		PreviousTagString = PreviousTagString.RightChop(DialogueTagString.Len() + 1); // chop off the parent tag and the .
	}

	if (PreviousTagString.IsNumeric())
	{
		ID = FCString::Atoi(*PreviousTagString);
	}
}

bool UYapBroker::GenerateFragmentTag(const UFlowNode_YapDialogue* Node, int32 FragmentIndex, FName& Result) const
{
	// Find all existing tags
	TSet<FName> ExistingTags;
	
	for (const FYapFragment& Fragment : Node->GetFragments())
	{
		if (Fragment.GetFragmentTag().IsValid())
		{
			ExistingTags.Add(Fragment.GetFragmentTag().GetTagName());
		}
	}
	
	if (!Node->GetDialogueTag().IsValid()) // TODO i should completely prevent this system from working if the parent tag isn't set
	{			
		return false;
	}
	
	FString ParentTag = Node->GetDialogueTag().ToString();
	
	// Generate a new tag, making sure it isn't already in use in the graph by another dialogue node
	FName NewGUID = NAME_None;
	int32 Safety = 0;
	do 
	{
		if (Safety > 1000)
		{
			UE_LOG(LogYap, Error, TEXT("Failed to generate a unique dialogue tag after 1000 iterations!"));
			NewGUID = NAME_None;
			break;
		}
		
		NewGUID = GenerateRandomFragmentTag();
		NewGUID = FName(ParentTag + "." + NewGUID.ToString());
	} while (ExistingTags.Contains(NewGUID));

	Result = NewGUID;
	return true;
	
	/*
	
	if (!ensure(IsValid(Node) && FragmentIndex >= 0 && FragmentIndex <= Node->GetNumFragments() + 1))
	{
		return false;
	}

	// Get existing surrounding fragment IDs
	int32 PreviousFragmentID = INDEX_NONE;
	int32 NewFragmentID = INDEX_NONE;
	int32 NextFragmentID = INDEX_NONE;

	FString DialogueTagString = Node->GetDialogueTag().ToString();

	if (FragmentIndex > 0)
	{
		GetFragmentID(Node, DialogueTagString, PreviousFragmentID, FragmentIndex - 1);
	}

	if (FragmentIndex < Node->GetNumFragments() - 1)
	{
		GetFragmentID(Node, DialogueTagString, NextFragmentID, FragmentIndex + 1);
	}

	// Inserting the first fragment ever
	if (FragmentIndex == 0 && Node->GetNumFragments() == 1)
	{
		NewFragmentID = 0;
	}

	// Adding a new fragment at the end
	else if (FragmentIndex == Node->GetNumFragments() - 1)
	{
		if (PreviousFragmentID != INDEX_NONE)
		{
			NewFragmentID = 10 * ((PreviousFragmentID / 10) + 1);
		}
	}

	// Inserting a fragment anywhere else
	else
	{
		// We can't autogenerate a tag if either surrounding tag is not automatically sequenced
		if (PreviousFragmentID == INDEX_NONE || NextFragmentID == INDEX_NONE || PreviousFragmentID > NextFragmentID)
		{
			return false;
		}

		// We can't autogenerate a tag if the next fragment is the next subsequence number (developer will have to manually intervene here)
		if (NextFragmentID - PreviousFragmentID <= 1)
		{
			return false;
		}

		// Figure out whether to increment the sequence number or the subsequence number
		if ((NextFragmentID / 10) - (PreviousFragmentID / 10) > 1)
		{
			NewFragmentID = 10 * ((PreviousFragmentID / 10) + 1);
		}
		else
		{
			NewFragmentID = PreviousFragmentID + 1;
		}
	}

	FNumberFormattingOptions Format;
	Format.MinimumIntegralDigits = 3;
	Format.UseGrouping = false;

	FString TagString = FText::AsNumber(NewFragmentID, &Format).ToString();

	Result = FName(TagString);
	
	return true;
	
	*/
}

FName UYapBroker::GenerateRandomFragmentTag() const
{
	TArray<char> AlphaNumerics {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};//'G','H',/*'I',*/'J','K','L','M','N',/*'O',*/'P','Q','R','S','T','U','V','W','X','Y','Z'};

	const uint8 Size = 3;

	FString String;
	String.Reserve(Size);
	
	for (uint8 i = 0; i < Size; ++i)
	{
		uint8 RandIndex = FMath::RandHelper(AlphaNumerics.Num());
		String += AlphaNumerics[RandIndex];
	}

	return FName(String);
}

#endif

void UYapBroker::Initialize_Internal()
{
	bWarned_Initialize = false;
	bWarned_GetMaturitySetting = false;
	bWarned_GetAudioAssetDuration = false;
#if WITH_EDITOR
	bWarned_PreviewAudioAsset = false;
#endif // WITH_EDITOR
	
	bImplemented_Initialize = GetClass()->IsFunctionImplementedInScript(GET_FUNCTION_NAME_CHECKED(UYapBroker, K2_Initialize));
	bImplemented_GetMaturitySetting = GetClass()->IsFunctionImplementedInScript(GET_FUNCTION_NAME_CHECKED(UYapBroker, K2_GetMaturitySetting));
	bImplemented_GetAudioAssetDuration = GetClass()->IsFunctionImplementedInScript(GET_FUNCTION_NAME_CHECKED(UYapBroker, K2_GetAudioAssetDuration));
#if WITH_EDITOR
	bImplemented_PreviewAudioAsset = GetClass()->IsFunctionImplementedInScript(GET_FUNCTION_NAME_CHECKED(UYapBroker, K2_PreviewAudioAsset));
#endif // WITH_EDITOR

	Initialize();
}

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