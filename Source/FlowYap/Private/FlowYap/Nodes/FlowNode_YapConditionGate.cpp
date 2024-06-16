#include "FlowYap/Nodes/FlowNode_YapConditionGate.h"

#include "GameplayTagsManager.h"
#include "FlowYap/FlowYapProjectSettings.h"
#include "FlowYap/FlowYapUtil.h"

UFlowNode_YapConditionGate::UFlowNode_YapConditionGate()
{
	NodeStyle = EFlowNodeStyle::Condition;

	OutputPins = { FName("No"), FName("Yes") };

#if WITH_EDITOR
	UFlowYapProjectSettings::RegisterTagFilter(this, GET_MEMBER_NAME_CHECKED(ThisClass, Condition), EFlowYap_TagFilter::Conditions);
#endif
}

FString UFlowNode_YapConditionGate::GetNodeCategory() const
{
	return FlowYapUtil::NodeCategory;
}

FText UFlowNode_YapConditionGate::GetNodeTitle() const
{
	return INVTEXT("Yap Condition");// Super::GetNodeTitle();
}

FText UFlowNode_YapConditionGate::GetNodeToolTip() const
{
	return Super::GetNodeToolTip();
}

void UFlowNode_YapConditionGate::InitializeInstance()
{
	Super::InitializeInstance();
}

void UFlowNode_YapConditionGate::OnActivate()
{
	Super::OnActivate();
}

void UFlowNode_YapConditionGate::ExecuteInput(const FName& PinName)
{
	Super::ExecuteInput(PinName);
}

FString UFlowNode_YapConditionGate::GetNodeDescription() const
{
	return UFlowYapProjectSettings::GetTrimmedGameplayTagString(EFlowYap_TagFilter::Conditions, Condition);
}
