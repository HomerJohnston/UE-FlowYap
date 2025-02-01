// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "YapEditor/GraphNodes/FlowGraphNode_YapDialogue.h"

#include "GameplayTagsEditorModule.h"
#include "Graph/FlowGraph.h"
#include "Graph/FlowGraphEditor.h"
#include "Graph/FlowGraphUtils.h"
#include "UObject/ObjectSaveContext.h"
#include "Yap/YapProjectSettings.h"
#include "Yap/Globals/YapFileUtilities.h"
#include "Yap/Nodes/FlowNode_YapDialogue.h"
#include "YapEditor/YapColors.h"
#include "YapEditor/YapDialogueNodeCommands.h"
#include "YapEditor/YapEditorEventBus.h"
#include "YapEditor/YapEditorEvents.h"
#include "YapEditor/YapLogEditor.h"
#include "YapEditor/YapEditorSubsystem.h"
#include "YapEditor/YapTransactions.h"
#include "YapEditor/Globals/YapTagHelpers.h"
#include "YapEditor/NodeWidgets/SFlowGraphNode_YapDialogueWidget.h"

#define LOCTEXT_NAMESPACE "YapEditor"

UFlowGraphNode_YapDialogue::UFlowGraphNode_YapDialogue()
{
	AssignedNodeClasses = {UFlowNode_YapDialogue::StaticClass()};

	EventBus << YapEditor::Events::DialogueNode::Test;
}

TSharedPtr<SGraphNode> UFlowGraphNode_YapDialogue::CreateVisualWidget()
{
	FYapDialogueNodeCommands::Register();

	Commands = MakeShared<FUICommandList>();
	Commands->MapAction(FYapDialogueNodeCommands::Get().RecalculateText, FExecuteAction::CreateUObject(this, &UFlowGraphNode_YapDialogue::RecalculateTextOnAllFragments));
	
	return SNew(SFlowGraphNode_YapDialogueWidget, this);
}

bool UFlowGraphNode_YapDialogue::ShowPaletteIconOnNode() const
{
	return true;
}

UFlowNode_YapDialogue* UFlowGraphNode_YapDialogue::GetYapDialogueNode() const
{
	return Cast<UFlowNode_YapDialogue>(GetFlowNodeBase());
}

FLinearColor UFlowGraphNode_YapDialogue::GetNodeBodyTintColor() const
{
	return 0.5 * (YapColor::LightGray + YapColor::Gray);
}

FSlateIcon UFlowGraphNode_YapDialogue::GetIconAndTint(FLinearColor& OutColor) const
{
	return FSlateIcon();
}

void UFlowGraphNode_YapDialogue::PreSave(FObjectPreSaveContext SaveContext)
{
	Super::PreSave(SaveContext);

	UFlowNode_YapDialogue* Node = GetYapDialogueNode();

	TMap<FYapFragment*, TArray<FName>> FragmentOptionalPins;
	FragmentOptionalPins.Reserve(Node->GetNumFragments());

	// TODO transaction here
	
	bool bDirty = false;
	
	for (FYapFragment& Fragment : Node->GetFragmentsMutable())
	{
		if (Fragment.UsesStartPin())
		{
			FName PinName = Fragment.GetStartPinName();

			if (Pins.ContainsByPredicate([PinName] (UEdGraphPin* PinEntry) { return !PinEntry->HasAnyConnections() && PinEntry->PinName == PinName; } ))
			{
				Fragment.ResetStartPin();
				bDirty = true;
			}
		}
		
		if (Fragment.UsesEndPin())
		{
			FName PinName = Fragment.GetEndPinName();

			if (Pins.ContainsByPredicate([PinName] (UEdGraphPin* PinEntry) { return !PinEntry->HasAnyConnections() && PinEntry->PinName == PinName; } ))
			{
				Fragment.ResetEndPin();
				bDirty = true;
			}
		}
	}

	if (bDirty)
	{
		ReconstructNode();
	}
}

void UFlowGraphNode_YapDialogue::GetNodeContextMenuActions(class UToolMenu* Menu, class UGraphNodeContextMenuContext* Context) const
{
	Super::GetNodeContextMenuActions(Menu, Context);

	const FYapDialogueNodeCommands& DialogeNodeCommands = FYapDialogueNodeCommands::Get();

	{
		FToolMenuSection& Section = Menu->AddSection("Yap", LOCTEXT("Yap", "Yap"));
		Section.AddMenuEntryWithCommandList(DialogeNodeCommands.RecalculateText, Commands);
	}
}

void UFlowGraphNode_YapDialogue::RecalculateTextOnAllFragments()
{
	FGraphPanelSelectionSet Nodes = FFlowGraphUtils::GetFlowGraphEditor(GetGraph())->GetSelectedNodes();
	
	FYapScopedTransaction T(FName("Default"), FText::Format(LOCTEXT("RecalculateTextLength_Command","Recalculate text length on {0} {0}|plural(one=node,other=nodes)"), Nodes.Num()), nullptr);

	for (UObject* Node : Nodes)
	{
		if (UFlowGraphNode_YapDialogue* DialogeGraphNode = Cast<UFlowGraphNode_YapDialogue>(Node))
		{
			UFlowNode_YapDialogue* DialogueNode2 = DialogeGraphNode->GetYapDialogueNode();
			DialogueNode2->Modify();

			for (FYapFragment& Fragment : DialogueNode2->GetFragmentsMutable())
			{
				// TODO
				//Fragment.GetBitMutable().RecacheSpeakingTime();
			}
		}
	}
}

