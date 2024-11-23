#include "Yap/Nodes/DEPRECATED_FlowNode_YapConditionGate.h"

#include "Yap/YapProjectSettings.h"
#include "Yap/YapUtil.h"

UDEPRECATED_FlowNode_YapConditionGate::UDEPRECATED_FlowNode_YapConditionGate()
{
#if WITH_EDITORONLY_DATA
	NodeStyle = EFlowNodeStyle::Condition;
#endif
	OutputPins = { FName("No"), FName("Yes") };

#if WITH_EDITOR
	UYapProjectSettings::RegisterTagFilter(this, GET_MEMBER_NAME_CHECKED(ThisClass, Condition), EYap_TagFilter::Conditions);
#endif
}

#if WITH_EDITOR
FString UDEPRECATED_FlowNode_YapConditionGate::GetNodeCategory() const
{
	return YapUtil::NodeCategory;
}

FText UDEPRECATED_FlowNode_YapConditionGate::GetNodeTitle() const
{
	if (IsTemplate())
	{
		return FText::FromString("Branch");
	}

	return INVTEXT("Branch");// Super::GetNodeTitle();
}

FText UDEPRECATED_FlowNode_YapConditionGate::GetNodeToolTip() const
{
	return Super::GetNodeToolTip();
}
#endif

void UDEPRECATED_FlowNode_YapConditionGate::InitializeInstance()
{
	Super::InitializeInstance();
}

void UDEPRECATED_FlowNode_YapConditionGate::OnActivate()
{
	Super::OnActivate();
}

void UDEPRECATED_FlowNode_YapConditionGate::ExecuteInput(const FName& PinName)
{
	Super::ExecuteInput(PinName);

	
}

#if WITH_EDITOR
FString UDEPRECATED_FlowNode_YapConditionGate::GetNodeDescription() const
{
	return UYapProjectSettings::GetTrimmedGameplayTagString(EYap_TagFilter::Conditions, Condition);
}
#endif