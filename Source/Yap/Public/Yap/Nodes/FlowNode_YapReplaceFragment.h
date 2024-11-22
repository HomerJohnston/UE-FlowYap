#pragma once
#include "Nodes/FlowNode.h"
#include "Yap/YapFragment.h"
#include "Yap/YapBitReplacement.h"

#include "FlowNode_YapReplaceFragment.generated.h"

class UFlowNode_YapDialogue;

UCLASS()
class YAP_API UFlowNode_YapReplaceFragment : public UFlowNode
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere)
	FGameplayTag TargetFragmentTag;
	
	UPROPERTY(EditAnywhere)
	FYapBitReplacement NewData;
	
public:
	UFlowNode_YapReplaceFragment();

public:
	virtual void ExecuteInput(const FName& PinName) override;
	
#if WITH_EDITOR
public:
	FString GetNodeCategory() const override;

	void OnGetCategoriesMetaFromPropertyHandle(TSharedPtr<IPropertyHandle> PropertyHandle, FString& String) const;
#endif
};