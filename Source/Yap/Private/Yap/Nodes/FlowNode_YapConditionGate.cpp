#include "Yap/Nodes/FlowNode_YapConditionGate.h"

#include "Yap/YapProjectSettings.h"
#include "Yap/YapUtil.h"

UFlowNode_YapConditionGate::UFlowNode_YapConditionGate()
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
FString UFlowNode_YapConditionGate::GetNodeCategory() const
{
	return YapUtil::NodeCategory;
}

FText UFlowNode_YapConditionGate::GetNodeTitle() const
{
	return INVTEXT("Yap Condition");// Super::GetNodeTitle();
}

FText UFlowNode_YapConditionGate::GetNodeToolTip() const
{
	return Super::GetNodeToolTip();
}
#endif

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

#if WITH_EDITOR
FString UFlowNode_YapConditionGate::GetNodeDescription() const
{
	return UYapProjectSettings::GetTrimmedGameplayTagString(EYap_TagFilter::Conditions, Condition);
}
#endif