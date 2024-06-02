// Copyright Ghost Pepper Games, Inc. All Rights Reserved.

#include "FlowYap/FlowYapFragment.h"

#include "FlowYap/FlowYapProjectSettings.h"

#define LOCTEXT_NAMESPACE "FlowYap"

#if WITH_EDITOR
int64 FFlowYapFragment::NextID = 0;

FFlowYapFragment::FFlowYapFragment()
{
	EditorID = ++NextID;
}
#endif

int64 FFlowYapFragment::GetEditorID() const
{
	return EditorID;
}

const FText& FFlowYapFragment::GetTitleText() const
{
	return TitleText;
}

void FFlowYapFragment::SetTitleText(FText NewText)
{
	TitleText = NewText;
}

const FText& FFlowYapFragment::GetDialogueText() const
{
	return DialogueText;
}

void FFlowYapFragment::SetDialogueText(FText NewText)
{
	DialogueText = NewText;

	CacheWordCountFromText();
}

const UObject* FFlowYapFragment::GetDialogueAsset() const
{
	return DialogueAudio;
}

void FFlowYapFragment::SetDialogueAudio(UObject* NewAudio)
{
	DialogueAudio = NewAudio;

	CacheTimeFromAudio();
}

bool FFlowYapFragment::GetUsesProjectDefaultTimeSettings() const
{
	return bUseProjectDefaultTimeSettings;
}

void FFlowYapFragment::SetUseProjectDefaultTimeSettings(bool NewValue)
{
	bUseProjectDefaultTimeSettings = NewValue;
}

const FFlowYapFragmentTimeSettings& FFlowYapFragment::GetTimeSettings() const
{
	return TimeSettings;
}

EFlowYapTimedMode FFlowYapFragment::GetTimedMode() const
{
	return TimeSettings.TimedMode;
}

void FFlowYapFragment::SetTimedMode(EFlowYapTimedMode NewValue)
{
	TimeSettings.TimedMode = NewValue;
}

void FFlowYapFragment::UnsetTimedMode()
{
	SetTimedMode(EFlowYapTimedMode::None);
}

double FFlowYapFragment::GetEnteredTimeValue() const
{
	return TimeSettings.EnteredTime;
}

void FFlowYapFragment::SetEnteredTimeValue(double NewValue)
{
	TimeSettings.EnteredTime = NewValue;
}

float FFlowYapFragment::GetEndPaddingTime() const
{
	return TimeSettings.EndPaddingTime;
}

void FFlowYapFragment::SetEndPaddingTime(float NewValue)
{
	TimeSettings.EndPaddingTime = NewValue;
}

bool FFlowYapFragment::GetUserInterruptible() const
{
	return TimeSettings.bUserInterruptible;
}

void FFlowYapFragment::SetUserInterruptible(bool bNewValue)
{
	TimeSettings.bUserInterruptible = bNewValue;
}

EFlowYapTimedMode FFlowYapFragment::GetRuntimeTimedMode() const
{
	const FFlowYapFragmentTimeSettings& TimeSettingsRef = GetRuntimeTimeSettings();

	if (TimeSettingsRef.TimedMode == EFlowYapTimedMode::AutomaticFromAudio)
	{
		if (HasDialogueAudioAsset())
		{
			return EFlowYapTimedMode::AutomaticFromAudio;
		}
		else
		{
			return GetDefault<UFlowYapProjectSettings>()->GetAudioTimeFallbackTimedMode();
		}
	}

	return TimeSettings.TimedMode;
}

double FFlowYapFragment::GetRuntimeTimedValue() const
{
	const FFlowYapFragmentTimeSettings& RefTimeSettings = bUseProjectDefaultTimeSettings ? GetDefault<UFlowYapProjectSettings>()->GetDefaultTimeSettings() : TimeSettings;

	switch (RefTimeSettings.TimedMode)
	{
	case EFlowYapTimedMode::None:
		{
			return 0.0;
		}
	case EFlowYapTimedMode::UseEnteredTime:
		{
			return RefTimeSettings.EnteredTime;
		}
	case EFlowYapTimedMode::AutomaticFromText:
		{
			int32 TWPM = GetDefault<UFlowYapProjectSettings>()->GetTextWordsPerMinute();
			double Min = GetDefault<UFlowYapProjectSettings>()->GetMinimumAutoTextTimeLength();
			double SecondsPerWord = 60.0 / (double)TWPM;

			return FMath::Max(CachedWordCount * SecondsPerWord, Min);
		}
	case EFlowYapTimedMode::AutomaticFromAudio:
		{
			return CachedAudioTime;
		}
	default:
		{
			return 0.0;
		}
	}
}

const FFlowYapFragmentTimeSettings& FFlowYapFragment::GetRuntimeTimeSettings() const
{
	return bUseProjectDefaultTimeSettings ? GetDefault<UFlowYapProjectSettings>()->GetDefaultTimeSettings() : TimeSettings;
}

void FFlowYapFragment::CacheWordCountFromText()
{
	FString TextAsString = DialogueText.ToString();

	bool bInSpace = false;
	bool bInNewParagraph = false;

	// Since the system only clicks a word after a space or return line, "Ok." won't be counted. Simple workaround.
	int32 WordCount = 1;

	for (int i = 0; i < TextAsString.Len(); ++i)
	{
		TCHAR* Char = &TextAsString.GetCharArray().GetData()[i];

		switch (*Char)
		{
			case TCHAR('\n'):
			case TCHAR('\r'):
			{
				if (!bInNewParagraph)
				{
					WordCount += 2;
					bInNewParagraph = true;
				}
				break;
			}
			case TCHAR(' '):
			{
				if (!bInSpace)
				{
					++WordCount;
					bInSpace = true;
				}
				break;
			}
			default:
			{
				bInSpace = false;
				bInNewParagraph = false;
				break;
			}
		}
	}

	CachedWordCount = WordCount;
}

void FFlowYapFragment::CacheTimeFromAudio()
{
	// TODO
	// Make user build a UObject with a CDO to work as an interface to calculate this? 
	CachedAudioTime = 1.0;
}

void FFlowYapFragment::SetPortraitKey(const FName& NewValue)
{
	PortraitKey = NewValue;
}

FName FFlowYapFragment::GetPortraitKey() const
{
	if (PortraitKey == NAME_None)
	{
		const UFlowYapProjectSettings* ProjectSettings = GetDefault<UFlowYapProjectSettings>();

		if (ProjectSettings->GetPortraitKeys().Num() == 0)
		{
			return NAME_None;
		}
	
		return ProjectSettings->GetPortraitKeys()[0];
	}

	return PortraitKey;
}

int32 FFlowYapFragment::GetActivationCount() const
{
	return ActivationCount;
}

int32 FFlowYapFragment::GetActivationLimit() const
{
	return ActivationLimit;
}

#if WITH_EDITOR
void FFlowYapFragment::SetDialogueAudioFromAsset(const FAssetData& AssetData)
{
	DialogueAudio = AssetData.GetAsset();
}

bool FFlowYapFragment::HasDialogueAudioAsset() const
{
	return !!DialogueAudio;
}

void FFlowYapFragment::SetActivationLimit(int32 NewValue)
{
	ActivationLimit = NewValue;
}
#endif

#undef LOCTEXT_NAMESPACE