#include "FlowYap/Nodes/FlowNode_YapUnsetCondition.h"

#include "GameplayTagsManager.h"
#include "FlowYap/FlowYapProjectSettings.h"
#include "FlowYap/Nodes/Category.h"

UFlowNode_YapUnsetCondition::UFlowNode_YapUnsetCondition()
{
	NodeStyle = EFlowNodeStyle::Default;
	
#if WITH_EDITOR
	UFlowYapProjectSettings::Get()->RegisterConditionContainerUser(this, GET_MEMBER_NAME_CHECKED(ThisClass, Condition));
#endif
}

FString UFlowNode_YapUnsetCondition::GetNodeCategory() const
{
	return FlowYapRuntime::NodeCategory;
}

FText UFlowNode_YapUnsetCondition::GetNodeTitle() const
{
	return INVTEXT("Yap Unset Condition");
}

FString UFlowNode_YapUnsetCondition::GetNodeDescription() const
{
	const FGameplayTag& ParentContainer = UFlowYapProjectSettings::Get()->GetConditionContainer();

	if (ParentContainer.IsValid() && ParentContainer != FGameplayTag::EmptyTag && Condition.MatchesTag(ParentContainer))
	{
		return Condition.ToString().RightChop(ParentContainer.ToString().Len() + 1);
	}
	
	return Condition.ToString();
}
