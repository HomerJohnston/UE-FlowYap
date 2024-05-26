#include "FlowYap/Nodes/FlowNode_YapDialogue.h"

#include "AkAudioEvent.h"
#include "FlowYap/FlowYapCharacter.h"
#include "FlowYap/FlowYapCharacterMood.h"
#include "FlowYap/FlowYapProjectSettings.h"

UFlowNode_YapDialogue::UFlowNode_YapDialogue()
{
	Category = TEXT("Yap");
	NodeStyle = EFlowNodeStyle::Custom;
	NodeColor = FLinearColor::Black;

	SetNumberedOutputPins(0, 0);
}

FText UFlowNode_YapDialogue::GetSpeakerName() const
{
	if (!Character)
	{
		return INVTEXT("NO CHARACTER SET");
	}

	return Character->GetEntityName();
}

FLinearColor UFlowNode_YapDialogue::GetSpeakerColor() const
{
	if (!Character)
	{
		return FLinearColor::Gray;
	}
	
	return Character->GetEntityColor();
}

const UTexture2D* UFlowNode_YapDialogue::GetDefaultSpeakerPortrait() const
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

const UTexture2D* UFlowNode_YapDialogue::GetSpeakerPortrait(const FName& RequestedPortraitKey) const
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

const FFlowYapFragment& UFlowNode_YapDialogue::GetFragment() const
{
	return Fragment;
}

FText UFlowNode_YapDialogue::GetTitleText() const
{
	return Fragment.TitleText;
}

FText UFlowNode_YapDialogue::GetDialogueText() const
{
	return Fragment.DialogueText;
}

UAkAudioEvent* UFlowNode_YapDialogue::GetDialogueAudio() const
{
	 return Fragment.DialogueAudio;
}

FText UFlowNode_YapDialogue::GetNodeTitle() const
{
	if (!Character)
	{
		return Super::GetNodeTitle();
	}
	
	return FText::Join(FText::FromString(": "), Super::GetNodeTitle(), GetSpeakerName());
}

void UFlowNode_YapDialogue::SetPortraitKey(const FName& NewValue)
{
	PortraitKey = NewValue;
}

FName UFlowNode_YapDialogue::GetPortraitKey() const
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
void UFlowNode_YapDialogue::SetDialogueText(const FText& CommittedText)
{
	Fragment.DialogueText = CommittedText;
}

void UFlowNode_YapDialogue::SetTitleText(const FText& CommittedText)
{
	Fragment.TitleText = CommittedText;
}

bool UFlowNode_YapDialogue::GetDynamicTitleColor(FLinearColor& OutColor) const
{
	if (!Character)
	{
		return Super::GetDynamicTitleColor(OutColor);
	}

	OutColor = Character->GetEntityColor();

	return true;
}

FSlateBrush* UFlowNode_YapDialogue::GetSpeakerPortraitBrush(const FName& RequestedPortraitKey) const
{
	return Character->GetPortraitBrush(RequestedPortraitKey);
}

void UFlowNode_YapDialogue::SetDialogueAudioAsset(const FAssetData& AssetData)
{
	//BeginModify(FText::FromString(FString("Set AkGeometry Texture")));

	Fragment.DialogueAudio = Cast<UAkAudioEvent>(AssetData.GetAsset());

	//EndModify();
}
#endif
