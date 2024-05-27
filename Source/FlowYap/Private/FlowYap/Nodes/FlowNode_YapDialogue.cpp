#include "FlowYap/Nodes/FlowNode_YapDialogue.h"

#include "AkAudioEvent.h"
#include "FlowYap/FlowYapCharacter.h"
#include "FlowYap/FlowYapLog.h"
#include "FlowYap/FlowYapProjectSettings.h"

UFlowNode_YapDialogue::UFlowNode_YapDialogue()
{
	NodeColor = FLinearColor::Black;
}

bool UFlowNode_YapDialogue::GetTimed() const
{
	return bTimed;
}

bool UFlowNode_YapDialogue::GetUseAutoTime() const
{
	return bUseAutoTime;
}

bool UFlowNode_YapDialogue::GetUseAudioAssetLength() const
{
	if (!bTimed)
	{
		return false;
	}
	
	return bUseAudioAssetLength;
}

double UFlowNode_YapDialogue::GetTime() const
{
	return Time;
}

bool UFlowNode_YapDialogue::GetUserInterruptible() const
{
	return bUserInterruptible;
}

void UFlowNode_YapDialogue::SetTimed(bool NewValue)
{
	bTimed = NewValue;
}

void UFlowNode_YapDialogue::SetUseAutoTime(bool NewValue)
{
	bUseAutoTime = NewValue;
}

void UFlowNode_YapDialogue::SetUseAudioAssetLength(bool NewValue)
{
	bUseAudioAssetLength = NewValue;
}

void UFlowNode_YapDialogue::SetTime(double NewValue)
{
	Time = NewValue;
}

void UFlowNode_YapDialogue::SetUserInterruptible(bool NewValue)
{
	bUserInterruptible = NewValue;
}
