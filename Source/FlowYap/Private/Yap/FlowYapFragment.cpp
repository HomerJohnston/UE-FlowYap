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

int32 FFlowYapFragment::GetGlobalActivationCount(UFlowNode_YapDialogue* Dialogue) const
{
	// Safety check because the widgets call into this, and it will crash if the widget is calling this without an actual preview entity selected
	UWorld* World = Dialogue->GetWorld();

#if WITH_EDITOR
	if (!World && GEditor->PlayWorld)
	{
		World = GEditor->PlayWorld;
	}
#endif
	
	if (!World || World->WorldType != EWorldType::Game && World->WorldType != EWorldType::PIE && World->WorldType != EWorldType::GamePreview)
	{
		return 0;
	}
	
	UFlowYapSubsystem* Subsystem = World->GetSubsystem<UFlowYapSubsystem>();
	
	return Subsystem->GetGlobalActivationCount(Dialogue, IndexInDialogue);
}

bool FFlowYapFragment::IsGlobalActivationLimitMet(UFlowNode_YapDialogue* Dialogue) const
{
	if (GlobalActivationLimit == 0)
	{
		return false;
	}
	
	return GetGlobalActivationCount(Dialogue) >= GlobalActivationLimit;
}

bool FFlowYapFragment::IsActivationLimitMet(UFlowNode_YapDialogue* Dialogue) const
{
	return IsLocalActivationLimitMet() || IsGlobalActivationLimitMet(Dialogue);
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
	LocalActivationCount++;
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