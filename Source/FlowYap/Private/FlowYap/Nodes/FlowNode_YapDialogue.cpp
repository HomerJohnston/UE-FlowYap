// Copyright Ghost Pepper Games, Inc. All Rights Reserved.

#include "FlowYap/Nodes/FlowNode_YapDialogue.h"

#include "FlowYap/FlowYapCharacter.h"
#include "FlowYap/FlowYapLog.h"
#include "FlowYap/FlowYapProjectSettings.h"

#define LOCTEXT_NAMESPACE "FlowYap"

UFlowNode_YapDialogue::UFlowNode_YapDialogue()
{
	Category = TEXT("Yap");
	NodeStyle = EFlowNodeStyle::Custom;

	bIsPlayerPrompt = false;
	NodeActivationLimit = 0;
	MultipleInputBehavior = EFlowYapMultipleInputBehavior::Sequential;

	// Always have at least one fragment.
	Fragments.Add(FFlowYapFragment());

	InputPins.Empty();
	OutputPins.Empty();

	// Workaround to "save" pins - putting them in the CDO causes some nuisance engine code to work correctly
	for (int Index = 0; Index <= 9; Index++)
	{
		InputPins.Add(Index);
		OutputPins.Add(Index);
	}
}

void UFlowNode_YapDialogue::SetConversationName(FName Name)
{
	if (ConversationName != NAME_None)
	{
		return;
	}

	ConversationName = Name;
}

FText UFlowNode_YapDialogue::GetSpeakerName() const
{
	if (!Character)
	{
		return LOCTEXT("DialogueNodeMissingCharacter", "NO CHARACTER SET");
	}

	return Character->GetEntityName();
}

FLinearColor UFlowNode_YapDialogue::GetSpeakerColor() const
{
	if (!Character)
	{
		return FLinearColor::Gray;
	}
	
	return Character->GetEntityColor();
}

const UTexture2D* UFlowNode_YapDialogue::GetDefaultSpeakerPortrait() const
{
	if (!Character)
	{
		return nullptr;
	}

	const UFlowYapProjectSettings* Settings = GetDefault<UFlowYapProjectSettings>();

	if (Settings->GetPortraitKeys().Num() == 0)
	{
		return nullptr;
	}

	const FName& Key = Settings->GetPortraitKeys()[0];
	
	return GetSpeakerPortrait(Key);
}

const UTexture2D* UFlowNode_YapDialogue::GetSpeakerPortrait(const FName& RequestedPortraitKey) const
{
	if (!Character)
	{
		return nullptr;
	}

	const TObjectPtr<UTexture2D>* Portrait = Character->GetPortraits().Find(RequestedPortraitKey);

	if (Portrait)
	{
		return *Portrait;
	}
	else
	{
		return nullptr;
	}
}

#if WITH_EDITOR
FFlowYapFragment& UFlowNode_YapDialogue::GetFragmentByID(int64 FragmentID)
{
	FFlowYapFragment* FoundFragment = Fragments.FindByPredicate(
		[&](FFlowYapFragment& Fragment)
	{
		return Fragment.GetEditorID() == FragmentID;
	});

	check(FoundFragment);

	return *FoundFragment;
}
#endif

TArray<FFlowYapFragment>& UFlowNode_YapDialogue::GetFragments()
{
	return Fragments;
}

int16 UFlowNode_YapDialogue::GetNumFragments() const
{
	return Fragments.Num();
}

FText UFlowNode_YapDialogue::GetNodeTitle() const
{
	if (!Character)
	{
		return Super::GetNodeTitle();
	}

	return GetSpeakerName();
}

bool UFlowNode_YapDialogue::GetIsPlayerPrompt() const
{
	return bIsPlayerPrompt;
}

int32 UFlowNode_YapDialogue::GetNodeActivationCount() const
{
	return NodeActivationCount;
}

int32 UFlowNode_YapDialogue::GetNodeActivationLimit() const
{
	return NodeActivationLimit;
}

void UFlowNode_YapDialogue::InitializeInstance()
{
	UE_LOG(FlowYap, Warning, TEXT("InitializeInstance"));

	Super::InitializeInstance();
}

void UFlowNode_YapDialogue::OnActivate()
{
	++NodeActivationCount;

	Super::OnActivate();
}

void UFlowNode_YapDialogue::ExecuteInput(const FName& PinName)
{
	UE_LOG(FlowYap, Warning, TEXT("ExecuteInput, conversation: %s"), *ConversationName.ToString());

	Super::ExecuteInput(PinName);
}

void UFlowNode_YapDialogue::AddFragment()
{
	if (Fragments.Num() >= 9)
	{
		return;
	}
	
	Fragments.Add(FFlowYapFragment());

	OnReconstructionRequested.ExecuteIfBound();
}

void UFlowNode_YapDialogue::RemoveFragmentByID(int64 EditorID)
{
	Fragments.RemoveAll(
		[&]
		(FFlowYapFragment& Fragment)
		{
			if (Fragments.Num() <= 1)
			{
				return false;
			}
			
			return Fragment.GetEditorID() == EditorID;
		}
	);
	
	OnReconstructionRequested.ExecuteIfBound();
}

#if WITH_EDITOR

bool UFlowNode_YapDialogue::GetDynamicTitleColor(FLinearColor& OutColor) const
{
	if (!Character)
	{
		return Super::GetDynamicTitleColor(OutColor);
	}

	OutColor = Character->GetEntityColor();

	return true;
}

bool UFlowNode_YapDialogue::SupportsContextPins() const
{
	return true;
}

bool UFlowNode_YapDialogue::GetUsesMultipleInputs()
{
	if (GetIsPlayerPrompt())
	{
		return false;
	}
	
	return true;
}

bool UFlowNode_YapDialogue::GetUsesMultipleOutputs()
{
	return true;
}

TArray<FFlowPin> UFlowNode_YapDialogue::GetContextInputs()
{
	InputPins.Empty();

	TArray<FFlowPin> ContextInputPins;

	uint8 NumPins = GetUsesMultipleInputs() ? Fragments.Num() : 1;

	for (uint8 Index = 0; Index < NumPins; ++Index)
	{
		ContextInputPins.Add(FName("In", Index));
	}

	return ContextInputPins;	
}

TArray<FFlowPin> UFlowNode_YapDialogue::GetContextOutputs()
{
	// Workaround for annoying editor code, pins that aren't kept on the CDO do dumb things (see root engine class code).
	OutputPins.Empty();

	TArray<FFlowPin> ContextOutputPins;

	uint8 NumPins = GetUsesMultipleOutputs() ? Fragments.Num() : 1;
	
	for (uint8 Index = 0; Index < NumPins; ++Index)
	{
		ContextOutputPins.Add(FName("Out", Index));
	}
	
	ContextOutputPins.Add(FName("Bypass"));
	
	return ContextOutputPins;
}

void UFlowNode_YapDialogue::SetIsPlayerPrompt(bool NewValue)
{
	bIsPlayerPrompt = NewValue;

	OnReconstructionRequested.ExecuteIfBound();
}

void UFlowNode_YapDialogue::SetNodeActivationLimit(int32 NewValue)
{
	NodeActivationLimit = NewValue;
}

FSlateBrush* UFlowNode_YapDialogue::GetSpeakerPortraitBrush(const FName& RequestedPortraitKey) const
{
	if (Character)
	{
		return Character->GetPortraitBrush(RequestedPortraitKey);
	}

	return nullptr;
}
#endif

#undef LOCTEXT_NAMESPACE