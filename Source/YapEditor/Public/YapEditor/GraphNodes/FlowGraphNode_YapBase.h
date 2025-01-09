// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once

#include "Graph/Nodes/FlowGraphNode.h"
#include "YapEditor/YapEditorEventBus.h"

#include "FlowGraphNode_YapBase.generated.h"

#define LOCTEXT_NAMESPACE "YapEditor"

UCLASS()
class UFlowGraphNode_YapBase : public UFlowGraphNode
{
	GENERATED_BODY()

public:
	UFlowGraphNode_YapBase();
	
protected:
	FYapEditorEventBus EventBus;

public:
	void Broadcast(FName Event);
};

#undef LOCTEXT_NAMESPACE