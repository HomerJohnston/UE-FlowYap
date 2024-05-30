#pragma once
#include "FlowYapFragmentSharedSettings.h"

#include "FlowYapFragment.generated.h"

class UAkAudioEvent;

USTRUCT(BlueprintType)
struct FLOWYAP_API FFlowYapFragment
{
	GENERATED_BODY()

#if WITH_EDITOR
	static int64 NextID;
	
	FFlowYapFragment();
#endif
	
protected:
	UPROPERTY(EditAnywhere, meta=(MultiLine=true))
	FText TitleText;
	
	UPROPERTY(EditAnywhere, meta=(MultiLine=true))
	FText DialogueText;

	// TODO soft pointer support for audio
	UPROPERTY(EditAnywhere)
	TObjectPtr<UAkAudioEvent> DialogueAudio;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bUseProjectSettings = true;
	
	UPROPERTY(EditAnywhere, meta = (EditCondition = "!bUseProjectSettings", EditConditionHides, ShowOnlyInnerProperties))
	FFlowYapFragmentSharedSettings SharedSettings;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName PortraitKey = NAME_None;

#if WITH_EDITOR
protected:
	int64 EditorID = -1;

public:
	int64 GetEditorID() const;
#endif
	
public:
	const FText& GetTitleText() const;

	void SetTitleText(FText NewText);

	const FText& GetDialogueText() const;

	void SetDialogueText(FText NewText);
	
	const UAkAudioEvent* GetDialogueAudio() const;

	void SetDialogueAudio(UAkAudioEvent* NewAudio);
	
	bool GetIsTimed() const;
	
	void SetIsTimed(bool bNewValue);

	double GetTimeManualValue() const;

	void SetTimeManualValue(double NewValue);

	bool GetUseAutoTime() const;

	void SetUseAutoTime(bool bNewValue);

	bool GetUseAudioTime() const;

	void SetUseAudioTime(bool bNewValue);

	float GetEndPaddingTime() const;

	void SetEndPaddingTime(float NewValue);

	bool GetUserInterruptible() const;

	void SetUserInterruptible(bool bNewValue);
	
	double GetTimedValue() const;

	void SetPortraitKey(const FName& NewValue);
	
	FName GetPortraitKey() const;

	void SetUseProjectSettings(bool bNewValue);

	bool GetUseProjectSettings() const;
	
#if WITH_EDITOR
public:
	void SetDialogueAudioFromAsset(const FAssetData& AssetData);
	
	bool HasDialogueAudio() const;
#endif
};