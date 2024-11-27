#pragma once

#include "YapConversationListenerInterface.h"

#include "YapConversationBrokerBase.generated.h"

/** Optional base class for brokering Yap to your game. Create a child class of this and override the functions to create conversation panels and/or display floating text widgets in your game. Then set Yap's project settings to use your class. */
UCLASS(Abstract, Blueprintable)
class UYapConversationBrokerBase : public UObject, public IYapConversationListenerInterface
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