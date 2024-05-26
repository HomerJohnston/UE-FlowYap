#pragma once

#include "FlowYapFragment.generated.h"

class UAkAudioEvent;

USTRUCT(BlueprintType)
struct FFlowYapFragment
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, meta=(MultiLine=true))
	FText TitleText;
	
	UPROPERTY(EditAnywhere, meta=(MultiLine=true))
	FText DialogueText;

	// TODO soft pointer support for audio
	UPROPERTY(EditAnywhere)
	TObjectPtr<UAkAudioEvent> DialogueAudio;
};