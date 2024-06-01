#pragma once

#include "Nodes/FlowNode.h"
#include "FlowYap/FlowYapFragment.h"
#include "FlowNode_YapDialogue.generated.h"

class UFlowYapCharacter;

UCLASS(NotBlueprintable)
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
	TArray<FFlowYapFragment> Fragments;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName ConversationName;

#if WITH_EDITORONLY_DATA
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bMultipleInputs;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bMultipleOutputs;
#endif
	// API
public:
	void SetConversationName(FName Name);
	
	FText GetSpeakerName() const;

	FLinearColor GetSpeakerColor() const;

	const UTexture2D* GetDefaultSpeakerPortrait() const;

	const UTexture2D* GetSpeakerPortrait(const FName& RequestedPortraitKey) const;

	FSlateBrush* GetSpeakerPortraitBrush(const FName& RequestedPortraitKey) const;
	
	FText GetNodeTitle() const override;

#if WITH_EDITOR
	FFlowYapFragment& GetFragment(int64 FragmentID);
#endif
	
	TArray<FFlowYapFragment>& GetFragments();

	int16 GetNumFragments() const;
	
	// -------------------

	void InitializeInstance() override;
	
	void OnActivate() override;
	
	void ExecuteInput(const FName& PinName) override;
	
	void AddFragment();

	void RemoveFragment(int64 EditorID);

#if WITH_EDITOR
public:
	bool GetDynamicTitleColor(FLinearColor& OutColor) const override;

	bool CanUserAddInput() const override { return false; }

	bool CanUserAddOutput() const override { return false; }

	bool SupportsContextPins() const override;
	
	void ToggleMultipleInputs();
	
	bool UsesMultipleInputs();
	
	void ToggleMultipleOutputs();
	
	bool UsesMultipleOutputs();
	
	virtual TArray<FFlowPin> GetContextInputs() override;

	virtual TArray<FFlowPin> GetContextOutputs() override;
#endif

	void PostLoad() override;
};