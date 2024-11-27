// Copyright Ghost Pepper Games, Inc. All Rights Reserved.

#include "Yap/Nodes/FlowNode_YapConversationStart.h"

#include "FlowAsset.h"
#include "Yap/YapLog.h"
#include "Yap/YapSubsystem.h"
#include "Yap/Nodes/FlowNode_YapConversationEnd.h"
#include "Yap/Nodes/FlowNode_YapDialogue.h"

#define LOCTEXT_NAMESPACE "FlowYap"

UFlowNode_YapConversationStart::UFlowNode_YapConversationStart()
{
#if WITH_EDITOR
	Category = TEXT("Yap");
#endif
}

void UFlowNode_YapConversationStart::InitializeInstance()
{
	Super::InitializeInstance();

	TArray<UFlowNode*> ConnectedNodes;

	// TODO find conversation ends and make sure no unconnected ends?

	IterateDownstreamNodes(this, ConnectedNodes);
}

void UFlowNode_YapConversationStart::IterateDownstreamNodes(UFlowNode* DownstreamNode, TArray<UFlowNode*>& ConnectedNodes)
{
	for (UFlowNode* ConnectedNode : DownstreamNode->GetConnectedNodes())
	{
		if (ConnectedNode && !ConnectedNodes.Contains(ConnectedNode))
		{
			ConnectedNodes.Add(ConnectedNode);

			UFlowNode_YapDialogue* DialogueNode = Cast<UFlowNode_YapDialogue>(ConnectedNode);
			
			if (DialogueNode)
			{
				//DialogueNode->SetConversationName(ConversationName);
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
	UE_LOG(LogYap, Verbose, TEXT("Conversation starting: %s"), *Conversation.ToString());

	GetWorld()->GetSubsystem<UYapSubsystem>()->StartConversation(GetFlowAsset(), Conversation);
}

void UFlowNode_YapConversationStart::ExecuteInput(const FName& PinName)
{
	Super::ExecuteInput(PinName);

	TriggerFirstOutput(true);
}

FText UFlowNode_YapConversationStart::GetNodeTitle() const
{
	if (IsTemplate())
	{
		return FText::FromString("Conversation Start");
	}

	return FText::FromString("Conv. Start");
}

#if WITH_EDITOR
void UFlowNode_YapConversationStart::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif

#undef LOCTEXT_NAMESPACE