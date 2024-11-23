#include "Yap/Nodes/DEPRECATED_FlowNode_YapSetCondition.h"

#include "Yap/YapProjectSettings.h"
#include "Yap/YapUtil.h"

UDEPRECATED_FlowNode_YapSetCondition::UDEPRECATED_FlowNode_YapSetCondition()
{
#if WITH_EDITOR
	NodeStyle = EFlowNodeStyle::Default;

	UYapProjectSettings::RegisterTagFilter(this, GET_MEMBER_NAME_CHECKED(ThisClass, Condition), EYap_TagFilter::Conditions);
#endif
}

#if WITH_EDITOR
FString UDEPRECATED_FlowNode_YapSetCondition::GetNodeCategory() const
{
	return YapUtil::NodeCategory;
}

FText UDEPRECATED_FlowNode_YapSetCondition::GetNodeTitle() const
{
	return INVTEXT("Yap Set Condition");
}

FString UDEPRECATED_FlowNode_YapSetCondition::GetNodeDescription() const
{
	return UYapProjectSettings::GetTrimmedGameplayTagString(EYap_TagFilter::Conditions, Condition);
}
#endif