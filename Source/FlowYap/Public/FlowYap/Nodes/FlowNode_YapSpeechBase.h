#pragma once

#include "Nodes/FlowNode.h"
#include "FlowYap/FlowYapFragment.h"
#include "FlowNode_YapSpeechBase.generated.h"

class UFlowYapCharacter;

UCLASS(Abstract, NotBlueprintable)
class FLOWYAP_API UFlowNode_YapSpeechBase : public UFlowNode
{
	GENERATED_BODY()
public:
	UFlowNode_YapSpeechBase();

	// SETTINGS
protected:	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UFlowYapCharacter> Character;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FFlowYapFragment Fragment;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName PortraitKey;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bUseProjectSettings;

protected:
	UPROPERTY(EditAnywhere)
	bool bTimed = true;

	UPROPERTY(EditAnywhere, meta = (EditCondition="bTimed", ClampMin = 0.0, UIMin = 0.0, UIMax = 30.0))
	double Time = 0.0;
	
	UPROPERTY(EditAnywhere, meta = (EditCondition="bTimed", EditConditionHides))
	bool bUseAutoTime = true;
	
	UPROPERTY(EditAnywhere, meta = (EditCondition="bTimed", EditConditionHides))
	bool bUseAudioAssetLength = false;
	
	/** After each dialogue is finished being spoken, a brief extra pause can be inserted before moving onto the next node. */
	UPROPERTY(EditAnywhere)
	float EndPaddingTime;
	
	UPROPERTY(EditAnywhere)
	bool bUserInterruptible = true;
	
	// STATE
protected:
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
	FName ConversationName;
	
	// API
public:
	void SetConversationName(FName Name);
	
	FText GetSpeakerName() const;

	FLinearColor GetSpeakerColor() const;

	const UTexture2D* GetDefaultSpeakerPortrait() const;

	const UTexture2D* GetSpeakerPortrait(const FName& RequestedPortraitKey) const;

	const FFlowYapFragment& GetFragment() const;

	FText GetTitleText() const;

	FText GetDialogueText() const;

	// TODO soft pointer support for audio
	UAkAudioEvent* GetDialogueAudio() const;
	
	FText GetNodeTitle() const override;
	
	void SetPortraitKey(const FName& NewValue);
	
	FName GetPortraitKey() const;

	// -------------------

	void InitializeInstance() override;
	
	void OnActivate() override;
	
	void ExecuteInput(const FName& PinName) override;

	
#if WITH_EDITOR
	void SetDialogueText(const FText& CommittedText);

	void SetTitleText(const FText& CommittedText);

	bool GetDynamicTitleColor(FLinearColor& OutColor) const override;
	
	bool CanUserAddOutput() const override { return true; }
	
	FSlateBrush* GetSpeakerPortraitBrush(const FName& RequestedPortraitKey) const;
	
	void SetDialogueAudioAsset(const FAssetData& AssetData);
#endif
	
public:
	bool GetTimed() const;

	bool GetUseAutoTime() const;
	
	bool GetUseAudioAssetLength() const;

	double GetTime() const;

	bool GetUserInterruptible() const;

public:
	void SetTimed(bool NewValue);

	void SetUseAutoTime(bool NewValue);
	
	void SetUseAudioAssetLength(bool NewValue);

	void SetTime(double NewValue);

	void SetUserInterruptible(bool NewValue);
};