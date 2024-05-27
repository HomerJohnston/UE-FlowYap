#pragma once

#include "FlowNode_YapSpeechBase.h"
#include "FlowYap/FlowYapFragment.h"
#include "FlowNode_YapDialogue.generated.h"

class UFlowYapCharacter;

enum class EFlowNode_YapDialogue_Settings : uint8;

UCLASS(NotBlueprintable, meta = (DisplayName = "Dialogue", Keywords = "event"))
class FLOWYAP_API UFlowNode_YapDialogue : public UFlowNode_YapSpeechBase
{
	GENERATED_BODY()
public:
	UFlowNode_YapDialogue();

protected:
	UPROPERTY(EditAnywhere)
	bool bTimed = true;

	UPROPERTY(EditAnywhere, meta = (EditCondition="bTimed", EditConditionHides))
	bool bUseAudioAssetLength = false;
	
	UPROPERTY(EditAnywhere, meta = (EditCondition="bTimed", ClampMin = 0.0, UIMin = 0.0, UIMax = 30.0))
	double Time = 0.0;
	
	UPROPERTY(EditAnywhere)
	bool bUserInterruptible = true;

public:
	bool GetTimed() const;

	bool GetUseAudioAssetLength() const;

	double GetTime() const;

	bool GetUserInterruptible() const;

public:
	void SetTimed(bool NewValue);

	void SetUseAudioAssetLength(bool NewValue);

	void SetTime(double NewValue);

	void SetUserInterruptible(bool NewValue);
};