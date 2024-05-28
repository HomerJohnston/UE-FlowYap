#include "FlowYap/Nodes/FlowNode_YapSpeechBase.h"

#include "AkAudioEvent.h"
#include "FlowYap/FlowYapCharacter.h"
#include "FlowYap/FlowYapLog.h"
#include "FlowYap/FlowYapProjectSettings.h"

UFlowNode_YapSpeechBase::UFlowNode_YapSpeechBase()
{
	Category = TEXT("Yap");
	NodeStyle = EFlowNodeStyle::Custom;

	SetNumberedOutputPins(0, 0);
}

void UFlowNode_YapSpeechBase::SetConversationName(FName Name)
{
	if (ConversationName != NAME_None)
	{
		UE_LOG(FlowYap, Warning, TEXT("Tried to set %s node to conversation %s but node already had a conversation set! Ignoring."), *GetNodeTitle().ToString(), *Name.ToString());
		return;
	}

	ConversationName = Name;
}

FText UFlowNode_YapSpeechBase::GetSpeakerName() const
{
	if (!Character)
	{
		return INVTEXT("NO CHARACTER SET");
	}

	return Character->GetEntityName();
}

FLinearColor UFlowNode_YapSpeechBase::GetSpeakerColor() const
{
	if (!Character)
	{
		return FLinearColor::Gray;
	}
	
	return Character->GetEntityColor();
}

const UTexture2D* UFlowNode_YapSpeechBase::GetDefaultSpeakerPortrait() const
{
	if (!Character)
	{
		return nullptr;
	}

	const UFlowYapProjectSettings* Settings = GetDefault<UFlowYapProjectSettings>();

	if (Settings->GetPortraitKeys().Num() == 0)
	{
		return nullptr;
	}

	const FName& Key = Settings->GetPortraitKeys()[0];
	
	return GetSpeakerPortrait(Key);
}

const UTexture2D* UFlowNode_YapSpeechBase::GetSpeakerPortrait(const FName& RequestedPortraitKey) const
{
	if (!Character)
	{
		return nullptr;
	}

	const TObjectPtr<UTexture2D>* Portrait = Character->GetPortraits().Find(RequestedPortraitKey);

	if (Portrait)
	{
		return *Portrait;
	}
	else
	{
		return nullptr;
	}
}

const FFlowYapFragment& UFlowNode_YapSpeechBase::GetFragment() const
{
	return Fragment;
}

FText UFlowNode_YapSpeechBase::GetTitleText() const
{
	return Fragment.TitleText;
}

FText UFlowNode_YapSpeechBase::GetDialogueText() const
{
	return Fragment.DialogueText;
}

UAkAudioEvent* UFlowNode_YapSpeechBase::GetDialogueAudio() const
{
	 return Fragment.DialogueAudio;
}

FText UFlowNode_YapSpeechBase::GetNodeTitle() const
{
	if (!Character)
	{
		return Super::GetNodeTitle();
	}
	
	return FText::Join(FText::FromString(": "), Super::GetNodeTitle(), GetSpeakerName());
}

void UFlowNode_YapSpeechBase::SetPortraitKey(const FName& NewValue)
{
	PortraitKey = NewValue;
}

FName UFlowNode_YapSpeechBase::GetPortraitKey() const
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

void UFlowNode_YapSpeechBase::InitializeInstance()
{
	UE_LOG(FlowYap, Warning, TEXT("InitializeInstance"));

	Super::InitializeInstance();
}

void UFlowNode_YapSpeechBase::OnActivate()
{
	UE_LOG(FlowYap, Warning, TEXT("OnActivate"));

	Super::OnActivate();
}

void UFlowNode_YapSpeechBase::ExecuteInput(const FName& PinName)
{
	UE_LOG(FlowYap, Warning, TEXT("ExecuteInput, conversation: %s"), *ConversationName.ToString());

	Super::ExecuteInput(PinName);
}

#if WITH_EDITOR
void UFlowNode_YapSpeechBase::SetDialogueText(const FText& CommittedText)
{
	Fragment.DialogueText = CommittedText;
}

void UFlowNode_YapSpeechBase::SetTitleText(const FText& CommittedText)
{
	Fragment.TitleText = CommittedText;
}

bool UFlowNode_YapSpeechBase::GetDynamicTitleColor(FLinearColor& OutColor) const
{
	if (!Character)
	{
		return Super::GetDynamicTitleColor(OutColor);
	}

	OutColor = Character->GetEntityColor();

	return true;
}

FSlateBrush* UFlowNode_YapSpeechBase::GetSpeakerPortraitBrush(const FName& RequestedPortraitKey) const
{
	if (Character)
	{
		return Character->GetPortraitBrush(RequestedPortraitKey);
	}

	return nullptr;
}

void UFlowNode_YapSpeechBase::SetDialogueAudioAsset(const FAssetData& AssetData)
{
	//BeginModify(FText::FromString(FString("Set AkGeometry Texture")));

	Fragment.DialogueAudio = Cast<UAkAudioEvent>(AssetData.GetAsset());

	//EndModify();
}
#endif

bool UFlowNode_YapSpeechBase::GetTimed() const
{
	return bTimed;
}

bool UFlowNode_YapSpeechBase::GetUseAutoTime() const
{
	return bUseAutoTime;
}

bool UFlowNode_YapSpeechBase::GetUseAudioAssetLength() const
{
	if (!bTimed)
	{
		return false;
	}
	
	return bUseAudioAssetLength;
}

double UFlowNode_YapSpeechBase::GetTime() const
{
	return Time;
}

bool UFlowNode_YapSpeechBase::GetUserInterruptible() const
{
	return bUserInterruptible;
}

void UFlowNode_YapSpeechBase::SetTimed(bool NewValue)
{
	bTimed = NewValue;
}

void UFlowNode_YapSpeechBase::SetUseAutoTime(bool NewValue)
{
	bUseAutoTime = NewValue;
}

void UFlowNode_YapSpeechBase::SetUseAudioAssetLength(bool NewValue)
{
	bUseAudioAssetLength = NewValue;
}

void UFlowNode_YapSpeechBase::SetTime(double NewValue)
{
	Time = NewValue;
}

void UFlowNode_YapSpeechBase::SetUserInterruptible(bool NewValue)
{
	bUserInterruptible = NewValue;
}
