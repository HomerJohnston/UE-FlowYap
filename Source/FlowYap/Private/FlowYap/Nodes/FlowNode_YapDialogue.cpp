#include "FlowYap/Nodes/FlowNode_YapDialogue.h"

#include "AkAudioEvent.h"
#include "FlowYap/FlowYapCharacter.h"
#include "FlowYap/FlowYapLog.h"
#include "FlowYap/FlowYapProjectSettings.h"

UFlowNode_YapDialogue::UFlowNode_YapDialogue()
{
	Category = TEXT("Yap");
	NodeStyle = EFlowNodeStyle::Custom;

	Fragments.Add(FFlowYapFragment());

	InputPins.Empty();
	OutputPins.Empty();
	
	for (int Index = 0; Index <= 9; Index++)
	{
		InputPins.Add(Index);
		OutputPins.Add(Index);
	}

	bMultipleInputs = false;
	bMultipleOutputs = false;
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
		return INVTEXT("NO CHARACTER SET");
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
FFlowYapFragment& UFlowNode_YapDialogue::GetFragment(int64 FragmentID)
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
	
	return FText::Join(FText::FromString(": "), Super::GetNodeTitle(), GetSpeakerName());
}

void UFlowNode_YapDialogue::InitializeInstance()
{
	UE_LOG(FlowYap, Warning, TEXT("InitializeInstance"));

	Super::InitializeInstance();
}

void UFlowNode_YapDialogue::OnActivate()
{
	UE_LOG(FlowYap, Warning, TEXT("OnActivate"));

	Super::OnActivate();
}

void UFlowNode_YapDialogue::ExecuteInput(const FName& PinName)
{
	UE_LOG(FlowYap, Warning, TEXT("ExecuteInput, conversation: %s"), *ConversationName.ToString());

	Super::ExecuteInput(PinName);
}

void UFlowNode_YapDialogue::AddFragment()
{
	Fragments.Add(FFlowYapFragment());

	OnReconstructionRequested.ExecuteIfBound();
}

void UFlowNode_YapDialogue::RemoveFragment(int64 EditorID)
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

void UFlowNode_YapDialogue::ToggleMultipleInputs()
{
	bMultipleInputs = !bMultipleInputs;

	OnReconstructionRequested.ExecuteIfBound();
}

bool UFlowNode_YapDialogue::UsesMultipleInputs()
{
	return bMultipleInputs;
}

void UFlowNode_YapDialogue::ToggleMultipleOutputs()
{
	bMultipleOutputs = !bMultipleOutputs;

	OnReconstructionRequested.ExecuteIfBound();
}

bool UFlowNode_YapDialogue::UsesMultipleOutputs()
{
	return bMultipleOutputs;
}

TArray<FFlowPin> UFlowNode_YapDialogue::GetContextInputs()
{
	InputPins.Empty();

	if (bMultipleInputs)
	{
		TArray<FFlowPin> ContextInputPins;

		uint8 Index = 0;
	
		for (FFlowYapFragment& Fragment : Fragments)
		{
			ContextInputPins.Add(Index++);
		}

		return ContextInputPins;	
	}

	return { 0 };
}

TArray<FFlowPin> UFlowNode_YapDialogue::GetContextOutputs()
{
	OutputPins.Empty();

	if (bMultipleOutputs)
	{
		TArray<FFlowPin> ContextOutputPins;

		uint8 Index = 0;

		for (FFlowYapFragment& Fragment : Fragments)
		{
			ContextOutputPins.Add(Index++);
		}
	
		return ContextOutputPins;
	}

	return { 0 };
}

void UFlowNode_YapDialogue::PostLoad()
{
	Super::PostLoad();
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

