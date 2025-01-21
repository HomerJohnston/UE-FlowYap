// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "Yap/YapBlueprintFunctionLibrary.h"

#if WITH_EDITOR
#include "AssetTypeActions/AssetDefinition_SoundBase.h"
#endif

#include "Yap/YapDialogueHandle.h"
#include "Yap/YapLog.h"
#include "Yap/YapPromptHandle.h"
#include "Yap/Nodes/FlowNode_YapDialogue.h"

#define LOCTEXT_NAMESPACE "Yap"

#if WITH_EDITOR
void UYapBlueprintFunctionLibrary::PlaySoundInEditor(USoundBase* Sound)
{
	if (Sound)
	{
		GEditor->PlayPreviewSound(Sound);
	}
	else
	{
		UE_LOG(LogYap, Warning, TEXT("Sound was null"));
	}
}
#endif

float UYapBlueprintFunctionLibrary::GetSoundLength(USoundBase* Sound)
{
	return Sound->Duration;
}

bool UYapBlueprintFunctionLibrary::SkipDialogue(const FYapDialogueHandle& Handle)
{
	Handle.GetDialogueNode()->Skip(Handle.GetFragmentIndex());
	return false;
}

bool UYapBlueprintFunctionLibrary::RunPrompt(const FYapPromptHandle& Handle)
{
	return false;
}

void UYapBlueprintFunctionLibrary::InvalidateDialogueHandle(FYapDialogueHandle& Handle)
{
	Handle.Invalidate();
}

void UYapBlueprintFunctionLibrary::InvalidatePromptHandle(FYapPromptHandle& Handle)
{
	Handle.Invalidate();
}
#undef LOCTEXT_NAMESPACE
