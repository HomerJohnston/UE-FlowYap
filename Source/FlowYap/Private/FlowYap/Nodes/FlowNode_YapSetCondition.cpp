#include "FlowYap/Nodes/FlowNode_YapSetCondition.h"

#include "GameplayTagsManager.h"
#include "FlowYap/FlowYapProjectSettings.h"
#include "FlowYap/Nodes/Category.h"

UFlowNode_YapSetCondition::UFlowNode_YapSetCondition()
{
	NodeStyle = EFlowNodeStyle::Default;

#if WITH_EDITOR
	UFlowYapProjectSettings::Get()->RegisterConditionContainerUser(this, GET_MEMBER_NAME_CHECKED(ThisClass, Condition));
#endif
}

FString UFlowNode_YapSetCondition::GetNodeCategory() const
{
	return FlowYapRuntime::NodeCategory;
}

FText UFlowNode_YapSetCondition::GetNodeTitle() const
{
	return INVTEXT("Yap Set Condition");
}

FString UFlowNode_YapSetCondition::GetNodeDescription() const
{
	const FGameplayTag& ParentContainer = UFlowYapProjectSettings::Get()->GetConditionContainer();

	if (ParentContainer.IsValid() && ParentContainer != FGameplayTag::EmptyTag && Condition.MatchesTag(ParentContainer))
	{
		return Condition.ToString().RightChop(ParentContainer.ToString().Len() + 1);
	}
	
	return Condition.ToString();
}
