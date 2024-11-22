// Copyright Ghost Pepper Games, Inc. All Rights Reserved.

#include "Yap/Nodes/FlowNode_YapConversationEnd.h"

#include "Yap/YapSubsystem.h"

#define LOCTEXT_NAMESPACE "FlowYap"

UFlowNode_YapConversationEnd::UFlowNode_YapConversationEnd()
{
#if WITH_EDITOR
	Category = TEXT("Yap");
#endif
}

void UFlowNode_YapConversationEnd::ExecuteInput(const FName& PinName)
{
	GetWorld()->GetSubsystem<UYapSubsystem>()->EndCurrentConversation();

	TriggerFirstOutput(true);
}

#undef LOCTEXT_NAMESPACE
