#pragma once
#include "FlowYap/FlowYapFragment.h"
#include "Nodes/FlowNode.h"

#include "FlowNode_YapPushFragmentOverride.generated.h"

class UFlowNode_YapDialogue;

UCLASS()
class FLOWYAP_API UFlowNode_YapPushFragmentOverride : public UFlowNode
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere)
	FName PromptName;
	
	UPROPERTY(EditAnywhere)
	FName DialogueEntry;

	UPROPERTY(EditAnywhere)
	FGuid DialogueNodeGUID;

	UPROPERTY(EditAnywhere)
	FFlowYapFragment NewFragment;

	UPROPERTY(EditAnywhere)
	FGameplayTag TargetPrompt;

	UPROPERTY(EditAnywhere)
	FGameplayTag TargetFragment;
	
	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<UFlowNode_YapDialogue> DialogueNode = nullptr;

public:
	UFlowNode_YapPushFragmentOverride();

public:

	FString GetNodeCategory() const override;

#if WITH_EDITOR
	void OnGetCategoriesMetaFromPropertyHandle(TSharedPtr<IPropertyHandle> PropertyHandle, FString& String) const;
#endif
};