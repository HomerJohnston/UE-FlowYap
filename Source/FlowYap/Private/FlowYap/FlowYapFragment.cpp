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

bool FFlowYapFragment::GetIsTimed() const
{
	return SharedSettings.bIsTimed;
}

void FFlowYapFragment::SetIsTimed(bool bNewValue)
{
	SharedSettings.bIsTimed = bNewValue;
}

double FFlowYapFragment::GetTimeManualValue() const
{
	return SharedSettings.TimeManual;
}

void FFlowYapFragment::SetTimeManualValue(double NewValue)
{
	SharedSettings.TimeManual = NewValue;
}

bool FFlowYapFragment::GetUseAutoTime() const
{
	return SharedSettings.bUseAutoTime;
}

void FFlowYapFragment::SetUseAutoTime(bool bNewValue)
{
	SharedSettings.bUseAutoTime = bNewValue;
}

bool FFlowYapFragment::GetUseAudioTime() const
{
	return SharedSettings.bUseAudioLength;
}

void FFlowYapFragment::SetUseAudioTime(bool bNewValue)
{
	SharedSettings.bUseAudioLength = bNewValue;
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

double FFlowYapFragment::GetTimedValue() const
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

void FFlowYapFragment::SetUseProjectSettings(bool bNewValue)
{
	bUseProjectSettings = bNewValue;
}

bool FFlowYapFragment::GetUseProjectSettings() const
{
	return bUseProjectSettings;
}

#if WITH_EDITOR
void FFlowYapFragment::SetDialogueAudioFromAsset(const FAssetData& AssetData)
{
	DialogueAudio = Cast<UAkAudioEvent>(AssetData.GetAsset());
}

bool FFlowYapFragment::HasDialogueAudio() const
{
	return !!DialogueAudio;
}
#endif
