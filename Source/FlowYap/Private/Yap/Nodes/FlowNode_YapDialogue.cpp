// Copyright Ghost Pepper Games, Inc. All Rights Reserved.

#include "Yap/Nodes/FlowNode_YapDialogue.h"

#include "GameplayTagsManager.h"
#include "Logging/StructuredLog.h"
#include "Yap/FlowYapBit.h"
#include "Yap/FlowYapFragment.h"
#include "Yap/FlowYapProjectSettings.h"
#include "Yap/FlowYapSubsystem.h"

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
	Fragments.Add(FFlowYapFragment());

#if WITH_EDITOR
	UFlowYapProjectSettings::RegisterTagFilter(this, GET_MEMBER_NAME_CHECKED(ThisClass, DialogueTag), EFlowYap_TagFilter::Prompts);
	
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

const TArray<FFlowYapFragment>& UFlowNode_YapDialogue::GetFragments() const
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

FFlowYapFragment* UFlowNode_YapDialogue::FindTaggedFragment(const FGameplayTag& Tag)
{
	for (FFlowYapFragment& Fragment : Fragments)
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

	for (FFlowYapFragment& Fragment : Fragments)
	{
		if (Fragment.GetFragmentTag().IsValid())
		{
			UFlowYapSubsystem* Subsystem = GetWorld()->GetSubsystem<UFlowYapSubsystem>();
			Subsystem->RegisterTaggedFragment(Fragment.GetFragmentTag(), this);
		}
	}
}

void UFlowNode_YapDialogue::OnActivate()
{
}

