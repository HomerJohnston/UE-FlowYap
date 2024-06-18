// Copyright Ghost Pepper Games, Inc. All Rights Reserved.

#include "Yap/FlowYapFragment.h"

#include "GameplayTagsManager.h"

#include "Yap/Nodes/FlowNode_YapDialogue.h"

#define LOCTEXT_NAMESPACE "FlowYap"

#undef LOCTEXT_NAMESPACE
FFlowYapFragment::FFlowYapFragment()
{
	FragmentTagFilterDelegateHandle = UGameplayTagsManager::Get().OnGetCategoriesMetaFromPropertyHandle.AddRaw(this, &FFlowYapFragment::OnGetCategoriesMetaFromPropertyHandle); // TODO is this safe!?
}

FFlowYapFragment::~FFlowYapFragment()
{
}

bool FFlowYapFragment::TryActivate()
{
	bool bResult = true;
	
	if (ActivationLimit > 0)
	{
		bResult = ActivationCount < ActivationLimit;
		ActivationCount++;
	}

	return bResult;
}

#if WITH_EDITOR
void FFlowYapFragment::OnGetCategoriesMetaFromPropertyHandle(TSharedPtr<IPropertyHandle> PropertyHandle, FString& MetaString) const
{
	if (!PropertyHandle || PropertyHandle->GetProperty()->GetFName() != GET_MEMBER_NAME_CHECKED(FFlowYapFragment, FragmentTag))
	{
		return;
	}

	TArray<UObject*> OuterObjects;
	PropertyHandle->GetOuterObjects(OuterObjects);

	for (const UObject* Object : OuterObjects)
	{
		const UFlowNode_YapDialogue* DialogueNode = Cast<UFlowNode_YapDialogue>(Object);

		if (!DialogueNode)
		{
			continue;
		}
		
		if (DialogueNode->GetIsPlayerPrompt())
		{
			MetaString = DialogueNode->GetDialogueTag().ToString();
		}
	}
}

#endif