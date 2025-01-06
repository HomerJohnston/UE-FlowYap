// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license.

#include "Yap/Nodes/FlowNode_YapUnsetCondition.h"

#include "Yap/YapProjectSettings.h"
#include "Yap/YapUtil.h"

UDEPRECATED_FlowNode_YapUnsetCondition::UDEPRECATED_FlowNode_YapUnsetCondition()
{
#if WITH_EDITOR
	NodeStyle = EFlowNodeStyle::Default;
	
	UYapProjectSettings::RegisterTagFilter(this, GET_MEMBER_NAME_CHECKED(ThisClass, Condition), EYap_TagFilter::Conditions);
#endif
}

#if WITH_EDITOR
FString UDEPRECATED_FlowNode_YapUnsetCondition::GetNodeCategory() const
{
	return YapUtil::NodeCategory;
}

FText UDEPRECATED_FlowNode_YapUnsetCondition::GetNodeTitle() const
{
	return INVTEXT("Yap Unset Condition");
}

FString UDEPRECATED_FlowNode_YapUnsetCondition::GetNodeDescription() const
{
	return UYapProjectSettings::GetTrimmedGameplayTagString(EYap_TagFilter::Conditions, Condition);
}
#endif