void UFlowNode_YapDialogue::ExecuteInput(const FName& PinName)
{
	if (NodeActivationLimit > 0 && NodeActivationCount++ >= NodeActivationLimit)
	{
		TriggerOutput("Bypass", true, EFlowPinActivationType::Default);
		return;
	}

	if (bIsPlayerPrompt)
	{
		BroadcastPrompts();
	}
	else
	{
		RunFragmentsAsDialogue(0, FragmentSequencing, false);
	}
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

// ================================================================================================

void UFlowNode_YapDialogue::BroadcastPrompts()
{
	TArray<uint8> BroadcastedFragments;
	
 	for (uint8 FragmentIndex = 0; FragmentIndex < Fragments.Num(); ++FragmentIndex)
	{
		FFlowYapFragment& Fragment = Fragments[FragmentIndex];

		if (Fragment.IsActivationLimitMet())
		{
			continue;
		}

		GetWorld()->GetSubsystem<UFlowYapSubsystem>()->BroadcastPrompt(this, Fragment.GetIndexInDialogue());
 		
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

void UFlowNode_YapDialogue::RunPromptAsDialogue(uint8 FragmentIndex)
{
	if (!RunFragment(FragmentIndex, EFlowYapMultipleFragmentSequencing::Prompt))
	{
		// TODO log error? This should never happen?
		
		TriggerOutput(FName("Bypass"), true);
	}
}

// ================================================================================================

void UFlowNode_YapDialogue::RunFragmentsAsDialogue(uint8 StartIndex, EFlowYapMultipleFragmentSequencing SequencingMode, bool bSuccess)
{
	for (uint8 FragmentIndex = StartIndex; FragmentIndex < Fragments.Num(); ++FragmentIndex)
	{
		if (RunFragment(FragmentIndex, SequencingMode))
		{
			return;
		}
	}

	if (bSuccess && GetConnection("Out").NodeGuid.IsValid())
	{
		TriggerOutput(FName("Out"), true);
	}
	else if (GetConnection("Bypass").NodeGuid.IsValid())
	{
		TriggerOutput(FName("Bypass"), true);
	}
}

bool UFlowNode_YapDialogue::RunFragment(uint8 FragmentIndex, EFlowYapMultipleFragmentSequencing SequencingMode)
{
	FFlowYapFragment& Fragment = Fragments[FragmentIndex];

	if (!TryBroadcastFragmentAsDialogue(Fragment))
	{
		return false;
	}

	UE_LOGFMT(FlowYap, Display, "Activating fragment {0}", FragmentIndex);
		
	TriggerOutput(FName("FragmentStart", FragmentIndex + 1));

	const FFlowYapBit& Bit = Fragment.GetBit();

	double Time = Bit.GetTime();

	if (Time == 0)
	{
		UE_LOGFMT(FlowYap, Warning, "Encountered 0 time for fragment containing text: {0}, using project default minimum instead", Bit.GetDialogueText().ToString());
		Time = UFlowYapProjectSettings::Get()->GetMinimumFragmentTime();
	}

	if (Time == 0)
	{
		OnFragmentComplete(FragmentIndex, SequencingMode);
	}
	else
	{
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, FTimerDelegate::CreateUObject(this, &ThisClass::OnFragmentComplete, FragmentIndex, SequencingMode), Time, false);
	}
		
#if WITH_EDITOR
	UE_LOG(FlowYap, Warning, TEXT("Setting running fragment index"));
	RunningFragmentIndex = FragmentIndex;
	FragmentStartedTime = GetWorld()->GetTimeSeconds();
#endif

	return true;
}

void UFlowNode_YapDialogue::OnFragmentComplete(uint8 FragmentIndex, EFlowYapMultipleFragmentSequencing SequencingMode)
{	
	const FFlowYapBit& Bit = Fragments[FragmentIndex].GetBit();
	
	GetWorld()->GetSubsystem<UFlowYapSubsystem>()->BroadcastDialogueEnd(this, FragmentIndex);

	double PaddingTime = Fragments[FragmentIndex].GetPaddingToNextFragment();

	TriggerOutput(FName("FragmentEnd", FragmentIndex + 1), true);
	
	if (PaddingTime > 0)
	{
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, FTimerDelegate::CreateUObject(this, &ThisClass::OnPaddingTimeComplete, FragmentIndex, SequencingMode), PaddingTime, false);
	}
	else
	{
		OnPaddingTimeComplete(FragmentIndex, SequencingMode);
	}

#if WITH_EDITOR
	FragmentEndedTime = GetWorld()->GetTimeSeconds();
#endif
}


void UFlowNode_YapDialogue::OnPaddingTimeComplete(uint8 FragmentIndex, EFlowYapMultipleFragmentSequencing SequencingMode)
{
#if WITH_EDITOR
	UE_LOG(FlowYap, Warning, TEXT("Resetting running fragment index"));
	RunningFragmentIndex.Reset();
#endif

	switch (SequencingMode)
	{
	case EFlowYapMultipleFragmentSequencing::Sequential:
		{
			RunFragmentsAsDialogue(FragmentIndex + 1, SequencingMode, true);
			break;
		}
	case EFlowYapMultipleFragmentSequencing::SelectOne:
		{
			if (GetConnection("Out").NodeGuid.IsValid())
			{
				TriggerOutput(FName("Out"), true);
			}
			else 
			{
				TriggerOutput(FName("Bypass"), true);
			}

			break;
		}
	case EFlowYapMultipleFragmentSequencing::Prompt:
		{
			break;
		}
	}
}

bool UFlowNode_YapDialogue::BypassPinRequired() const
{
	if (Conditions.Num() > 0)
	{
		return true;
	}

	for (const FFlowYapFragment& Fragment : Fragments)
	{
		if (Fragment.GetConditions().Num() == 0)
		{
			return false;
		}
	}

	return true;
}

bool UFlowNode_YapDialogue::TryBroadcastFragmentAsDialogue(FFlowYapFragment& Fragment)
{
	if (Fragment.IsActivationLimitMet())
	{
		return false;
	}

	Fragment.IncrementActivations();

	GetWorld()->GetSubsystem<UFlowYapSubsystem>()->BroadcastDialogueStart(this, Fragment.GetIndexInDialogue());

	return true;
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
	InsertFragment(Fragments.Num());

	/*
	FFlowYapFragment NewFragment;

	Fragments.Emplace(NewFragment);

	UpdateFragmentIndices();

	OnReconstructionRequested.ExecuteIfBound();
	*/
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

TArray<FFlowPin> UFlowNode_YapDialogue::GetContextOutputs()
{
	FragmentPinMap.Empty();
	OutPin = NAME_None;
	OnStartPins.Empty();
	OnEndPins.Empty();
	OptionalPins.Empty();
	BypassPin = NAME_None;

	TArray<FFlowPin> ContextOutputPins;

	if (bIsPlayerPrompt)
	{
		OutputPins.Remove(FName("Out"));
		OutPin = "";
	}
	else
	{
		OutPin = "Out";
	}

	for (uint8 Index = 0; Index < Fragments.Num(); ++Index)
	{
		FFlowYapFragment& Fragment = Fragments[Index];
		
		if (Fragments[Index].GetShowOnEndPin() || bIsPlayerPrompt)
		{
			FName PinName = FName("FragmentEnd_" + Fragment.GetGuid().ToString());
			FragmentPinMap.Add(PinName, Fragment.GetGuid());
			ContextOutputPins.Add(PinName);

			/*
			FFlowYapPinInfo NewPinInfo;
			NewPinInfo.ToolTip = INVTEXT("On End");
			NewPinInfo.DisconnectedColor = FLinearColor(0.300, 0.030, 0.005, 1.0);
			NewPinInfo.BottomPadding = 54;
			*/
			
			if (!bIsPlayerPrompt)
			{
				//NewPinInfo.PinStyle = EFlowYapPinStyle::Hyphen;
				OptionalPins.Add(PinName);
			}
			
			//PinInfo.Add(PinName, NewPinInfo);
			OnEndPins.Add(PinName);
		}
		
		if (Fragments[Index].GetShowOnStartPin())
		{
			FName PinName = FName("FragmentStart_" + Fragment.GetGuid().ToString());
			ContextOutputPins.Add(PinName);
			FragmentPinMap.Add(PinName, Fragment.GetGuid());

			/*
			FFlowYapPinInfo NewPinInfo;
			NewPinInfo.ToolTip = INVTEXT("On Start");
			NewPinInfo.DisconnectedColor = FLinearColor(0.300, 0.030, 0.005, 1.0);
			NewPinInfo.PinStyle = EFlowYapPinStyle::Hyphen;
			NewPinInfo.BottomPadding = 4;
			*/
			OptionalPins.Add(PinName);

			//PinInfo.Add(PinName, NewPinInfo);
			OnStartPins.Add(PinName);
		}
	}

	if (BypassPinRequired())
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
	NodeActivationLimit = NewValue;
}

void UFlowNode_YapDialogue::CycleFragmentSequencingMode()
{
	uint8 AsInt = static_cast<uint8>(FragmentSequencing);

	if (++AsInt >= static_cast<uint8>(EFlowYapMultipleFragmentSequencing::Prompt))
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

void UFlowNode_YapDialogue::InsertFragment(uint8 Index)
{
	if (Fragments.Num() >= 255)
	{
		return;
	}

	FFlowYapFragment NewFragment;

	if (Index > 0 || Index >= Fragments.Num())
	{
		uint8 PreviousFragmentIndex = Index - 1;

		if (Fragments.IsValidIndex(PreviousFragmentIndex))
		{
			NewFragment.GetBitMutable().SetCharacter(GetFragmentByIndex(PreviousFragmentIndex)->GetBit().GetCharacterMutable());
		}
	}
	else
	{
		uint8 NextFragmentIndex = Index + 1;

		if (Fragments.IsValidIndex(NextFragmentIndex))
		{
			NewFragment.GetBitMutable().SetCharacter(GetFragmentByIndex(NextFragmentIndex)->GetBit().GetCharacterMutable());
		}
	}
	
	Fragments.Insert(NewFragment, Index);

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

	//OnReconstructionRequested.ExecuteIfBound();
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

	if (ParentTagContainer.HasTagExact(UFlowYapProjectSettings::Get()->DialogueTagsParent))
	{
		bArg = true;
	}

	bArg = false;
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