void UFlowGraphNode_YapDialogue::PostPlacedNewNode()
{
	Super::PostPlacedNewNode();
	
	RandomizeDialogueTag();
	//GenerateFragmentTags();
}

void UFlowGraphNode_YapDialogue::PostPasteNode()
{
	Super::PostPasteNode();

	GetYapDialogueNode()->DialogueTag = FGameplayTag::EmptyTag;
	GetYapDialogueNode()->InvalidateFragmentTags();
	
	RandomizeDialogueTag();
}

void UFlowGraphNode_YapDialogue::RandomizeDialogueTag()
{
	if (IsTemplate())
	{
		return;
	}
	
	if (!GetYapDialogueNode()->GetDialogueTag().IsValid() && true /* // TODO UYapProjectSettings::GetGenerateDialogueNodeTags()*/)
	{
		const UYapBroker* Broker = UYapProjectSettings::GetEditorBrokerDefault();

		FName Tag = Broker->GenerateDialogueNodeTag(GetYapDialogueNode());

		TSharedPtr<FGameplayTagNode> Node = UGameplayTagsManager::Get().FindTagNode(Tag);

		if (!Node)
		{
			IGameplayTagsEditorModule::Get().AddNewGameplayTagToINI(Tag.ToString(), "Auto Generated", Yap::FileUtilities::GetTagConfigFileName());
			Node = UGameplayTagsManager::Get().FindTagNode(Tag);
		}

		if (Node)
		{
			GetYapDialogueNode()->DialogueTag = Node->GetCompleteTag();
		}
	}
}

void UFlowGraphNode_YapDialogue::GenerateFragmentTags()
{
	const UYapBroker* Broker = UYapProjectSettings::GetEditorBrokerDefault();
	
	for (uint8 i = 0; i < GetYapDialogueNode()->GetNumFragments(); ++i)
	{
		GenerateFragmentTag(i);
	}
}

void UFlowGraphNode_YapDialogue::GenerateFragmentTag(uint8 FragmentIndex)
{
	const UYapBroker* Broker = UYapProjectSettings::GetEditorBrokerDefault();
	
	FName Tag;
	
	if (Broker->GenerateFragmentTag(GetYapDialogueNode(), FragmentIndex, Tag))
	{
		TSharedPtr<FGameplayTagNode> Node = UGameplayTagsManager::Get().FindTagNode(Tag);

		if (!Node)
		{
			IGameplayTagsEditorModule::Get().AddNewGameplayTagToINI(Tag.ToString(), "Auto Generated", Yap::FileUtilities::GetTagConfigFileName());
			Node = UGameplayTagsManager::Get().FindTagNode(Tag);
		}

		if (Node)
		{
			GetYapDialogueNode()->GetFragmentsMutable()[FragmentIndex].FragmentTag = Node->GetCompleteTag();
		}
	}
}

void UFlowGraphNode_YapDialogue::AddFragment(int32 InsertionIndex)
{
	if (GetYapDialogueNode()->Fragments.Num() >= 255)
	{
		// TODO nicer logging
		UE_LOG(LogYap, Warning, TEXT("Yap is currently hard-coded to prevent more than 256 fragments per dialogeue node, sorry!"));
		return;
	}

	if (InsertionIndex == INDEX_NONE)
	{
		InsertionIndex = GetYapDialogueNode()->Fragments.Num();
	}

	FYapFragment NewFragment;
	uint8 CopyFragmentIndex = InsertionIndex == 0 ? InsertionIndex : InsertionIndex - 1;

	if (GetYapDialogueNode()->Fragments.IsValidIndex(CopyFragmentIndex))
	{
		const FYapFragment& PreviousFragment = GetYapDialogueNode()->GetFragmentByIndex(CopyFragmentIndex);
		NewFragment.GetBitMutable().SetSpeaker(PreviousFragment.GetBit().GetSpeakerAsset());
		NewFragment.GetBitMutable().SetMoodTag(PreviousFragment.GetBit().GetMoodTag());
	}

	GetYapDialogueNode()->Fragments.Insert(NewFragment, InsertionIndex);

	GetYapDialogueNode()->UpdateFragmentIndices();

	//GetGraphNode()->ReconstructNode(); // TODO This works nicer but crashes because of pin connections. I might not need full reconstruction if I change how my multi-fragment nodes work.
	(void)GetYapDialogueNode()->OnReconstructionRequested.ExecuteIfBound();

	//GenerateFragmentTag(InsertionIndex);
}

#undef LOCTEXT_NAMESPACE
