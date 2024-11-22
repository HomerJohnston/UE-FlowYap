
#include "Yap/Nodes/FlowNode_YapUnsetCondition.h"

#include "Yap/YapProjectSettings.h"
#include "Yap/YapUtil.h"

UFlowNode_YapUnsetCondition::UFlowNode_YapUnsetCondition()
{
#if WITH_EDITOR
	NodeStyle = EFlowNodeStyle::Default;
	
	UYapProjectSettings::RegisterTagFilter(this, GET_MEMBER_NAME_CHECKED(ThisClass, Condition), EYap_TagFilter::Conditions);
#endif
}

#if WITH_EDITOR
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
#endif