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

int64 FFlowYapFragment::GetEditorID()
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
	return bIsTimed;
}

void FFlowYapFragment::SetIsTimed(bool bNewValue)
{
	bIsTimed = bNewValue;
}

double FFlowYapFragment::GetTimeManualValue() const
{
	return TimeManual;
}

void FFlowYapFragment::SetTimeManualValue(double NewValue)
{
	TimeManual = NewValue;
}

bool FFlowYapFragment::GetUseAutoTime() const
{
	return bUseAutoTime;
}

void FFlowYapFragment::SetUseAutoTime(bool bNewValue)
{
	bUseAutoTime = bNewValue;
}

bool FFlowYapFragment::GetUseAudioTime() const
{
	return bUseAudioTime;
}

void FFlowYapFragment::SetUseAudioTime(bool bNewValue)
{
	bUseAudioTime = bNewValue;
}

float FFlowYapFragment::GetEndPaddingTime() const
{
	return EndPaddingTime;
}

void FFlowYapFragment::SetEndPaddingTime(float NewValue)
{
	EndPaddingTime = NewValue;
}

bool FFlowYapFragment::GetUserInterruptible() const
{
	return bUserInterruptible;
}

void FFlowYapFragment::SetUserInterruptible(bool bNewValue)
{
	bUserInterruptible = bNewValue;
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
