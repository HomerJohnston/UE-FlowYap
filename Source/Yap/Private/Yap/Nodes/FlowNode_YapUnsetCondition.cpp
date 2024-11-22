
#include "Yap/Nodes/FlowNode_YapUnsetCondition.h"

#include "Yap/YapProjectSettings.h"
#include "Yap/YapUtil.h"

UFlowNode_YapUnsetCondition::UFlowNode_YapUnsetCondition()
{
	NodeStyle = EFlowNodeStyle::Default;
	
#if WITH_EDITOR
	UYapProjectSettings::RegisterTagFilter(this, GET_MEMBER_NAME_CHECKED(ThisClass, Condition), EYap_TagFilter::Conditions);
#endif
}

FString UFlowNode_YapUnsetCondition::GetNodeCategory() const
{
	return YapUtil::NodeCategory;
}

FText UFlowNode_YapUnsetCondition::GetNodeTitle() const
{
	return INVTEXT("Yap Unset Condition");
}

FString UFlowNode_YapUnsetCondition::GetNodeDescription() const
{
	return UYapProjectSettings::GetTrimmedGameplayTagString(EYap_TagFilter::Conditions, Condition);
}
