// Copyright Ghost Pepper Games, Inc. All Rights Reserved.

#include "Yap/FlowYapFragment.h"

#include "GameplayTagsManager.h"
#include "Yap/FlowYapProjectSettings.h"
#include "Yap/FlowYapSubsystem.h"

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

float FFlowYapFragment::GetPaddingToNextFragment() const
{
	if (CommonPaddingSetting.IsSet())
	{
		return UFlowYapProjectSettings::Get()->GetCommonFragmentPaddings()[CommonPaddingSetting.GetValue()];
	}
	
	return PaddingToNextFragment;
}

void FFlowYapFragment::IncrementActivations()
{
	ActivationCount++;
}

void FFlowYapFragment::ReplaceBit(const FFlowYapBitReplacement& ReplacementBit)
{
	Bit = ReplacementBit;

	bBitReplaced = true;
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

bool FFlowYapFragment::GetBitReplaced() const
{
	return bBitReplaced;
}

#endif
