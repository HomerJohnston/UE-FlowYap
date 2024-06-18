#pragma once
#include "Nodes/FlowNode.h"
#include "Yap/FlowYapFragment.h"

#include "FlowNode_YapReplaceFragment.generated.h"

class UFlowNode_YapDialogue;

UCLASS()
class FLOWYAP_API UFlowNode_YapReplaceFragment : public UFlowNode
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
	UFlowNode_YapReplaceFragment();

public:

	FString GetNodeCategory() const override;

#if WITH_EDITOR
	void OnGetCategoriesMetaFromPropertyHandle(TSharedPtr<IPropertyHandle> PropertyHandle, FString& String) const;
#endif
};