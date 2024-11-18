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
	Category = TEXT("Yap");

	NodeStyle = EFlowNodeStyle::Custom;

	bIsPlayerPrompt = false;
	
	NodeActivationLimit = 0;
	
	FragmentSequencing = EFlowYapMultipleFragmentSequencing::Sequential;

	Interruptible = EFlowYapInterruptible::UseProjectDefaults;

	// Always have at least one fragment.
	Fragments.Add(FYapFragment());

	OutputPins = {};
	
#if WITH_EDITOR
	UYapProjectSettings::RegisterTagFilter(this, GET_MEMBER_NAME_CHECKED(ThisClass, DialogueTag), EFlowYap_TagFilter::Prompts);
	
	if (IsTemplate())
	{
		UGameplayTagsManager::Get().OnFilterGameplayTagChildren.AddUObject(this, &ThisClass::OnFilterGameplayTagChildren);
	}
#endif
}

FText UFlowNode_YapDialogue::GetSpeakerName() const
{
	return FText::GetEmpty();
	
	/*
	if (!Character)
	{
		return LOCTEXT("DialogueNodeMissingCharacter", "NO CHARACTER SET");
	}

	return Character->GetEntityName();
	*/
}

const UTexture2D* UFlowNode_YapDialogue::GetDefaultSpeakerPortrait() const
{
	return nullptr;
	
	/*
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
	*/
}

const UTexture2D* UFlowNode_YapDialogue::GetSpeakerPortrait(const FName& RequestedMoodKey) const
{
	return nullptr;

	/*
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
	*/
}

const TArray<FYapFragment>& UFlowNode_YapDialogue::GetFragments() const
{
	return Fragments;
}

uint8 UFlowNode_YapDialogue::GetNumFragments() const
{
	return Fragments.Num();
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

void UFlowNode_YapDialogue::OnActivate()
{
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
		RunFragments();
	}
}

void UFlowNode_YapDialogue::Cleanup()
{
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

bool UFlowNode_YapDialogue::GetInterruptible() const
{
	if (Interruptible == EFlowYapInterruptible::UseProjectDefaults)
	{
		return UYapProjectSettings::Get()->GetDialogueInterruptibleByDefault();
	}
	else
	{
		return Interruptible == EFlowYapInterruptible::Interruptible;
	}
}

EFlowYapInterruptible UFlowNode_YapDialogue::GetInterruptibleSetting() const
{
	return Interruptible;
}

// ================================================================================================

void UFlowNode_YapDialogue::InvalidateFragmentTags()
{
	for (uint8 FragmentIndex = 0; FragmentIndex < Fragments.Num(); ++FragmentIndex)
	{
		FYapFragment& Fragment = Fragments[FragmentIndex];

		Fragment.InvalidateFragmentTag();
	}
}

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
		// TODO - auto select last option. Project setting? Overridable in dialogue node???
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

void UFlowNode_YapDialogue::RunFragments()
{
	bool bStartedSuccessfully = false;
	
	for (uint8 i = 0; i < Fragments.Num(); ++i)
	{
		switch (FragmentSequencing)
		{
			case EFlowYapMultipleFragmentSequencing::SelectOne:
			{
				bStartedSuccessfully = RunFragment(i, false);
				break;
			}
			case EFlowYapMultipleFragmentSequencing::Sequential:
			{
				bStartedSuccessfully = RunFragment(i, true);
				break;
			}
		}

		if (bStartedSuccessfully)
		{
			break;
		}
	}
	
	if (bStartedSuccessfully)
	{
		++NodeActivationCount;
	}
	else
	{
		TriggerOutput(FName("Bypass"), true);
	}
}

bool UFlowNode_YapDialogue::RunFragment(uint8 FragmentIndex, bool bRunNext)
{
	if (!Fragments.IsValidIndex(FragmentIndex))
	{
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

		FName StartPinName = Fragment.GetStartPinName();
		
		if (StartPinName != NAME_None)
		{
			TriggerOutput(StartPinName, true);
		}

		double Time = Fragment.GetBit().GetTime();

		if (Time == 0)
		{
			OnFragmentComplete(FragmentIndex, bRunNext);
		}
		else
		{
			GetWorld()->GetTimerManager().SetTimer(TimerHandle, FTimerDelegate::CreateUObject(this, &ThisClass::OnFragmentComplete, FragmentIndex, bRunNext), Time, false);
		}

		return true;
	}
	else
	{
		return false;
	}
}

void UFlowNode_YapDialogue::OnFragmentComplete(uint8 FragmentIndex, bool bRunNext)
{
	FYapFragment& Fragment = Fragments[FragmentIndex];

	GetWorld()->GetSubsystem<UYapSubsystem>()->BroadcastDialogueEnd(this, FragmentIndex);

	double PaddingTime = Fragments[FragmentIndex].GetPaddingToNextFragment();

	const FName EndPinName = Fragment.GetEndPinName();

	if (!GetIsPlayerPrompt() && Fragment.GetShowOnEndPin())
	{
		TriggerOutput(EndPinName, true);
	}
	
	if (PaddingTime > 0)
	{
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, FTimerDelegate::CreateUObject(this, &ThisClass::OnPaddingTimeComplete, FragmentIndex, bRunNext), PaddingTime, false);
	}
	else
	{
		OnPaddingTimeComplete(FragmentIndex, bRunNext);
	}

#if WITH_EDITOR
	FragmentEndedTime = GetWorld()->GetTimeSeconds();
#endif
}


