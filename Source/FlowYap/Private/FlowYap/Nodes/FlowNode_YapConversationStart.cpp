// Copyright Ghost Pepper Games, Inc. All Rights Reserved.

#include "FlowYap/Nodes/FlowNode_YapConversationStart.h"

#include "FlowAsset.h"
#include "FlowYap/FlowYapLog.h"
#include "FlowYap/FlowYapSubsystem.h"
#include "FlowYap/Nodes/FlowNode_YapConversationEnd.h"
#include "FlowYap/Nodes/FlowNode_YapDialogue.h"

#define LOCTEXT_NAMESPACE "FlowYap"

UFlowNode_YapConversationStart::UFlowNode_YapConversationStart()
{
	Category = TEXT("Yap");
}

void UFlowNode_YapConversationStart::InitializeInstance()
{
	Super::InitializeInstance();

	TArray<UFlowNode*> ConnectedNodes;

	// TODO find conversation ends and make sure no unconnected ends?

	IterateDownstreamNodes(this, ConnectedNodes);

	for (UFlowNode* DownstreamNode : ConnectedNodes)
	{
		UE_LOG(FlowYap, Warning, TEXT("Conversation start is linked to: %s"), *DownstreamNode->GetNodeTitle().ToString());
	}
}

void UFlowNode_YapConversationStart::IterateDownstreamNodes(UFlowNode* DownstreamNode, TArray<UFlowNode*>& ConnectedNodes)
{
	for (UFlowNode* ConnectedNode : DownstreamNode->GetConnectedNodes())
	{
		if (ConnectedNode)
		{
			ConnectedNodes.Add(ConnectedNode);

			UFlowNode_YapDialogue* DialogueNode = Cast<UFlowNode_YapDialogue>(ConnectedNode);
			
			if (DialogueNode)
			{
				DialogueNode->SetConversationName(ConversationName);
			}
			
			if (UFlowNode_YapConversationEnd* ConversationEnd = Cast<UFlowNode_YapConversationEnd>(ConnectedNode))
			{
				return;
			}
			
			IterateDownstreamNodes(ConnectedNode, ConnectedNodes);
		}
	}
}

void UFlowNode_YapConversationStart::OnActivate()
{
	UE_LOG(FlowYap, Warning, TEXT("Conversation started: %s"), *ConversationName.ToString());

	GetWorld()->GetSubsystem<UFlowYapSubsystem>()->StartConversation(ConversationName);
}

void UFlowNode_YapConversationStart::ExecuteInput(const FName& PinName)
{
	Super::ExecuteInput(PinName);

	TriggerFirstOutput(true);
}

void UFlowNode_YapConversationStart::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	UE_LOG(FlowYap, Warning, TEXT("PECP on start node"));
}

#undef LOCTEXT_NAMESPACE