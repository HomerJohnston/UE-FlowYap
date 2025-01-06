// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license.

#pragma once

#include "Nodes/FlowNode_YapDialogue.h"
#include "Yap/YapBit.h"
#include "YapBitReplacement.generated.h"

class UObject;
class UYapCharacter;

USTRUCT()
struct FYapBitReplacement
{
	GENERATED_BODY()

	FYapBitReplacement();
	
	/**  */
	UPROPERTY(EditAnywhere)
	TOptional<TSoftObjectPtr<UYapCharacter>> SpeakerAsset;

	/**  */
	UPROPERTY(EditAnywhere)
	TOptional<TSoftObjectPtr<UYapCharacter>> DirectedAtAsset;

	/**  */
	UPROPERTY(EditAnywhere)
	TOptional<FText> MatureTitleText;
	
	/**  */
	UPROPERTY(EditAnywhere)
	TOptional<FText> SafeTitleText;

	/**  */
	UPROPERTY(EditAnywhere)
	TOptional<FText> MatureDialogueText;
	
	/**  */
	UPROPERTY(EditAnywhere)
	TOptional<FText> SafeDialogueText;
	
	/**  */
	UPROPERTY(EditAnywhere)
	TOptional<TSoftObjectPtr<UObject>> MatureDialogueAudioAsset;
	
	/**  */
	UPROPERTY(EditAnywhere)
	TOptional<TSoftObjectPtr<UObject>> SafeDialogueAudioAsset;

	/**  */
	UPROPERTY(EditAnywhere)
	TOptional<FGameplayTag> MoodKey = FGameplayTag::EmptyTag;

	/**  */
	UPROPERTY(EditAnywhere)
	TOptional<EYapTimeMode> TimeMode = EYapTimeMode::AudioTime;

	/**  */
	UPROPERTY(EditAnywhere)
	TOptional<EYapDialogueSkippable> Skippable = EYapDialogueSkippable::Default;

	// TODO calculate and serialize below on PostEditChangeProperty

	/**  */
	UPROPERTY(EditAnywhere)
	TOptional<float> ManualTime = 0;

	/**  */
	UPROPERTY(VisibleAnywhere)
	TOptional<int32> CachedMatureWordCount = 0;

	/**  */
	UPROPERTY(VisibleAnywhere)
	TOptional<int32> CachedSafeWordCount = 0;
	
	/**  */
	UPROPERTY(VisibleAnywhere)
	TOptional<float> CachedMatureAudioTime = 0;

	/**  */
	UPROPERTY(VisibleAnywhere)
	TOptional<float> CachedSafeAudioTime = 0;
};

inline FYapBitReplacement::FYapBitReplacement()
{
	SpeakerAsset.Reset();
	DirectedAtAsset.Reset();

	// TODO investigate this more, why is FText TOptional crashing if I don't do this?
	MatureTitleText = FText::GetEmpty();
	MatureTitleText.Reset();

	SafeTitleText = FText::GetEmpty();
	SafeTitleText.Reset();

	MatureDialogueText = FText::GetEmpty();
	MatureDialogueText.Reset();
	
	SafeDialogueText = FText::GetEmpty();
	SafeDialogueText.Reset();
	
	MatureDialogueAudioAsset.Reset();
	SafeDialogueAudioAsset.Reset();
	
	MoodKey.Reset();
	TimeMode.Reset();
	Skippable.Reset();

	ManualTime.Reset();

	CachedMatureWordCount.Reset();
	CachedSafeWordCount.Reset();
	
	CachedMatureAudioTime.Reset();
	CachedSafeAudioTime.Reset();
}
