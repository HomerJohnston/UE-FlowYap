#include "FlowYap/Nodes/FlowNode_YapConditionGate.h"

#include "GameplayTagsManager.h"
#include "FlowYap/FlowYapProjectSettings.h"
#include "FlowYap/Nodes/Category.h"

UFlowNode_YapConditionGate::UFlowNode_YapConditionGate()
{
	NodeStyle = EFlowNodeStyle::Condition;

	OutputPins = { FName("No"), FName("Yes") };

#if WITH_EDITOR
	UFlowYapProjectSettings::Get()->RegisterConditionContainerUser(this, GET_MEMBER_NAME_CHECKED(ThisClass, Condition));
#endif
}

FString UFlowNode_YapConditionGate::GetNodeCategory() const
{
	return FlowYapRuntime::NodeCategory;
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
	const FGameplayTag& ParentContainer = UFlowYapProjectSettings::Get()->GetConditionContainer();

	if (ParentContainer.IsValid() && ParentContainer != FGameplayTag::EmptyTag && Condition.MatchesTag(ParentContainer))
	{
		return Condition.ToString().RightChop(ParentContainer.ToString().Len() + 1) + "?";
	}
	
	return Condition.ToString() + "?";
}
