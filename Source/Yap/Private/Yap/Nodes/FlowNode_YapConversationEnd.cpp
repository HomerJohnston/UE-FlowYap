// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "Yap/Nodes/FlowNode_YapConversationEnd.h"

#include "Yap/YapSubsystem.h"

#define LOCTEXT_NAMESPACE "Yap"

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

#if WITH_EDITOR
FText UFlowNode_YapConversationEnd::GetNodeTitle() const
{
	if (IsTemplate())
	{
		return FText::FromString("Conversation End");
	}

	return FText::FromString("Conv. End");
}
#endif

#undef LOCTEXT_NAMESPACE
