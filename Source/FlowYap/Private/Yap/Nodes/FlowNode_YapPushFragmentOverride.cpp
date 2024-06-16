
#include "Yap/Nodes/FlowNode_YapPushFragmentOverride.h"

#include "GameplayTagsManager.h"
#include "Yap/FlowYapProjectSettings.h"
#include "Yap/FlowYapUtil.h"

UFlowNode_YapPushFragmentOverride::UFlowNode_YapPushFragmentOverride()
{
#if WITH_EDITOR
	UFlowYapProjectSettings::RegisterTagFilter(this, GET_MEMBER_NAME_CHECKED(ThisClass, TargetPrompt), EFlowYap_TagFilter::Prompts);

	if (IsTemplate())
	{
		UGameplayTagsManager::Get().OnGetCategoriesMetaFromPropertyHandle.AddUObject(this, &ThisClass::OnGetCategoriesMetaFromPropertyHandle);
	}
#endif
}

FString UFlowNode_YapPushFragmentOverride::GetNodeCategory() const
{
	return FlowYapUtil::NodeCategory;
}

void UFlowNode_YapPushFragmentOverride::OnGetCategoriesMetaFromPropertyHandle(TSharedPtr<IPropertyHandle> PropertyHandle, FString& MetaString) const
{
	if (!PropertyHandle || PropertyHandle->GetProperty()->GetFName() != GET_MEMBER_NAME_CHECKED(ThisClass, TargetFragment))
	{
		return;
	}

	TArray<UObject*> OuterObjects;
	PropertyHandle->GetOuterObjects(OuterObjects);

	for (const UObject* Object : OuterObjects)
	{
		const UFlowNode_YapPushFragmentOverride* Outer = Cast<UFlowNode_YapPushFragmentOverride>(Object);
		
		if (Outer)
		{
			MetaString = Outer->TargetPrompt.ToString();
			return;
		}
	}
}
