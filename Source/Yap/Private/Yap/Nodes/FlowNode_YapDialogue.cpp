// Copyright Ghost Pepper Games, Inc. All Rights Reserved.

#include "Yap/Nodes/FlowNode_YapDialogue.h"

#include "GameplayTagsManager.h"
#include "Yap/YapBit.h"
#include "Yap/YapFragment.h"
#include "Yap/YapProjectSettings.h"
#include "Yap/YapSubsystem.h"

#define LOCTEXT_NAMESPACE "FlowYap"

UFlowNode_YapDialogue::UFlowNode_YapDialogue()
{
#if WITH_EDITOR
	Category = TEXT("Yap");

	NodeStyle = EFlowNodeStyle::Custom;
#endif

	DialogueNodeType = EYapDialogueNodeType::Talk;
	
	NodeActivationLimit = 0;
	
	TalkSequencing = EYapDialogueTalkSequencing::RunAll;

	Skippable = EYapDialogueSkippable::Default;

	// Always have at least one fragment.
	Fragments.Add(FYapFragment());

	// The node will only have certain context-outputs which depend on the node type. 
	OutputPins = {};
	
#if WITH_EDITOR
	// TODO use the subsystem to manage crap like this
	UYapProjectSettings::RegisterTagFilter(this, GET_MEMBER_NAME_CHECKED(ThisClass, DialogueTag), EYap_TagFilter::Prompts);
	
	if (IsTemplate())
	{
		UGameplayTagsManager::Get().OnFilterGameplayTagChildren.AddUObject(this, &ThisClass::OnFilterGameplayTagChildren);
	}
#endif
}

int16 UFlowNode_YapDialogue::FindFragmentIndex(const FGuid& InFragmentGuid) const
{
	for (uint8 i = 0; i < Fragments.Num(); ++i)
	{
		if (Fragments[i].GetGuid() == InFragmentGuid)
		{
			return i;
		}
	}

	return INDEX_NONE;
}

FYapFragment* UFlowNode_YapDialogue::FindTaggedFragment(const FGameplayTag& Tag)
{
	for (FYapFragment& Fragment : Fragments)
	{
		if (Fragment.GetFragmentTag() == Tag)
		{
			return &Fragment;
		}
	}

	return nullptr;
}

void UFlowNode_YapDialogue::InitializeInstance()
{
	Super::InitializeInstance();

	for (FYapFragment& Fragment : Fragments)
	{
		if (Fragment.GetFragmentTag().IsValid())
		{
			UYapSubsystem* Subsystem = GetWorld()->GetSubsystem<UYapSubsystem>();
			Subsystem->RegisterTaggedFragment(Fragment.GetFragmentTag(), this);
		}
	}
}

void UFlowNode_YapDialogue::ExecuteInput(const FName& PinName)
{
	if (ActivationLimitsMet())
	{
		TriggerOutput("Bypass", true, EFlowPinActivationType::Default);
		return;
	}

	if (GetIsPlayerPrompt())
	{
		BroadcastPrompts();
	}
	else
	{
		FindStartingFragment();
	}
}

void UFlowNode_YapDialogue::OnPassThrough_Implementation()
{
	if (GetIsPlayerPrompt())
	{
		TriggerOutput("Bypass", true, EFlowPinActivationType::PassThrough);
	}
	else
	{
		TriggerOutput("Out", true, EFlowPinActivationType::PassThrough);
	}
}

bool UFlowNode_YapDialogue::GetSkippable() const
{
	if (Skippable == EYapDialogueSkippable::Default)
	{
		return UYapProjectSettings::Get()->GetDialogueSkippableByDefault();
	}
	else
	{
		return Skippable == EYapDialogueSkippable::Skippable;
	}
}

EYapDialogueSkippable UFlowNode_YapDialogue::GetSkippableSetting() const
{
	return Skippable;
}

// ================================================================================================

#if WITH_EDITOR
void UFlowNode_YapDialogue::InvalidateFragmentTags()
{
	for (uint8 FragmentIndex = 0; FragmentIndex < Fragments.Num(); ++FragmentIndex)
	{
		FYapFragment& Fragment = Fragments[FragmentIndex];

		Fragment.InvalidateFragmentTag();
	}
}
#endif

