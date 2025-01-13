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
	// TODO should this just be a friend class of my dialogue node class?
	GENERATED_BODY()

public:
	UFlowGraphNode_YapDialogue();
	
public:	
	TSharedPtr<SGraphNode> CreateVisualWidget() override;

	bool ShowPaletteIconOnNode() const override;

	UFlowNode_YapDialogue* GetFlowYapNode() const;

	FLinearColor GetNodeBodyTintColor() const override;

	FSlateIcon GetIconAndTint(FLinearColor& OutColor) const override;

public:
	void PreSave(FObjectPreSaveContext SaveContext) override;
};

#undef LOCTEXT_NAMESPACE