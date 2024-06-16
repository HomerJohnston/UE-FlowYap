#include "Yap/Nodes/FlowNode_YapSetCondition.h"

#include "Yap/FlowYapProjectSettings.h"
#include "Yap/FlowYapUtil.h"

UFlowNode_YapSetCondition::UFlowNode_YapSetCondition()
{
	NodeStyle = EFlowNodeStyle::Default;

#if WITH_EDITOR
	UFlowYapProjectSettings::RegisterTagFilter(this, GET_MEMBER_NAME_CHECKED(ThisClass, Condition), EFlowYap_TagFilter::Conditions);
#endif
}

FString UFlowNode_YapSetCondition::GetNodeCategory() const
{
	return FlowYapUtil::NodeCategory;
}

FText UFlowNode_YapSetCondition::GetNodeTitle() const
{
	return INVTEXT("Yap Set Condition");
}

FString UFlowNode_YapSetCondition::GetNodeDescription() const
{
	return UFlowYapProjectSettings::GetTrimmedGameplayTagString(EFlowYap_TagFilter::Conditions, Condition);
}
