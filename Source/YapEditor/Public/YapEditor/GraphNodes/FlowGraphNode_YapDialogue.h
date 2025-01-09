// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license.

#pragma once

#include "FlowGraphNode_YapBase.h"

#include "FlowGraphNode_YapDialogue.generated.h"

class UFlowNode_YapDialogue;
class UAkAudioEvent;
class SGraphNode;
struct FYapFragment;

#define LOCTEXT_NAMESPACE "YapEditor"

UCLASS()
class UFlowGraphNode_YapDialogue : public UFlowGraphNode_YapBase
{
	GENERATED_BODY()

public:
	UFlowGraphNode_YapDialogue();

protected:
	// Fragment GUID, array of pin names for this fragment - basically only used for fragment deletion
	TMap<FGuid, TArray<FName>> FragmentPins;
	
public:	
	TSharedPtr<SGraphNode> CreateVisualWidget() override;

	bool ShowPaletteIconOnNode() const override;

	UFlowNode_YapDialogue* GetFlowYapNode() const;

	FLinearColor GetNodeBodyTintColor() const override;

	FSlateIcon GetIconAndTint(FLinearColor& OutColor) const override;
};

#undef LOCTEXT_NAMESPACE