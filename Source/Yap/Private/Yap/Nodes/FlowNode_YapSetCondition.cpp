#include "Yap/Nodes/FlowNode_YapSetCondition.h"

#include "Yap/YapProjectSettings.h"
#include "Yap/YapUtil.h"

UFlowNode_YapSetCondition::UFlowNode_YapSetCondition()
{
	NodeStyle = EFlowNodeStyle::Default;

#if WITH_EDITOR
	UYapProjectSettings::RegisterTagFilter(this, GET_MEMBER_NAME_CHECKED(ThisClass, Condition), EYap_TagFilter::Conditions);
#endif
}

FString UFlowNode_YapSetCondition::GetNodeCategory() const
{
	return YapUtil::NodeCategory;
}

FText UFlowNode_YapSetCondition::GetNodeTitle() const
{
	return INVTEXT("Yap Set Condition");
}

FString UFlowNode_YapSetCondition::GetNodeDescription() const
{
	return UYapProjectSettings::GetTrimmedGameplayTagString(EYap_TagFilter::Conditions, Condition);
}
