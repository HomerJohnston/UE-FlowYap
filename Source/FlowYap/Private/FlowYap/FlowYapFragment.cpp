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

EFlowYapTimedMode FFlowYapFragment::GetTimedMode() const
{
	return SharedSettings.TimedMode;
}

void FFlowYapFragment::SetTimedMode(EFlowYapTimedMode NewValue)
{
	SharedSettings.TimedMode = NewValue;
}

void FFlowYapFragment::UnsetTimedMode(EFlowYapTimedMode NewValue)
{
	SetTimedMode(EFlowYapTimedMode::None);
}

double FFlowYapFragment::GetTimeEnteredValue() const
{
	return SharedSettings.EnteredTime;
}

void FFlowYapFragment::SetEnteredTimeValue(double NewValue)
{
	SharedSettings.EnteredTime = NewValue;
}

float FFlowYapFragment::GetEndPaddingTime() const
{
	return SharedSettings.EndPaddingTime;
}

void FFlowYapFragment::SetEndPaddingTime(float NewValue)
{
	SharedSettings.EndPaddingTime = NewValue;
}

bool FFlowYapFragment::GetUserInterruptible() const
{
	return SharedSettings.bUserInterruptible;
}

void FFlowYapFragment::SetUserInterruptible(bool bNewValue)
{
	SharedSettings.bUserInterruptible = bNewValue;
}

double FFlowYapFragment::GetCalculatedTimedValue() const
{
	check(false);
	return 0.0f;
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
