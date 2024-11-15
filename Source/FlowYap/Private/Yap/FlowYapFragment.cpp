// Copyright Ghost Pepper Games, Inc. All Rights Reserved.

#include "Yap/FlowYapFragment.h"

#include "GameplayTagsManager.h"
#include "Yap/FlowYapCondition.h"
#include "Yap/FlowYapProjectSettings.h"
#include "Yap/FlowYapSubsystem.h"

#include "Yap/Nodes/FlowNode_YapDialogue.h"

#define LOCTEXT_NAMESPACE "FlowYap"

#undef LOCTEXT_NAMESPACE
FFlowYapFragment::FFlowYapFragment()
{
#if WITH_EDITOR
	//FragmentTagFilterDelegateHandle = UGameplayTagsManager::Get().OnGetCategoriesMetaFromPropertyHandle.AddStatic(&FFlowYapFragment::OnGetCategoriesMetaFromPropertyHandle);
#endif
	
	Guid = FGuid::NewGuid();
}

FFlowYapFragment::~FFlowYapFragment()
{
}

bool FFlowYapFragment::CheckConditions() const
{
	for (TObjectPtr<UFlowYapCondition> Condition : Conditions)
	{
		if (!IsValid(Condition))
		{
			UE_LOG(FlowYap, Warning, TEXT("Ignoring null condition. Clean this up!")); // TODO more info
			continue;
		}
		if (!Condition->Evaluate())
		{
			return false;
		}
	}
	
	return true;
}

void FFlowYapFragment::ResetOptionalPins()
{
	bShowOnStartPin = false;
	bShowOnEndPin = false;
}

float FFlowYapFragment::GetPaddingToNextFragment() const
{
	if (CommonPaddingSetting.IsSet())
	{
		return UFlowYapProjectSettings::Get()->GetCommonFragmentPaddings()[CommonPaddingSetting.GetValue()];
	}
	
	return PaddingToNextFragment;
}

void FFlowYapFragment::IncrementActivations()
{
	ActivationCount++;
}

void FFlowYapFragment::ReplaceBit(const FFlowYapBitReplacement& ReplacementBit)
{
	Bit = ReplacementBit;

	bBitReplaced = true;
}

FName FFlowYapFragment::GetStartPinName()
{
	if (GetShowOnStartPin())
	{
		// TODO cache this at editor time
		return FName("FragmentStart_" + GetGuid().ToString());
	}
	else
	{
		return NAME_None;
	}
}

FName FFlowYapFragment::GetEndPinName()
{
	if (GetShowOnEndPin())
 	{
 		// TODO cache this at editor time
 		return FName("FragmentEnd_" + GetGuid().ToString());
 	}
 	else
 	{
 		return NAME_None;
 	}
}

#if WITH_EDITOR
void FFlowYapFragment::OnGetCategoriesMetaFromPropertyHandle(TSharedPtr<IPropertyHandle> PropertyHandle, FString& MetaString)
{
	if (!PropertyHandle || PropertyHandle->GetProperty()->GetFName() != GET_MEMBER_NAME_CHECKED(FFlowYapFragment, FragmentTag))
	{
		return;
	}

	TArray<UObject*> OuterObjects;
	PropertyHandle->GetOuterObjects(OuterObjects);

	for (const UObject* Object : OuterObjects)
	{
		const UFlowNode_YapDialogue* DialogueNode = Cast<UFlowNode_YapDialogue>(Object);

		if (!DialogueNode)
		{
			continue;
		}
		
		if (DialogueNode->GetIsPlayerPrompt())
		{
			MetaString = DialogueNode->GetDialogueTag().ToString();
		}
	}
}

bool FFlowYapFragment::GetBitReplaced() const
{
	return bBitReplaced;
}

TArray<FFlowPin> FFlowYapFragment::GetOutputPins() const
{
	TArray<FFlowPin> Pins;
	
	if (Owner.IsValid() && Owner->GetIsPlayerPrompt())
	{
		Pins.Add(GetPromptPin());
	}
	
	if (GetShowOnEndPin())
	{
		Pins.Add(GetEndPin());
	}
	
	if (GetShowOnStartPin())
	{
		Pins.Add(GetStartPin());
	}
	
	return Pins;
}

FFlowPin FFlowYapFragment::GetPromptPin() const
{
	FFlowPin PromptPin = FName("Prompt_" + GetGuid().ToString());
	PromptPin.PinToolTip = "Out";
	return PromptPin;
}

FFlowPin FFlowYapFragment::GetEndPin() const
{
	FFlowPin EndPin = FName("End_" + GetGuid().ToString());
	EndPin.PinToolTip = "Runs before any end-padding time begins";
	return EndPin;
}

FFlowPin FFlowYapFragment::GetStartPin() const
{
	FFlowPin StartPin = FName("Start_" + GetGuid().ToString());
	StartPin.PinToolTip = "Runs when the fragment starts playback";
	return StartPin;
}

void FFlowYapFragment::InvalidateFragmentTag()
{
	FragmentTag = FGameplayTag::EmptyTag;
}

#endif
