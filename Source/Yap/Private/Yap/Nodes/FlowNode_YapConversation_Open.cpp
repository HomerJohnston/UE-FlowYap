// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "Yap/Nodes/FlowNode_YapConversation_Open.h"

#include "FlowAsset.h"
#include "Yap/YapLog.h"
#include "Yap/YapSubsystem.h"
#include "Yap/Nodes/FlowNode_YapConversation_Close.h"
#include "Yap/Nodes/FlowNode_YapDialogue.h"

#define LOCTEXT_NAMESPACE "Yap"

UFlowNode_YapConversation_Open::UFlowNode_YapConversation_Open()
{
#if WITH_EDITOR
	Category = TEXT("Yap");
#endif
}

void UFlowNode_YapConversation_Open::InitializeInstance()
{
	Super::InitializeInstance();

	TArray<UFlowNode*> ConnectedNodes;

	IterateDownstreamNodes(this, ConnectedNodes);
}

void UFlowNode_YapConversation_Open::IterateDownstreamNodes(UFlowNode* DownstreamNode, TArray<UFlowNode*>& ConnectedNodes)
{
	// TODO determine if I need to do this

	/*
	for (UFlowNode* ConnectedNode : DownstreamNode->GatherConnectedNodes())
	{
		if (ConnectedNode && !ConnectedNodes.Contains(ConnectedNode))
		{
			ConnectedNodes.Add(ConnectedNode);

			UFlowNode_YapDialogue* DialogueNode = Cast<UFlowNode_YapDialogue>(ConnectedNode);
			
			if (DialogueNode)
			{
				DialogueNode->SetConversationName(ConversationName);
			}
			
			if (UFlowNode_YapConversationClose* ConversationClose = Cast<UFlowNode_YapConversationClose>(ConnectedNode))
			{
				return;
			}
			
			IterateDownstreamNodes(ConnectedNode, ConnectedNodes);
		}
	}
	*/
}

void UFlowNode_YapConversation_Open::OnActivate()
{
	UE_LOG(LogYap, Verbose, TEXT("Conversation opening: %s"), *Conversation.ToString());

	GetWorld()->GetSubsystem<UYapSubsystem>()->OpenConversation(GetFlowAsset(), Conversation);
}

void UFlowNode_YapConversation_Open::ExecuteInput(const FName& PinName)
{
	Super::ExecuteInput(PinName);

	TriggerFirstOutput(true);
}

#if WITH_EDITOR
FText UFlowNode_YapConversation_Open::GetNodeTitle() const
{
	if (IsTemplate())
	{
		return FText::FromString("Conversation - Open");
	}

	return FText::FromString("Open Convo.");
}

void UFlowNode_YapConversation_Open::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif

#undef LOCTEXT_NAMESPACE