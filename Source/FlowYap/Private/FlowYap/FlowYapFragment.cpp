#include "FlowYap/FlowYapFragment.h"

#include "AkAudioEvent.h"
#include "FlowYap/FlowYapProjectSettings.h"

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
}

const UAkAudioEvent* FFlowYapFragment::GetDialogueAudio() const
{
	return DialogueAudio;
}

void FFlowYapFragment::SetDialogueAudio(UAkAudioEvent* NewAudio)
{
	DialogueAudio = NewAudio;
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

double FFlowYapFragment::GetCalculatedTimedValue() const
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
			return CalculateTimeFromText();
		}
	case EFlowYapTimedMode::AutomaticFromAudio:
		{
			return CalculateTimeFromAudio();
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

double FFlowYapFragment::CalculateTimeFromText() const
{
	// TODO
	return 1.0;
}

double FFlowYapFragment::CalculateTimeFromAudio() const
{
	// TODO
	return 1.0;
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

#if WITH_EDITOR
void FFlowYapFragment::SetDialogueAudioFromAsset(const FAssetData& AssetData)
{
	DialogueAudio = Cast<UAkAudioEvent>(AssetData.GetAsset());
}

bool FFlowYapFragment::HasDialogueAudioAsset() const
{
	return !!DialogueAudio;
}
#endif
