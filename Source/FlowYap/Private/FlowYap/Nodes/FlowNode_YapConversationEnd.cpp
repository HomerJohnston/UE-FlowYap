// Copyright Ghost Pepper Games, Inc. All Rights Reserved.

#include "FlowYap/Nodes/FlowNode_YapConversationEnd.h"

#include "FlowYap/FlowYapSubsystem.h"

#define LOCTEXT_NAMESPACE "FlowYap"

UFlowNode_YapConversationEnd::UFlowNode_YapConversationEnd()
{
	Category = TEXT("Yap");
}

void UFlowNode_YapConversationEnd::ExecuteInput(const FName& PinName)
{
	GetWorld()->GetSubsystem<UFlowYapSubsystem>()->EndCurrentConversation();

	TriggerFirstOutput(true);
}

#undef LOCTEXT_NAMESPACE
