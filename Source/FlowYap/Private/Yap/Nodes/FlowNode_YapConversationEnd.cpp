// Copyright Ghost Pepper Games, Inc. All Rights Reserved.

#include "Yap/Nodes/FlowNode_YapConversationEnd.h"

#include "Yap/FlowYapSubsystem.h"

#define LOCTEXT_NAMESPACE "FlowYap"

UFlowNode_YapConversationEnd::UFlowNode_YapConversationEnd()
{
	Category = TEXT("Yap");
}

void UFlowNode_YapConversationEnd::ExecuteInput(const FName& PinName)
{
	GetWorld()->GetSubsystem<UFlowYapSubsystem>()->EndCurrentConversation();

	TriggerFirstOutput(true);

	FVector MyVec = FVector({1},{2},{3});
}

#undef LOCTEXT_NAMESPACE
