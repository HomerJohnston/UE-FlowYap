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
};