void UFlowNode_YapDialogue::BroadcastPrompts()
{
	TArray<uint8> BroadcastedFragments;
	
 	for (uint8 FragmentIndex = 0; FragmentIndex < Fragments.Num(); ++FragmentIndex)
	{
		FYapFragment& Fragment = Fragments[FragmentIndex];

		if (Fragment.IsActivationLimitMet())
		{
			continue;
		}

		GetWorld()->GetSubsystem<UYapSubsystem>()->BroadcastPrompt(this, FragmentIndex);
 		
		BroadcastedFragments.Add(FragmentIndex);
	}

	if (BroadcastedFragments.Num() == 0)
	{
		TriggerOutput(FName("Bypass"), true);
	}
	else if (BroadcastedFragments.Num() == 1)
	{
		GetWorld()->GetSubsystem<UYapSubsystem>()->OnFinishedBroadcastingPrompts();
		// TODO - auto select last option. Project setting? Overridable in dialogue node???
		// TODO instead, I should emit a "Prompts Finished Broadcasting" type of event and let the game decide whether to automatically select it or not
	}
}

void UFlowNode_YapDialogue::RunPrompt(uint8 FragmentIndex)
{
	if (!RunFragment(FragmentIndex))
	{
		// TODO log error? This should never happen?
		
		TriggerOutput(FName("Bypass"), true);
	}

	++NodeActivationCount;
}

// ================================================================================================

void UFlowNode_YapDialogue::FindStartingFragment()
{
	bool bStartedSuccessfully = false;
	
	for (uint8 i = 0; i < Fragments.Num(); ++i)
	{
		bStartedSuccessfully = RunFragment(i);

		if (bStartedSuccessfully)
		{
			++NodeActivationCount;
			break;
		}
	}
	
	if (!bStartedSuccessfully)
	{
		TriggerOutput(FName("Bypass"), true);
	}
}

bool UFlowNode_YapDialogue::RunFragment(uint8 FragmentIndex)
{
	if (!Fragments.IsValidIndex(FragmentIndex))
	{
		UE_LOG(LogYap, Error, TEXT("Attempted run invalid fragment index!"));
		return false;
	}
	
	FYapFragment& Fragment = Fragments[FragmentIndex];

	if (TryBroadcastFragment(FragmentIndex))
	{
		Fragment.IncrementActivations();

#if WITH_EDITOR
		RunningFragmentIndex = FragmentIndex;
		FragmentStartedTime = GetWorld()->GetTimeSeconds();
#endif

		if (Fragment.UsesStartPin())
		{
			const FFlowPin StartPin = Fragment.GetStartPin();
			TriggerOutput(StartPin.PinName, false);
		}

		double Time = Fragment.GetBit().GetTime();

		if (Time <= 0.f)
		{
			WhenFragmentComplete(FragmentIndex);
		}
		else
		{
			GetWorld()->GetTimerManager().SetTimer(FragmentTimerHandle, FTimerDelegate::CreateUObject(this, &ThisClass::WhenFragmentComplete, FragmentIndex), Time, false);
		}

		return true;
	}
	else
	{
		return false;
	}
}

void UFlowNode_YapDialogue::WhenFragmentComplete(uint8 FragmentIndex)
{
	FYapFragment& Fragment = Fragments[FragmentIndex];

	GetWorld()->GetSubsystem<UYapSubsystem>()->BroadcastDialogueEnd(this, FragmentIndex);

	double PaddingTime = Fragments[FragmentIndex].GetPaddingToNextFragment();

	if (Fragment.UsesEndPin())
	{
		const FFlowPin EndPin = Fragment.GetEndPin();
		TriggerOutput(EndPin.PinName, false);
	}
	
	if (PaddingTime > 0)
	{
		GetWorld()->GetTimerManager().SetTimer(FragmentTimerHandle, FTimerDelegate::CreateUObject(this, &ThisClass::WhenPaddingTimeComplete, FragmentIndex), PaddingTime, false);
	}
	else
	{
		WhenPaddingTimeComplete(FragmentIndex);
	}

#if WITH_EDITOR
	FragmentEndedTime = GetWorld()->GetTimeSeconds();
#endif
}


