#pragma once
#include "Nodes/FlowNode.h"

#include "DEPRECATED_FlowNode_YapConditionGate.generated.h"

// This should get replaced by a separate "facts" plugin
UCLASS(meta = (DisplayName = "Condition Gate"), Deprecated)
class YAP_API UDEPRECATED_FlowNode_YapConditionGate : public UFlowNode
{
	GENERATED_BODY()

public:
	UDEPRECATED_FlowNode_YapConditionGate();

public:
	UPROPERTY(EditAnywhere)
	FString TestText;

	UPROPERTY(EditAnywhere)
	FGameplayTag Condition;

#if WITH_EDITOR
public:
	virtual FString GetNodeCategory() const;
	virtual FText GetNodeTitle() const;
	virtual FText GetNodeToolTip() const;
#endif
	
public:
	void InitializeInstance() override;

	void OnActivate() override;

	void ExecuteInput(const FName& PinName) override;

#if WITH_EDITOR
	FString GetNodeDescription() const override;
#endif
};

