// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "Yap/Nodes/FlowNode_YapDialogue.h"

#include "GameplayTagsManager.h"
#include "GameplayTagsModule.h"
#include "Yap/YapBit.h"
#include "Yap/YapCondition.h"
#include "Yap/YapFragment.h"
#include "Yap/YapProjectSettings.h"
#include "Yap/YapSubsystem.h"
#include "Yap/Enums/YapLoadContext.h"

#define LOCTEXT_NAMESPACE "Yap"

FName UFlowNode_YapDialogue::OutputPinName = FName("Out");
FName UFlowNode_YapDialogue::BypassPinName = FName("Bypass");

UFlowNode_YapDialogue::UFlowNode_YapDialogue()
{
#if WITH_EDITOR
	Category = TEXT("Yap");

	NodeStyle = EFlowNodeStyle::Custom;
#endif

	DialogueNodeType = EYapDialogueNodeType::Talk;
	
	NodeActivationLimit = 0;
	
	TalkSequencing = EYapDialogueTalkSequencing::RunAll;

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

bool UFlowNode_YapDialogue::Skip(int32 FragmentIndex)
{
	if (FragmentAwaitingManualAdvance == FragmentIndex)
	{
		AdvanceToNextFragment(FragmentIndex);
		return true;
	}
	
	bool bPreventSkippingTimers = !GetFragmentByIndex(FragmentIndex).GetSkippable(this->GetSkippable());
			
	if (bPreventSkippingTimers && (FragmentTimerHandle.IsValid() || PaddingTimerHandle.IsValid()))
	{
		return false;
	}

	bool bSkipped = false;
	
	if (FragmentTimerHandle.IsValid())
	{
		OnSpeakingComplete(FragmentIndex);
		bSkipped = true;
	}

	if (PaddingTimerHandle.IsValid())
	{
		OnPaddingComplete(FragmentIndex);
		bSkipped = true;
	}

	if (!bSkipped)
	{
		return false;
	}
	
	if (FragmentAwaitingManualAdvance == FragmentIndex)
	{
		AdvanceToNextFragment(FragmentIndex);
	}

	return true;
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
	
	TriggerPreload();
}

void UFlowNode_YapDialogue::ExecuteInput(const FName& PinName)
{
#if WITH_EDITOR
	FinishedFragments.Empty();
	RunningFragment = nullptr;
#endif

	if (!CheckConditions())
	{
		TriggerOutput("Bypass", true, EFlowPinActivationType::Default);
		return;
	}
	
	if (ActivationLimitsMet())
	{
		TriggerOutput("Bypass", true, EFlowPinActivationType::Default);
		return;
	}

	if (IsPlayerPrompt())
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
	if (IsPlayerPrompt())
	{
		TriggerOutput("Bypass", true, EFlowPinActivationType::PassThrough);
	}
	else
	{
		TriggerOutput("Out", true, EFlowPinActivationType::PassThrough);
	}
}

bool UFlowNode_YapDialogue::CheckConditions()
{
	for (UYapCondition* Condition : Conditions)
	{
		if (!IsValid(Condition))
		{
			UE_LOG(LogYap, Warning, TEXT("Ignoring null condition. Clean this up!")); // TODO more info
			continue;
		}

		if (!Condition->EvaluateCondition_Internal())
		{
			return false;
		}
	}
	
	return true;
}

bool UFlowNode_YapDialogue::UsesTitleText() const
{
	return IsPlayerPrompt() || UYapProjectSettings::GetShowTitleTextOnTalkNodes();
}

bool UFlowNode_YapDialogue::GetSkippable() const
{
	return Skippable.Get(UYapProjectSettings::GetDefaultSkippableSetting());
}

bool UFlowNode_YapDialogue::GetAutoAdvance() const
{
	return AutoAdvance.Get(UYapProjectSettings::GetDefaultAutoAdvanceSetting());
}

#if WITH_EDITOR
TOptional<bool> UFlowNode_YapDialogue::GetSkippableSetting() const
{
	return Skippable;
}
#endif

// ================================================================================================

#if WITH_EDITOR
void UFlowNode_YapDialogue::InvalidateFragmentTags()
{
	for (uint8 FragmentIndex = 0; FragmentIndex < Fragments.Num(); ++FragmentIndex)
	{
		FYapFragment& Fragment = Fragments[FragmentIndex];

		Fragment.InvalidateFragmentTag(this);
	}
}
#endif

void UFlowNode_YapDialogue::BroadcastPrompts()
{
	TArray<uint8> BroadcastedFragments;
	FYapPromptHandle LastHandle;
	
 	for (uint8 FragmentIndex = 0; FragmentIndex < Fragments.Num(); ++FragmentIndex)
	{
		FYapFragment& Fragment = Fragments[FragmentIndex];

 		if (!Fragment.CheckConditions())
 		{
 			continue;
 		}
 		
		if (Fragment.IsActivationLimitMet())
		{
			continue;
		}

		LastHandle = GetWorld()->GetSubsystem<UYapSubsystem>()->BroadcastPrompt(this, FragmentIndex);
 		
		BroadcastedFragments.Add(FragmentIndex);
	}

	GetWorld()->GetSubsystem<UYapSubsystem>()->OnFinishedBroadcastingPrompts();

	if (BroadcastedFragments.Num() == 0)
	{
		TriggerOutput(BypassPinName, true);
	}
	else if (BroadcastedFragments.Num() == 1)
	{
		if (UYapProjectSettings::GetAutoSelectLastPromptSetting())
		{
			LastHandle.RunPrompt(this);
		}
	}
}

void UFlowNode_YapDialogue::RunPrompt(uint8 FragmentIndex)
{
	if (!RunFragment(FragmentIndex))
	{
		// TODO log error? This should never happen?
		
		TriggerOutput(BypassPinName, true);
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
		TriggerOutput(BypassPinName, true);
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
		RunningFragment = &Fragment;
		FragmentStartedTime = GetWorld()->GetTimeSeconds();
#endif
		RunningFragmentIndex = FragmentIndex;

		if (Fragment.UsesStartPin())
		{
			const FFlowPin StartPin = Fragment.GetStartPin();
			TriggerOutput(StartPin.PinName, false);
		}

		TOptional<float> Time = Fragment.GetTime();

		if (!Time.IsSet())
		{
			// We do nothing! This dialogue can only be advanced by using the Dialogue Handle to skip the dialogue.
		}
		else
		{
			GetWorld()->GetTimerManager().SetTimer(FragmentTimerHandle, FTimerDelegate::CreateUObject(this, &ThisClass::OnSpeakingComplete, FragmentIndex), Time.GetValue(), false);
		}

		return true;
	}
	else
	{
		return false;
	}
}

void UFlowNode_YapDialogue::OnSpeakingComplete(uint8 FragmentIndex)
{
	FTimerManager& TimerManager = GetWorld()->GetTimerManager();

	if (TimerManager.TimerExists(FragmentTimerHandle))
	{
		TimerManager.ClearTimer(FragmentTimerHandle);
	}
	
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
		TimerManager.SetTimer(PaddingTimerHandle, FTimerDelegate::CreateUObject(this, &ThisClass::OnPaddingComplete, FragmentIndex), PaddingTime, false);
	}
	else
	{
		OnPaddingComplete(FragmentIndex);
	}
	
#if WITH_EDITOR
	FragmentEndedTime = GetWorld()->GetTimeSeconds();
#endif
}