void UFlowNode_YapDialogue::WhenPaddingTimeComplete(uint8 FragmentIndex)
{
#if WITH_EDITOR
	RunningFragmentIndex.Reset();
#endif

	if (GetIsPlayerPrompt())
	{
		FYapFragment& Fragment = Fragments[FragmentIndex];

		FName PromptOutPinName("PromptOut_" + Fragment.GetGuid().ToString());

		TriggerOutput(PromptOutPinName, true);
	}
	else
	{
		if (TalkSequencing == EYapDialogueTalkSequencing::SelectOne)
		{
			TriggerOutput(FName("Out"), true);
		}
		else
		{
			for (uint8 NextIndex = FragmentIndex + 1; NextIndex < Fragments.Num(); ++NextIndex)
			{
				bool bRanNextFragment =  RunFragment(NextIndex);

				if (!bRanNextFragment && TalkSequencing == EYapDialogueTalkSequencing::RunUntilFailure)
				{
					// Whoops, this is the end of the line
					TriggerOutput(FName("Out"), true);
					return;
				}
				else if (bRanNextFragment)
				{
					// We'll delegate further behavior to the next running fragment
					return;
				}
			}

			// No more fragments to try and run!
			TriggerOutput(FName("Out"), true);
		}
	}
}

bool UFlowNode_YapDialogue::IsBypassPinRequired() const
{
	// If there are any conditions, we will need a bypass node in case all conditions are false
	if (Conditions.Num() > 0 || GetNodeActivationLimit() > 0)
	{
		return true;
	}
	
	// If all of the fragments have conditions, we will need a bypass node in case all fragments are unusable
	for (const FYapFragment& Fragment : Fragments)
	{
		if (Fragment.GetConditions().Num() == 0 && Fragment.GetActivationLimit() == 0)
		{
			return false;
		}
	}

	return true;
}

bool UFlowNode_YapDialogue::TryBroadcastFragment(uint8 FragmentIndex)
{
	const FYapFragment& Fragment = GetFragmentByIndex(FragmentIndex);
	
	if (!Fragment.CheckConditions())
	{
		return false;
	}
	
	if (Fragment.IsActivationLimitMet())
	{
		return false;
	}
	
	GetWorld()->GetSubsystem<UYapSubsystem>()->BroadcastDialogueStart(this, FragmentIndex);

	return true;
}

const FYapFragment& UFlowNode_YapDialogue::GetFragmentByIndex(uint8 Index) const
{
	check(Fragments.IsValidIndex(Index));

	return Fragments[Index];
}

#if WITH_EDITOR
FYapFragment& UFlowNode_YapDialogue::GetFragmentByIndexMutable(uint8 Index)
{
	check (Fragments.IsValidIndex(Index))

	return Fragments[Index];
}

TArray<FYapFragment>& UFlowNode_YapDialogue::GetFragmentsMutable()
{
	return Fragments;
}

void UFlowNode_YapDialogue::RemoveFragment(int32 Index)
{
	Fragments.RemoveAt(Index);
}

FText UFlowNode_YapDialogue::GetNodeTitle() const
{
	if (IsTemplate())
	{
		return FText::FromString("Dialogue");
	}

	return FText::FromString(" ");
}

