#pragma once
#include "Nodes/FlowNode.h"
#include "Yap/FlowYapFragment.h"
#include "Yap/FlowYapBitReplacement.h"

#include "FlowNode_YapReplaceFragment.generated.h"

class UFlowNode_YapDialogue;

UCLASS()
class FLOWYAP_API UFlowNode_YapReplaceFragment : public UFlowNode
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere)
	FGameplayTag TargetFragmentTag;

	UPROPERTY(EditAnywhere)
	bool bReplacePermanently = true;
	
	UPROPERTY(EditAnywhere)
	FFlowYapBitReplacement NewData;

public:
	UFlowNode_YapReplaceFragment();

public:

	FString GetNodeCategory() const override;

	virtual void ExecuteInput(const FName& PinName) override;
	
#if WITH_EDITOR
	void OnGetCategoriesMetaFromPropertyHandle(TSharedPtr<IPropertyHandle> PropertyHandle, FString& String) const;
#endif
};