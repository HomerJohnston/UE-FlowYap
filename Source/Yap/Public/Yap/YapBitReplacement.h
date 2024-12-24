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
	TOptional<TSoftObjectPtr<UYapCharacter>> CharacterAsset;

	/**  */
	UPROPERTY(EditAnywhere)
	TOptional<FText> TitleText;

	/**  */
	UPROPERTY(EditAnywhere)
	TOptional<FText> DialogueText;

	/**  */
	UPROPERTY(EditAnywhere)
	TOptional<TSoftObjectPtr<UObject>> DialogueAudioAsset;

	/**  */
	UPROPERTY(EditAnywhere)
	TOptional<FGameplayTag> MoodKey = FGameplayTag::EmptyTag;

	/**  */
	UPROPERTY(EditAnywhere)
	TOptional<bool> bUseProjectDefaultTimeSettings = true;

	/**  */
	UPROPERTY(EditAnywhere)
	TOptional<EYapTimeMode> TimeMode = EYapTimeMode::AudioTime;

	/**  */
	UPROPERTY(EditAnywhere)
	TOptional<EYapDialogueSkippable> Skippable = EYapDialogueSkippable::Default;

	// TODO calculate and serialize below on PostEditChangeProperty

	/**  */
	UPROPERTY(EditAnywhere)
	TOptional<double> ManualTime = 0;

	/**  */
	UPROPERTY(VisibleAnywhere)
	TOptional<int32> CachedWordCount = 0;

	/**  */
	UPROPERTY(VisibleAnywhere)
	TOptional<double> CachedAudioTime = 0;
};

inline FYapBitReplacement::FYapBitReplacement()
{
	CharacterAsset.Reset();

	// TODO investigate this more, why is FText TOptional crashing if I don't do this?
	TitleText = FText::GetEmpty();
	DialogueText = FText::GetEmpty();

	TitleText.Reset();
	DialogueText.Reset();

	DialogueAudioAsset.Reset();
	MoodKey.Reset();
	bUseProjectDefaultTimeSettings.Reset();
	TimeMode.Reset();
	Skippable.Reset();

	ManualTime.Reset();
	CachedWordCount.Reset();
	CachedAudioTime.Reset();
}
