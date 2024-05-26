#pragma once

#include "Nodes/FlowNode.h"
#include "FlowYap/FlowYapCharacterMood.h"
#include "FlowYap/FlowYapFragment.h"
#include "FlowNode_YapDialogue.generated.h"

class UFlowYapCharacter;

UCLASS(NotBlueprintable, meta = (DisplayName = "Dialogue", Keywords = "event"))
class FLOWYAP_API UFlowNode_YapDialogue : public UFlowNode
{
	GENERATED_BODY()
public:
	UFlowNode_YapDialogue();

	// SETTINGS
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UFlowYapCharacter> Character;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FFlowYapFragment Fragment;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName PortraitKey;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	
	
	// API
public:
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
	
#if WITH_EDITOR
	void SetDialogueText(const FText& CommittedText);

	void SetTitleText(const FText& CommittedText);

	//void UpdateColor();

	bool GetDynamicTitleColor(FLinearColor& OutColor) const override;
	
	bool CanUserAddOutput() const override { return true; }
	
	FSlateBrush* GetSpeakerPortraitBrush(const FName& RequestedPortraitKey) const;
	
	void SetDialogueAudioAsset(const FAssetData& AssetData);
#endif
};