bool UFlowNode_YapDialogue::GetDynamicTitleColor(FLinearColor& OutColor) const
{
	return Super::GetDynamicTitleColor(OutColor);

	/*
	if (!Character)
	{
		return Super::GetDynamicTitleColor(OutColor);
	}

	OutColor = Character->GetEntityColor();
	return true;
	*/
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

EYapDialogueTalkSequencing UFlowNode_YapDialogue::GetMultipleFragmentSequencing() const
{
	return TalkSequencing;
}

TArray<FFlowPin> UFlowNode_YapDialogue::GetContextOutputs() const
{
	TArray<FFlowPin> ContextOutputPins;

	if (!GetIsPlayerPrompt())
	{
		ContextOutputPins.Add(FName("Out"));
	}

	for (uint8 Index = 0; Index < Fragments.Num(); ++Index)
	{
		const FYapFragment& Fragment = Fragments[Index];
		
		if (Fragment.UsesEndPin())
		{
			ContextOutputPins.Add(Fragment.GetEndPin());
		}
		
		if (Fragment.UsesStartPin())
		{
			ContextOutputPins.Add(Fragment.GetStartPin());
		}

		if (GetIsPlayerPrompt())
		{
			ContextOutputPins.Add(Fragment.GetPromptPin());
		}
	}

	if (IsBypassPinRequired())
	{
		ContextOutputPins.Add(FName("Bypass"));
	}
	
	return ContextOutputPins;
}

void UFlowNode_YapDialogue::SetNodeActivationLimit(int32 NewValue)
{
	bool bBypassRequired = IsBypassPinRequired();
	
	NodeActivationLimit = NewValue;

	if (bBypassRequired != IsBypassPinRequired())
	{
		OnReconstructionRequested.ExecuteIfBound();
	}
}

void UFlowNode_YapDialogue::CycleFragmentSequencingMode()
{
	uint8 AsInt = static_cast<uint8>(TalkSequencing);

	if (++AsInt >= static_cast<uint8>(EYapDialogueTalkSequencing::COUNT))
	{
		AsInt = 0;
	}

	TalkSequencing = static_cast<EYapDialogueTalkSequencing>(AsInt);
}

void UFlowNode_YapDialogue::DeleteFragmentByIndex(int16 DeleteIndex)
{
	if (!Fragments.IsValidIndex(DeleteIndex))
	{
		UE_LOG(LogYap, Error, TEXT("Invalid deletion index!"));
	}

	Fragments.RemoveAt(DeleteIndex);

	UpdateFragmentIndices();
	
	OnReconstructionRequested.ExecuteIfBound();
}

void UFlowNode_YapDialogue::AddFragment(int32 InsertionIndex)
{
	if (Fragments.Num() >= 255)
	{
		// TODO nicer logging
		UE_LOG(LogYap, Warning, TEXT("Yap is currently hard-coded to prevent more than 256 fragments per dialogeue node, sorry!"));
		return;
	}

	if (InsertionIndex == INDEX_NONE)
	{
		InsertionIndex = Fragments.Num();
	}

	FYapFragment NewFragment;

	if (InsertionIndex > 0 || InsertionIndex >= Fragments.Num()) // TODO this looks messed up, wtf?
	{
		uint8 PreviousFragmentIndex = InsertionIndex - 1;

		if (Fragments.IsValidIndex(PreviousFragmentIndex))
		{
			NewFragment.GetBitMutable().SetCharacter(GetFragmentByIndex(PreviousFragmentIndex).GetBit().GetCharacterMutable());
		}
	}
	else
	{
		uint8 NextFragmentIndex = InsertionIndex + 1;

		if (Fragments.IsValidIndex(NextFragmentIndex))
		{
			NewFragment.GetBitMutable().SetCharacter(GetFragmentByIndex(NextFragmentIndex).GetBit().GetCharacterMutable());
		}
	}
	
	Fragments.Insert(NewFragment, InsertionIndex);

	UpdateFragmentIndices();

	//GetGraphNode()->ReconstructNode(); // TODO This works nicer but crashes because of pin connections. I might not need full reconstruction if I change how my multi-fragment nodes work.
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

	OnReconstructionRequested.ExecuteIfBound();
}

FString UFlowNode_YapDialogue::GetNodeDescription() const
{
	return "";
	
	//return UFlowYapProjectSettings::GetTrimmedGameplayTagString(EFlowYap_TagFilter::Prompts, DialogueTag);
}

void UFlowNode_YapDialogue::OnFilterGameplayTagChildren(const FString& String, TSharedPtr<FGameplayTagNode>& GameplayTagNode, bool& bArg) const
{
	const FGameplayTagContainer& ParentTagContainer = GameplayTagNode->GetParentTagNode()->GetSingleTagContainer();

	if (ParentTagContainer.HasTagExact(UYapProjectSettings::Get()->DialogueTagsParent))
	{
		bArg = true;
	}

	bArg = false;
}
#endif

bool UFlowNode_YapDialogue::ActivationLimitsMet() const
{
	if (GetNodeActivationLimit() > 0 && GetNodeActivationCount() >= GetNodeActivationLimit())
	{
		return true;
	}

	for (int i = 0; i < Fragments.Num(); ++i)
	{
		int32 ActivationLimit = Fragments[i].GetActivationLimit();
		int32 ActivationCount = Fragments[i].GetActivationCount();

		if (ActivationLimit == 0 || ActivationCount < ActivationLimit)
		{
			return false;
		}
	}

	return true;
}

void UFlowNode_YapDialogue::ToggleNodeType()
{
	uint8 AsInt = static_cast<uint8>(DialogueNodeType);

	if (++AsInt >= static_cast<uint8>(EYapDialogueNodeType::COUNT))
	{
		AsInt = 0;
	}

	DialogueNodeType = static_cast<EYapDialogueNodeType>(AsInt);
}

#if WITH_EDITOR
void UFlowNode_YapDialogue::ForceReconstruction()
{
	OnReconstructionRequested.ExecuteIfBound();
}
#endif

#if WITH_EDITOR
void UFlowNode_YapDialogue::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif

#if WITH_EDITOR
void UFlowNode_YapDialogue::PostEditImport()
{
	Super::PostEditImport();

	for (FYapFragment& Fragment : Fragments)
	{
		Fragment.ResetGUID();
		Fragment.ResetOptionalPins();
	}
}
#endif

#undef LOCTEXT_NAMESPACE