void UFlowNode_YapDialogue::OnPaddingTimeComplete(uint8 FragmentIndex, bool bRunNext)
{
#if WITH_EDITOR
	RunningFragmentIndex.Reset();
#endif

	if (GetIsPlayerPrompt())
	{
		FYapFragment& Fragment = Fragments[FragmentIndex];

		FName EndPinName("PromptOut_" + Fragment.GetGuid().ToString());

		TriggerOutput(EndPinName, true);
	}
	else
	{
		if (bRunNext)
		{
			for (uint8 NextIndex = FragmentIndex + 1; NextIndex < Fragments.Num(); ++NextIndex)
			{
				if (RunFragment(NextIndex))
				{
					return;
				}
			}
		}

		TriggerOutput(FName("Out"), true);
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

#if WITH_EDITOR

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

EFlowYapMultipleFragmentSequencing UFlowNode_YapDialogue::GetMultipleFragmentSequencing() const
{
	return FragmentSequencing;
}

TArray<FFlowPin> UFlowNode_YapDialogue::GetContextOutputs() const
{
	TArray<FFlowPin> ContextOutputPins;

	if (!bIsPlayerPrompt)
	{
		ContextOutputPins.Add(FName("Out"));
	}

	for (uint8 Index = 0; Index < Fragments.Num(); ++Index)
	{
		const FYapFragment& Fragment = Fragments[Index];
		
		if (Fragment.GetShowOnEndPin())
		{
			ContextOutputPins.Add(Fragment.GetEndPin());
		}
		
		if (Fragment.GetShowOnStartPin())
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

void UFlowNode_YapDialogue::SetIsPlayerPrompt(bool NewValue)
{
	bIsPlayerPrompt = NewValue;

	OnReconstructionRequested.ExecuteIfBound();
}

void UFlowNode_YapDialogue::SetNodeActivationLimit(int32 NewValue)
{
	bool bBypassRequired = IsBypassPinRequired();
	
	NodeActivationLimit = NewValue;

	if (bBypassRequired != IsBypassPinRequired())
	{
		OnReconstructionRequested.Execute();
	}
}

void UFlowNode_YapDialogue::CycleFragmentSequencingMode()
{
	uint8 AsInt = static_cast<uint8>(FragmentSequencing);

	if (++AsInt >= static_cast<uint8>(EFlowYapMultipleFragmentSequencing::COUNT))
	{
		AsInt = 0;
	}

	FragmentSequencing = static_cast<EFlowYapMultipleFragmentSequencing>(AsInt);
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

void UFlowNode_YapDialogue::AddFragment(int32 InsertionIndex)
{
	if (Fragments.Num() >= 255)
	{
		// TODO nicer logging
		UE_LOG(FlowYap, Warning, TEXT("Yap is currently hard-coded to prevent more than 256 fragments per dialogeue node, sorry!"));
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

int32 UFlowNode_YapDialogue::GetFragmentIndex(const FGuid& Guid) const
{
	for (int32 i = 0; i < Fragments.Num(); ++i)
	{
		if (Fragments[i].GetGuid() == Guid)
		{
			return i;
		}
	}

	return -1;
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

void UFlowNode_YapDialogue::ForceReconstruction()
{
	OnReconstructionRequested.ExecuteIfBound();
}

void UFlowNode_YapDialogue::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	FProperty* Property = PropertyChangedEvent.Property;
	
	if (Property && Property->HasMetaData("Yap_ReconstructNodeOnChange"))
	{
		ForceReconstruction();
	}
}


void UFlowNode_YapDialogue::PostEditImport()
{
	Super::PostEditImport();

	for (FYapFragment& Fragment : Fragments)
	{
		Fragment.ResetGUID();
		Fragment.ResetOptionalPins();
	}
}

FSlateBrush* UFlowNode_YapDialogue::GetSpeakerPortraitBrush(const FName& RequestedMoodKey) const
{
	return nullptr;

	/*
	if (Character)
	{
		return Character->GetPortraitBrush(RequestedMoodKey);
	}

	return nullptr;
	*/
}
#endif

#undef LOCTEXT_NAMESPACE