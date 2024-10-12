#pragma once

#include "Yap/FlowYapBit.h"
#include "FlowYapBitReplacement.generated.h"

class UObject;
class UFlowYapCharacter;

USTRUCT()
struct FFlowYapBitReplacement
{
	GENERATED_BODY()

	FFlowYapBitReplacement();
	
	/**  */
	UPROPERTY(EditAnywhere)
	TOptional<TSoftObjectPtr<UFlowYapCharacter>> Character;

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
	TOptional<EFlowYapTimeMode> TimeMode = EFlowYapTimeMode::AudioTime;

	/**  */
	UPROPERTY(EditAnywhere)
	TOptional<EFlowYapInterruptible> Interruptible = EFlowYapInterruptible::UseProjectDefaults;

	// TODO calculate and serialize below on PostEditChangeProperty

	/**  */
	UPROPERTY(VisibleAnywhere)
	TOptional<double> ManualTime = 0;

	/**  */
	UPROPERTY(VisibleAnywhere)
	TOptional<int32> CachedWordCount = 0;

	/**  */
	UPROPERTY(VisibleAnywhere)
	TOptional<double> CachedAudioTime = 0;
};

inline FFlowYapBitReplacement::FFlowYapBitReplacement()
{
	Character.Reset();

	// TODO investigate this more, why is FText TOptional crashing if I don't do all this?
	TitleText = FText::GetEmpty();
	DialogueText = FText::GetEmpty();

	TitleText.Reset();
	DialogueText.Reset();

	DialogueAudioAsset.Reset();
	MoodKey.Reset();
	bUseProjectDefaultTimeSettings.Reset();
	TimeMode.Reset();
	Interruptible.Reset();

	ManualTime.Reset();
	CachedWordCount.Reset();
	CachedAudioTime.Reset();
}
