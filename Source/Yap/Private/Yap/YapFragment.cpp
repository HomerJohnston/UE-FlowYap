// Copyright Ghost Pepper Games, Inc. All Rights Reserved.

#include "Yap/YapFragment.h"

#include "GameplayTagsManager.h"
#include "Yap/YapCondition.h"
#include "Yap/YapProjectSettings.h"
#include "Yap/YapSubsystem.h"

#include "Yap/Nodes/FlowNode_YapDialogue.h"

#define LOCTEXT_NAMESPACE "FlowYap"

#undef LOCTEXT_NAMESPACE
FYapFragment::FYapFragment()
{
#if WITH_EDITOR
	//FragmentTagFilterDelegateHandle = UGameplayTagsManager::Get().OnGetCategoriesMetaFromPropertyHandle.AddStatic(&FFlowYapFragment::OnGetCategoriesMetaFromPropertyHandle);
#endif
	Guid = FGuid::NewGuid();
}

FYapFragment::~FYapFragment()
{
}

bool FYapFragment::CheckConditions() const
{
	for (TObjectPtr<UYapCondition> Condition : Conditions)
	{
		if (!IsValid(Condition))
		{
			UE_LOG(LogYap, Warning, TEXT("Ignoring null condition. Clean this up!")); // TODO more info
			continue;
		}
		if (!Condition->EvaluateCondition())
		{
			return false;
		}
	}
	
	return true;
}

void FYapFragment::ResetOptionalPins()
{
	bShowOnStartPin = false;
	bShowOnEndPin = false;
}

float FYapFragment::GetPaddingToNextFragment() const
{
	if (PaddingToNextFragment < 0 && UYapProjectSettings::Get()->IsDefaultFragmentPaddingTimeEnabled())
	{
		return UYapProjectSettings::Get()->GetDefaultFragmentPaddingTime();
	}
	
	return FMath::Max(PaddingToNextFragment, 0);
}

void FYapFragment::IncrementActivations()
{
	ActivationCount++;
}

void FYapFragment::ReplaceBit(const FYapBitReplacement& ReplacementBit)
{
	Bit = ReplacementBit;
}

#if WITH_EDITOR
void FYapFragment::OnGetCategoriesMetaFromPropertyHandle(TSharedPtr<IPropertyHandle> PropertyHandle, FString& MetaString)
{
	if (!PropertyHandle || PropertyHandle->GetProperty()->GetFName() != GET_MEMBER_NAME_CHECKED(FYapFragment, FragmentTag))
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

TArray<FFlowPin> FYapFragment::GetOutputPins() const
{
	TArray<FFlowPin> Pins;
	
	if (Owner.IsValid() && Owner->GetIsPlayerPrompt())
	{
		Pins.Add(GetPromptPin());
	}
	
	if (UsesEndPin())
	{
		Pins.Add(GetEndPin());
	}
	
	if (UsesStartPin())
	{
		Pins.Add(GetStartPin());
	}
	
	return Pins;
}

FFlowPin FYapFragment::GetPromptPin() const
{
	FFlowPin PromptPin = FName("Prompt_" + GetGuid().ToString());
	PromptPin.PinToolTip = "Out";
	return PromptPin;
}

FFlowPin FYapFragment::GetEndPin() const
{
	FFlowPin EndPin = FName("End_" + GetGuid().ToString());
	EndPin.PinToolTip = "Runs before any end-padding time begins";
	return EndPin;
}

FFlowPin FYapFragment::GetStartPin() const
{
	FFlowPin StartPin = FName("Start_" + GetGuid().ToString());
	StartPin.PinToolTip = "Runs when the fragment starts playback";
	return StartPin;
}

void FYapFragment::InvalidateFragmentTag()
{
	FragmentTag = FGameplayTag::EmptyTag;
}

#endif
