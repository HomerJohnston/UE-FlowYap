// Copyright Ghost Pepper Games, Inc. All Rights Reserved.

#include "FlowYap/Nodes/FlowNode_YapDialogue.h"

#include "FlowYap/FlowYapCharacter.h"
#include "FlowYap/FlowYapLog.h"
#include "FlowYap/FlowYapProjectSettings.h"
#include "FlowYap/FlowYapSubsystem.h"

#define LOCTEXT_NAMESPACE "FlowYap"

UFlowNode_YapDialogue::UFlowNode_YapDialogue()
{
	Category = TEXT("Yap");

	NodeStyle = EFlowNodeStyle::Custom;

	bIsPlayerPrompt = false;
	
	NodeActivationLimit = 0;
	
	MultipleFragmentSequencing = EFlowYapMultipleFragmentSequencing::Sequential;

	Interruptible = EFlowYapInterruptible::UseProjectDefaults;

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

	const UFlowYapProjectSettings* Settings = UFlowYapProjectSettings::Get();

	if (Settings->GetMoodKeys().Num() == 0)
	{
		return nullptr;
	}

	const FName& Key = Settings->GetMoodKeys()[0];
	
	return GetSpeakerPortrait(Key);
}

const UTexture2D* UFlowNode_YapDialogue::GetSpeakerPortrait(const FName& RequestedMoodKey) const
{
	if (!Character)
	{
		return nullptr;
	}

	const TObjectPtr<UTexture2D>* Portrait = Character->GetPortraits().Find(RequestedMoodKey);

	if (Portrait)
	{
		return *Portrait;
	}
	else
	{
		return nullptr;
	}
}

const TArray<FFlowYapFragment>& UFlowNode_YapDialogue::GetFragments()
{
	return Fragments;
}

uint8 UFlowNode_YapDialogue::GetNumFragments() const
{
	return Fragments.Num();
}

int16 UFlowNode_YapDialogue::FindFragmentIndex(FFlowYapFragment* InFragment) const
{
	for (uint8 i = 0; i < Fragments.Num(); ++i)
	{
		if (&Fragments[i] == InFragment)
		{
			return i;
		}
	}

	return INDEX_NONE;
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
}

void UFlowNode_YapDialogue::ExecuteInput(const FName& PinName)
{
	++NodeActivationCount;

	TOptional<uint8> FragmentIndex;
	
	switch (MultipleFragmentSequencing)
	{
	case EFlowYapMultipleFragmentSequencing::Sequential:
		{
			for (uint8 i = 0; i < Fragments.Num(); ++i)
			{
				if (Fragments[i].TryActivate())
				{
					FragmentIndex = i;
					break;
				}
			}
			break;
		}
	case EFlowYapMultipleFragmentSequencing::Random:
		{
			return;
		}
	default:
		{
			return;
		}
	}

	if (!FragmentIndex)
	{
		TriggerOutput("Bypass", true, EFlowPinActivationType::Default);
		return;
	}
	
	// Choose a fragment and send its bit
	// TODO

	FFlowYapFragment& SelectedFragment = Fragments[FragmentIndex.GetValue()];
	
	GetWorld()->GetSubsystem<UFlowYapSubsystem>()->DialogueStart(ConversationName, SelectedFragment.GetBit());

	double Time = SelectedFragment.GetBit().GetTime();

	GetWorld()->GetTimerManager().SetTimer(TimerHandle, FTimerDelegate::CreateUObject(this, &ThisClass::OnTextTimeComplete, FragmentIndex.GetValue()), Time, false);

	TriggerOutput(FName("DialogueStart", FragmentIndex.GetValue() + 1));

#if WITH_EDITOR
	RunningFragmentIndex = FragmentIndex;
#endif
}

bool UFlowNode_YapDialogue::GetInterruptible() const
{
	if (Interruptible == EFlowYapInterruptible::UseProjectDefaults)
	{
		return UFlowYapProjectSettings::Get()->GetDialogueInterruptibleByDefault();
	}
	else
	{
		return Interruptible == EFlowYapInterruptible::Interruptible;
	}
}

