#pragma once
#include "Nodes/FlowNode.h"

#include "FlowNode_YapConditionGate.generated.h"

UCLASS()
class YAP_API UFlowNode_YapConditionGate : public UFlowNode
{
	GENERATED_BODY()

public:
	UFlowNode_YapConditionGate();

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