void UFlowNode_YapDialogue::OnPaddingComplete(uint8 FragmentIndex)
{
	FTimerManager& TimerManager = GetWorld()->GetTimerManager();

	if (TimerManager.TimerExists(PaddingTimerHandle))
	{
		TimerManager.ClearTimer(PaddingTimerHandle);
	}
	
#if WITH_EDITOR
	FinishedFragments.Add(RunningFragment);
	RunningFragment = nullptr;
#endif

	RunningFragmentIndex = INDEX_NONE;

	GetWorld()->GetSubsystem<UYapSubsystem>()->BroadcastPaddingTimeOver(this, FragmentIndex);
	
	FYapFragment& Fragment = Fragments[FragmentIndex];
	
	if (Fragment.GetAutoAdvance(this->GetAutoAdvance()))
	{
		AdvanceToNextFragment(FragmentIndex);
	}
	else
	{
		FragmentAwaitingManualAdvance = FragmentIndex;
	}
}

void UFlowNode_YapDialogue::AdvanceToNextFragment(uint8 CurrentFragmentIndex)
{
	FYapFragment& Fragment = Fragments[CurrentFragmentIndex];

	if (IsPlayerPrompt())
	{
		TriggerOutput(Fragment.GetPromptPin().PinName, true);
	}
	else
	{
		if (TalkSequencing == EYapDialogueTalkSequencing::SelectOne)
		{
			TriggerOutput(OutputPinName, true);
		}
		else
		{
			for (uint8 NextIndex = CurrentFragmentIndex + 1; NextIndex < Fragments.Num(); ++NextIndex)
			{
				bool bRanNextFragment =  RunFragment(NextIndex);

				if (!bRanNextFragment && TalkSequencing == EYapDialogueTalkSequencing::RunUntilFailure)
				{
					// Whoops, this is the end of the line
					TriggerOutput(OutputPinName, true);
					return;
				}
				else if (bRanNextFragment)
				{
					// We'll delegate further behavior to the next running fragment
					return;
				}
			}

			// No more fragments to try and run!
			TriggerOutput(OutputPinName, true);
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
	TArray<FFlowPin> ContextOutputPins = Super::GetContextOutputs();

	if (!IsPlayerPrompt())
	{
		ContextOutputPins.Add(OutputPinName);
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

		if (IsPlayerPrompt())
		{
			ContextOutputPins.Add(Fragment.GetPromptPin());
		}
	}

	if (IsBypassPinRequired())
	{
		ContextOutputPins.Add(BypassPinName);
	}
	
	return ContextOutputPins;
}

void UFlowNode_YapDialogue::SetNodeActivationLimit(int32 NewValue)
{
	bool bBypassRequired = IsBypassPinRequired();
	
	NodeActivationLimit = NewValue;

	if (bBypassRequired != IsBypassPinRequired())
	{
		(void)OnReconstructionRequested.ExecuteIfBound();
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
	
	(void)OnReconstructionRequested.ExecuteIfBound();
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

	(void)OnReconstructionRequested.ExecuteIfBound();
}

FString UFlowNode_YapDialogue::GetNodeDescription() const
{
	return "";
}

void UFlowNode_YapDialogue::OnFilterGameplayTagChildren(const FString& String, TSharedPtr<FGameplayTagNode>& GameplayTagNode, bool& bArg) const
{
	if (GameplayTagNode == nullptr)
	{
		bArg = false;
		return;
	}

	TSharedPtr<FGameplayTagNode> ParentTagNode = GameplayTagNode->GetParentTagNode();

	if (ParentTagNode == nullptr)
	{
		bArg = false;
		return;
	}
	
	const FGameplayTagContainer& ParentTagContainer = ParentTagNode->GetSingleTagContainer();

	if (ParentTagContainer.HasTagExact(UYapProjectSettings::GetDialogueTagsParent()))
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

EYapFragmentState UFlowNode_YapDialogue::GetFragmentState(int32 FragmentIndex) const
{
	if (!Fragments.IsValidIndex(FragmentIndex))
	{
		return EYapFragmentState::Undefined;
	}

	if (RunningFragmentIndex == FragmentIndex)
	{
		if (PaddingTimerHandle.IsValid())
		{
			return EYapFragmentState::InPadding;
		}
		
		return EYapFragmentState::Running;
	}

	return EYapFragmentState::Idle;
}

#if WITH_EDITOR
void UFlowNode_YapDialogue::ToggleNodeType()
{
	uint8 AsInt = static_cast<uint8>(DialogueNodeType);

	if (++AsInt >= static_cast<uint8>(EYapDialogueNodeType::COUNT))
	{
		AsInt = 0;
	}

	DialogueNodeType = static_cast<EYapDialogueNodeType>(AsInt);
}
#endif

#if WITH_EDITOR
void UFlowNode_YapDialogue::ForceReconstruction()
{
	(void)OnReconstructionRequested.ExecuteIfBound();
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

void UFlowNode_YapDialogue::PostLoad()
{
	Super::PostLoad();
	
	TriggerPreload();
}

void UFlowNode_YapDialogue::PreloadContent()
{
	UWorld* World = GetWorld();

	EYapLoadContext LoadContext = EYapLoadContext::Async;
	EYapMaturitySetting MaturitySetting = EYapMaturitySetting::Unspecified;

#if WITH_EDITOR
	if (!World || (World->WorldType != EWorldType::Game && World->WorldType != EWorldType::PIE && World->WorldType != EWorldType::GamePreview))
	{
		LoadContext = EYapLoadContext::AsyncEditorOnly;
	}
#endif

	for (FYapFragment& Fragment : Fragments)
	{
		Fragment.PreloadContent(MaturitySetting, LoadContext);
	}
}
#endif

#undef LOCTEXT_NAMESPACE