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

bool FFlowYapFragment::TryActivate(UFlowNode_YapDialogue* FlowNode_YapDialogue)
{
	if (IsLocalActivationLimitMet() || IsGlobalActivationLimitMet(FlowNode_YapDialogue))
	{
		return false;
	}

	LocalActivationCount++;

	FlowNode_YapDialogue->GetWorld()->GetSubsystem<UFlowYapSubsystem>()->BroadcastFragmentStart(FlowNode_YapDialogue, IndexInDialogue);

	return true;
}

int32 FFlowYapFragment::GetGlobalActivationCount(UFlowNode_YapDialogue* OwnerDialogue) const
{	
	UFlowYapSubsystem* Subsystem = OwnerDialogue->GetWorld()->GetSubsystem<UFlowYapSubsystem>();
	
	return Subsystem->GetGlobalActivationCount(OwnerDialogue, IndexInDialogue);
}

bool FFlowYapFragment::IsGlobalActivationLimitMet(UFlowNode_YapDialogue* OwnerDialogue) const
{
	if (GlobalActivationLimit == 0)
	{
		return false;
	}
	
	return GetGlobalActivationCount(OwnerDialogue) >= GlobalActivationLimit;
}

float FFlowYapFragment::GetPaddingToNextFragment() const
{
	if (CommonPaddingSetting.IsSet())
	{
		return UFlowYapProjectSettings::Get()->GetCommonFragmentPaddings()[CommonPaddingSetting.GetValue()];
	}
	
	return PaddingToNextFragment;
}

bool FFlowYapFragment::IncrementActivations()
{
	bool bResult = true;
	
	if (LocalActivationLimit > 0)
	{
		bResult = LocalActivationCount < LocalActivationLimit;
		LocalActivationCount++;
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