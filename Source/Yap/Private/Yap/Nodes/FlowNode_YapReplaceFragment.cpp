
#include "Yap/Nodes/FlowNode_YapReplaceFragment.h"

#include "GameplayTagsManager.h"
#include "Yap/YapProjectSettings.h"
#include "Yap/YapSubsystem.h"
#include "Yap/YapUtil.h"

UFlowNode_YapReplaceFragment::UFlowNode_YapReplaceFragment()
{
#if WITH_EDITOR
	UYapProjectSettings::RegisterTagFilter(this, GET_MEMBER_NAME_CHECKED(ThisClass, TargetFragmentTag), EYap_TagFilter::Prompts);

	if (IsTemplate())
	{
		//UGameplayTagsManager::Get().OnGetCategoriesMetaFromPropertyHandle.AddUObject(this, &ThisClass::OnGetCategoriesMetaFromPropertyHandle);
	}
#endif
}

#if WITH_EDITOR
FString UFlowNode_YapReplaceFragment::GetNodeCategory() const
{
	return YapUtil::NodeCategory;
}
#endif

void UFlowNode_YapReplaceFragment::ExecuteInput(const FName& PinName)
{
	Super::ExecuteInput(PinName);

	// TODO is this a bad idea? Can I save the changes to the flow node? Other systems Moth made do it so maybe it's 
	FYapFragment* Fragment = GetWorld()->GetSubsystem<UYapSubsystem>()->FindTaggedFragment(TargetFragmentTag);

	if (Fragment)
	{
		Fragment->ReplaceBit(NewData);

		// TODO should this have settings to control this? Yes probably. There may be times when I want to forcefully flip-flop back and forth.
		// SignalMode = EFlowSignalMode::PassThrough;
	}
	
	TriggerFirstOutput(true);
}

FText UFlowNode_YapReplaceFragment::GetNodeTitle() const
{
	if (IsTemplate())
	{
		return INVTEXT("Replace Fragment");
	}

	return INVTEXT("Replace Fragment");
}

#if WITH_EDITOR
void UFlowNode_YapReplaceFragment::OnGetCategoriesMetaFromPropertyHandle(TSharedPtr<IPropertyHandle> PropertyHandle, FString& MetaString) const
{
	if (!PropertyHandle || PropertyHandle->GetProperty()->GetFName() != GET_MEMBER_NAME_CHECKED(ThisClass, TargetFragmentTag))
	{
		return;
	}

	TArray<UObject*> OuterObjects;
	PropertyHandle->GetOuterObjects(OuterObjects);

	for (const UObject* Object : OuterObjects)
	{
		const UFlowNode_YapReplaceFragment* Outer = Cast<UFlowNode_YapReplaceFragment>(Object);
		
		if (Outer)
		{
			//MetaString = Outer->TargetPrompt.ToString();
			return;
		}
	}
}
#endif