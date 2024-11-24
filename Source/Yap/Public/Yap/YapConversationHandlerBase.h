#pragma once

#include "YapConversationHandlerInterface.h"

#include "YapConversationHandlerBase.generated.h"

UCLASS(Abstract, Blueprintable)
class UYapConversationHandlerBase : public UObject, public IYapConversationHandlerInterface
{
	GENERATED_BODY()

public:
	bool ImplementsGetWorld() const override { return true; }
	
public:
	
	void OnConversationStarts_Implementation(const FGameplayTag& Conversation) override;

	void OnConversationEnds_Implementation(const FGameplayTag& Conversation) override;

	void OnDialogueStart_Implementation(const FGameplayTag& Conversation, const FYapBit& DialogueInfo) override;

	void OnDialogueEnd_Implementation(const FGameplayTag& Conversation, const FYapBit& DialogueInfo) override;

	void AddPrompt_Implementation(const FGameplayTag& Conversation, const FYapBit& DialogueInfo, FYapPromptHandle Handle) override;
};