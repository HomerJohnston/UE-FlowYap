// Copyright Ghost Pepper Games, Inc. All Rights Reserved.

#include "Yap/Nodes/FlowNode_YapDialogue.h"

#include "GameplayTagsManager.h"
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
	
	MultipleFragmentSequencing = EFlowYapMultipleFragmentSequencing::Sequential;

	Interruptible = EFlowYapInterruptible::UseProjectDefaults;

	// Always have at least one fragment.
	Fragments.Add(FFlowYapFragment());

	InputPins.Empty();
	OutputPins.Empty();

	InputPins.Add(FName("In"));
	OutputPins.Add(FName("Out"));
	OutputPins.Add(FName("Bypass"));

#if WITH_EDITOR
	UFlowYapProjectSettings::RegisterTagFilter(this, GET_MEMBER_NAME_CHECKED(ThisClass, PromptTag), EFlowYap_TagFilter::Prompts);
	
	if (IsTemplate())
	{
		UGameplayTagsManager::Get().OnFilterGameplayTagChildren.AddUObject(this, &ThisClass::OnFilterGameplayTagChildren);
	}
#endif
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
			RunFragmentsSequentiallyFrom(0);
			break;
		}
	case EFlowYapMultipleFragmentSequencing::SelectOne:
		{
			for (uint8 i = 0; i < Fragments.Num(); ++i)
			{
				if (Fragments[i].TryActivate())
				{
					FragmentIndex = i;
					break;
				}
			}
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
		TriggerOutput(FName("FragmentEnd", FragmentIndex + 1), true);
	}
	
#if WITH_EDITOR
	RunningFragmentIndex.Reset();
#endif
}

void UFlowNode_YapDialogue::OnPaddingTimeComplete(uint8 FragmentIndex)
{
	//TriggerOutput(FName("FraEnd", FragmentIndex + 1), true);
}

void UFlowNode_YapDialogue::RunFragmentsSequentiallyFrom(uint8 StartIndex)
{
	for (uint8 i = StartIndex; i < Fragments.Num(); ++i)
	{
		if (Fragments[i].TryActivate())
		{
			FFlowYapFragment& SelectedFragment = Fragments[i];
	
			GetWorld()->GetSubsystem<UFlowYapSubsystem>()->DialogueStart(ConversationName, SelectedFragment.GetBit());

			double Time = SelectedFragment.GetBit().GetTime();

			GetWorld()->GetTimerManager().SetTimer(TimerHandle, FTimerDelegate::CreateUObject(this, &ThisClass::RunFragmentsSequentiallyFrom, (uint8)(i + 1)), Time, false);

			TriggerOutput(FName("DialogueStart", i + 1));

#if WITH_EDITOR
			RunningFragmentIndex = i;
#endif
			break;
		}
	}
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
	return MultipleFragmentSequencing;
}

TArray<FFlowPin> UFlowNode_YapDialogue::GetContextInputs()
{
	return TArray<FFlowPin>();
}

TArray<FFlowPin> UFlowNode_YapDialogue::GetContextOutputs()
{
	if (bIsPlayerPrompt)
	{
		// Leave bypass! No normal out!
		OutputPins.Remove(FName("Out"));
		//OutputPins.Empty();
	}

	TArray<FFlowPin> ContextOutputPins;

	for (uint8 Index = 1; Index <= Fragments.Num(); ++Index) // using 1-based indexing because UE is annoying, FName(X,0) becomes X and FName(X,1) becomes X_0... wtf?
	{
		ContextOutputPins.Add(FName("FragmentStart", Index));
		ContextOutputPins.Add(FName("FragmentEnd", Index));
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

FString UFlowNode_YapDialogue::GetNodeDescription() const
{
	return UFlowYapProjectSettings::GetTrimmedGameplayTagString(EFlowYap_TagFilter::Prompts, PromptTag);
}

void UFlowNode_YapDialogue::OnFilterGameplayTagChildren(const FString& String, TSharedPtr<FGameplayTagNode>& GameplayTagNode, bool& bArg) const
{
	const FGameplayTagContainer& ParentTagContainer = GameplayTagNode->GetParentTagNode()->GetSingleTagContainer();

	if (ParentTagContainer.HasTagExact(UFlowYapProjectSettings::Get()->PromptsContainer))
	{
		bArg = true;
	}

	bArg = false;
	
	UE_LOG(FlowYap, Warning, TEXT("%s"), *GameplayTagNode->GetSimpleTagName().ToString());
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