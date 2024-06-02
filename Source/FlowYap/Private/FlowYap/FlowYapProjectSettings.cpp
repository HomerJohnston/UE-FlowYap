#include "FlowYap/FlowYapProjectSettings.h"

#include "EngineUtils.h"
#include "FlowYap/FlowYapEngineUtils.h"

UFlowYapProjectSettings::UFlowYapProjectSettings()
{
	PortraitKeyIconPath.Path = "";

	AudioTimeFallbackTimedMode = EFlowYapTimedMode::None;
	
	DialogueWidthAdjustment = 0;
}

#if WITH_EDITOR
FString UFlowYapProjectSettings::GetPortraitIconPath(FName Key) const
{
	if (PortraitKeyIconPath.Path == "")
	{
		return FFlowYapEngineUtils::GetFlowYapPluginDir() / FString::Format(TEXT("Resources/PortraitKeys/{1}.png"), { PortraitKeyIconPath.Path, Key.ToString() });
	}
	
	return FPaths::ProjectDir() / FString::Format(TEXT("{0}/{1}.png"), { PortraitKeyIconPath.Path, Key.ToString() });
}

const TArray<FName>& UFlowYapProjectSettings::GetPortraitKeys() const
{
	return PortraitKeys;
}

void UFlowYapProjectSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	FName PropertyName = PropertyChangedEvent.Property->GetFName();

	if (PropertyName == GET_MEMBER_NAME_CHECKED(ThisClass, PortraitKeys))
	{
		OnPortraitKeysChanged.Broadcast();
	}
}

const FFlowYapFragmentTimeSettings& UFlowYapProjectSettings::GetDefaultTimeSettings() const
{
	return DefaultTimeSettings;
}

EFlowYapTimedMode UFlowYapProjectSettings::GetAudioTimeFallbackTimedMode() const
{
	return AudioTimeFallbackTimedMode;
}

UClass* UFlowYapProjectSettings::GetDialogueAssetClass() const
{
	return DialogueAssetClass;
}

int32 UFlowYapProjectSettings::GetDialogueWidthAdjustment() const
{
	return DialogueWidthAdjustment;
}

bool UFlowYapProjectSettings::GetHideTitleTextOnNPCDialogueNodes() const
{
	return bHideTitleTextOnNPCDialogueNodes;
}
#endif
