#pragma once
#include "FlowYapAssetWrapper.h"
#include "FlowYapFragmentTimeSettings.h"

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

	// Settings
protected:
	UPROPERTY(EditAnywhere, meta=(MultiLine=true))
	FText TitleText;
	
	UPROPERTY(EditAnywhere, meta=(MultiLine=true))
	FText DialogueText;

	// TODO soft pointer support for audio!
	UPROPERTY(EditAnywhere)
	TObjectPtr<UObject> DialogueAudio;

	UPROPERTY(EditAnywhere)
	bool bUseProjectDefaultTimeSettings = true;
	
	UPROPERTY(EditAnywhere, meta = (EditCondition = "!bUseProjectDefaultTimeSettings", ShowOnlyInnerProperties))
	FFlowYapFragmentTimeSettings TimeSettings;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName PortraitKey = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 ActivationLimit = 0;
	
	// State
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly) // TODO VisibleAnywhere
	int32 ActivationCount = 0;
	
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

	template<class T>
	const T* GetDialogueAsset() const { return Cast<T>(GetDialogueAsset()); };
	
	const UObject* GetDialogueAsset() const;
	void SetDialogueAudio(UObject* NewAudio);

	bool GetUsesProjectDefaultTimeSettings() const;
	void SetUseProjectDefaultTimeSettings(bool NewValue);

	//	===============================================================================================
#pragma region Timed Settings API group
	//	-----------------------------------------------------------------------------------------------
	const FFlowYapFragmentTimeSettings& GetTimeSettings() const;
	
	EFlowYapTimedMode GetTimedMode() const;

	void SetTimedMode(EFlowYapTimedMode NewValue);
	void UnsetTimedMode();
	
	double GetEnteredTimeValue() const;
	void SetEnteredTimeValue(double NewValue);

	float GetEndPaddingTime() const;
	void SetEndPaddingTime(float NewValue);

	bool GetUserInterruptible() const;
	void SetUserInterruptible(bool bNewValue);

	EFlowYapTimedMode GetRuntimeTimedMode() const;
	double GetCalculatedTimedValue() const;

	const FFlowYapFragmentTimeSettings& GetRuntimeTimeSettings() const;
	double CalculateTimeFromText() const;
	double CalculateTimeFromAudio() const;
	//	-----------------------------------------------------------------------------------------------
#pragma endregion 
	//	-----------------------------------------------------------------------------------------------

	void SetPortraitKey(const FName& NewValue);
	
	FName GetPortraitKey() const;

	int32 GetActivationCount() const;
	
	int32 GetActivationLimit() const;
	
#if WITH_EDITOR
public:
	void SetDialogueAudioFromAsset(const FAssetData& AssetData);
	
	bool HasDialogueAudioAsset() const;

	void SetActivationLimit(int32 NewValue);
#endif
};