void UFlowNode_YapDialogue::OnTextTimeComplete(uint8 FragmentIndex)
{
	FFlowYapFragment& SelectedFragment = Fragments[FragmentIndex];
	
	GetWorld()->GetSubsystem<UFlowYapSubsystem>()->DialogueEnd(ConversationName, SelectedFragment.GetBit());

	double PaddingTime = UFlowYapProjectSettings::Get()->GetDialoguePaddingTime();

	if (PaddingTime > 0)
	{
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, FTimerDelegate::CreateUObject(this, &ThisClass::OnPaddingTimeComplete, FragmentIndex), PaddingTime, false);
	}
	else
	{
		TriggerOutput(FName("DialogueEnd", FragmentIndex + 1), true);
	}
	
#if WITH_EDITOR
	RunningFragmentIndex.Reset();
#endif
}

void UFlowNode_YapDialogue::OnPaddingTimeComplete(uint8 FragmentIndex)
{
	TriggerOutput(FName("DialogueEnd", FragmentIndex + 1), true);
}

const FFlowYapFragment* UFlowNode_YapDialogue::GetFragmentByIndex(int16 Index) const
{
	if (!Fragments.IsValidIndex(Index))
	{
		return nullptr;
	}
	
	return &Fragments[Index];
}

FFlowYapFragment* UFlowNode_YapDialogue::GetFragmentByIndexMutable(int16 Index)
{
	if (!Fragments.IsValidIndex(Index))
	{
		return nullptr;
	}
	
	return &Fragments[Index];
}

TArray<FFlowYapFragment>& UFlowNode_YapDialogue::GetFragmentsMutable()
{
	return Fragments;
}

void UFlowNode_YapDialogue::AddFragment()
{
	if (Fragments.Num() >= 9)
	{
		return;
	}

	FFlowYapFragment NewFragment;
	
	Fragments.Emplace(NewFragment);

	UpdateFragmentIndices();

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
	return false;
}

bool UFlowNode_YapDialogue::GetUsesMultipleOutputs()
{
	return true;
}

EFlowYapMultipleFragmentSequencing UFlowNode_YapDialogue::GetMultipleFragmentSequencing() const
{
	return MultipleFragmentSequencing;
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

	for (uint8 Index = 1; Index <= Fragments.Num(); ++Index)
	{
		ContextOutputPins.Add(FName("DialogueEnd", Index));
		ContextOutputPins.Add(FName("DialogueStart", Index));
		ContextOutputPins.Add(FName("DialogueInterrupt", Index));
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

void UFlowNode_YapDialogue::CycleFragmentSequencingMode()
{
	uint8 AsInt = static_cast<uint8>(MultipleFragmentSequencing);

	if (++AsInt >= static_cast<uint8>(EFlowYapMultipleFragmentSequencing::COUNT))
	{
		AsInt = 0;
	}

	MultipleFragmentSequencing = static_cast<EFlowYapMultipleFragmentSequencing>(AsInt);
}

void UFlowNode_YapDialogue::DeleteFragmentByIndex(int16 DeleteIndex)
{
	if (!Fragments.IsValidIndex(DeleteIndex))
	{
		UE_LOG(FlowYap, Error, TEXT("Invalid deletion index!"));
	}
	
	Fragments.RemoveAt(DeleteIndex);

	UpdateFragmentIndices();
	
	OnReconstructionRequested.ExecuteIfBound();
}

void UFlowNode_YapDialogue::InsertFragment(uint8 Index)
{
	if (Fragments.Num() >= 9)
	{
		return;
	}

	FFlowYapFragment NewFragment;
	Fragments.Insert(NewFragment, Index);

	UpdateFragmentIndices();

	OnReconstructionRequested.ExecuteIfBound();
}

void UFlowNode_YapDialogue::UpdateFragmentIndices()
{
	for (int i = 0; i < Fragments.Num(); ++i)
	{
		Fragments[i].SetIndexInDialogue(i);
	}
}

void UFlowNode_YapDialogue::SwapFragments(uint8 IndexA, uint8 IndexB)
{
	Fragments.Swap(IndexA, IndexB);

	UpdateFragmentIndices();

	//OnReconstructionRequested.ExecuteIfBound();
}

FSlateBrush* UFlowNode_YapDialogue::GetSpeakerPortraitBrush(const FName& RequestedMoodKey) const
{
	if (Character)
	{
		return Character->GetPortraitBrush(RequestedMoodKey);
	}

	return nullptr;
}
#endif

#undef LOCTEXT_NAMESPACE