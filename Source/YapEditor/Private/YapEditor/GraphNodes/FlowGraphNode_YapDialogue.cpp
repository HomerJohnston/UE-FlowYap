// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "YapEditor/GraphNodes/FlowGraphNode_YapDialogue.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "Graph/FlowGraphEditor.h"
#include "Graph/FlowGraphUtils.h"
#include "UObject/ObjectSaveContext.h"
#include "Yap/YapProjectSettings.h"
#include "Yap/Nodes/FlowNode_YapDialogue.h"
#include "YapEditor/YapColors.h"
#include "YapEditor/YapDialogueNodeCommands.h"
#include "YapEditor/YapEditorEventBus.h"
#include "YapEditor/YapEditorEvents.h"
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
	Commands->MapAction(FYapDialogueNodeCommands::Get().AutoAssignAudio, FExecuteAction::CreateUObject(this, &UFlowGraphNode_YapDialogue::AutoAssignAudioOnAllFragments));
	Commands->MapAction(FYapDialogueNodeCommands::Get().AutoAssignAudioOnAll, FExecuteAction::CreateUObject(this, &UFlowGraphNode_YapDialogue::AutoAssignAudioOnAllNodes));
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
		Section.AddMenuEntryWithCommandList(DialogeNodeCommands.AutoAssignAudio, Commands);
		Section.AddMenuEntryWithCommandList(DialogeNodeCommands.AutoAssignAudioOnAll, Commands);
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
				// Fragment.GetBitMutable().RecacheSpeakingTime();
			}
		}
	}
}

void UFlowGraphNode_YapDialogue::AutoAssignAudioOnAllNodes()
{
	{
		FYapTransactions::BeginModify(INVTEXT("TODO"), GetFlowAsset());

		{
			TArray<UFlowGraphNode_YapDialogue*> Nodes;
			GetFlowAsset()->GetGraph()->GetNodesOfClass(Nodes);

			for (UFlowGraphNode_YapDialogue* Node : Nodes)
			{
				Node->AutoAssignAudioOnAllFragments();
			}
		}
		
		FYapTransactions::EndModify();
	}
}

void UFlowGraphNode_YapDialogue::AutoAssignAudioOnAllFragments()
{
	FYapTransactions::BeginModify(INVTEXT("TODO"), GetFlowAsset());

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::Get().LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	TArray<FAssetData> DependencyAssetData;
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	FString RootFolder = "/Game" / UYapProjectSettings::GetAudioAssetRootFolder();

	if (RootFolder.IsEmpty())
	{
		// TODO log warning
		return;
	}
	
	TArray<FAssetData> AssetDatas;
	AssetRegistry.GetAssetsByPath(FName(RootFolder), AssetDatas, true);

	int32 AudioIDLen = GetYapDialogueNode()->GetAudioID().Len();
	int32 FragmentIDLen = 3; // TODO magic number move this to project settings or some other constant
	
	// Matches AAA-555 and allows for the start or end to be end of string or a non-alphanumeric
	FRegexPattern Regex(FString::Format(TEXT("(^| |[^a-zA-Z\\d\\s:])[a-zA-Z]{{0}}-\\d{{1}}([^a-zA-Z\\d\\s:]| |$)"), {AudioIDLen, FragmentIDLen} ));
	
	// Matches AAA-555 specifically
	FRegexPattern RegexActual(FString::Format(TEXT("[a-zA-Z]{{0}}-\\d{{1}}"), {AudioIDLen, FragmentIDLen}));
	
	TMap<FString, TArray<FAssetData>> AudioAssetData;
	
	for (const FAssetData& AssetData : AssetDatas)
	{
		FString ObjectPathString = AssetData.GetObjectPathString();
		
		FRegexMatcher Matcher(Regex, *ObjectPathString);
		
		if (Matcher.FindNext())
		{
			FRegexMatcher ID(RegexActual, Matcher.GetCaptureGroup(0));

			if (ID.FindNext())
			{
				FString AudioID(ID.GetCaptureGroup(0));

				TArray<FAssetData>& Datas = AudioAssetData.FindOrAdd(AudioID);
				Datas.Add(AssetData);
			}
		}
	}
	
	for (uint8 FragmentIndex = 0; FragmentIndex < GetYapDialogueNode()->GetNumFragments(); ++FragmentIndex)
	{
		FYapFragment& Fragment = GetYapDialogueNode()->Fragments[FragmentIndex];

		FNumberFormattingOptions Args;
		Args.UseGrouping = false;
		Args.MinimumIntegralDigits = 3; // TODO magic number move this to project settings or some other constant
		
		FString AudioID = GetYapDialogueNode()->GetAudioID() + "-" + (FText::AsNumber(FragmentIndex + 1, &Args)).ToString();

		TArray<FAssetData>* Datas = AudioAssetData.Find(AudioID);

		if (Datas)
		{
			UE_LOG(LogYap, Display, TEXT("Assigning X"));

			bool bAssignedMature = false;
			bool bAssignedChildSafe = false;
			
			for (const FAssetData& Data : *Datas)
			{
				FRegexPattern ChildSafe(FString::Format(TEXT("[a-zA-Z]{{0}}-\\d{{1}}-PG"), {AudioIDLen, FragmentIDLen}));
				FRegexMatcher ChildSafeMatch(ChildSafe, Data.GetObjectPathString());

				if (ChildSafeMatch.FindNext())
				{
					if (bAssignedChildSafe)
					{
						UE_LOG(LogYap, Error, TEXT("Found multiple child-safe audio assets for %s"), *AudioID);
					}
					else
					{
						GetYapDialogueNode()->Modify();
						Fragment.Bit.SetSafeDialogueAudioAsset(Data.GetAsset());
						bAssignedChildSafe = true;
					}
				}
				else
				{
					if (bAssignedMature)
					{
						UE_LOG(LogYap, Error, TEXT("Found multiple mature audio assets for %s"), *AudioID);
					}
					else
					{
						GetYapDialogueNode()->Modify();
						Fragment.Bit.SetMatureDialogueAudioAsset(Data.GetAsset());
						bAssignedMature = true;
					}
				}	
			}
		}
	}
	
	FYapTransactions::EndModify();
}

void UFlowGraphNode_YapDialogue::PostPlacedNewNode()
{
	Super::PostPlacedNewNode();
	
	RandomizeAudioID();
}

void UFlowGraphNode_YapDialogue::PostPasteNode()
{
	Super::PostPasteNode();

	GetYapDialogueNode()->DialogueTag = FGameplayTag::EmptyTag;
	GetYapDialogueNode()->InvalidateFragmentTags();
	
	RandomizeAudioID();
}

void UFlowGraphNode_YapDialogue::RandomizeAudioID()
{
	if (IsTemplate())
	{
		return;
	}
	
	if (!GetYapDialogueNode()->GetDialogueTag().IsValid() && true /* // TODO UYapProjectSettings::GetGenerateDialogueNodeTags()*/)
	{
		const UYapBroker* Broker = UYapProjectSettings::GetEditorBrokerDefault();

		FString Tag = Broker->GenerateDialogueAudioID(GetYapDialogueNode());

		GetYapDialogueNode()->AudioID = Tag;
	}
}

void UFlowGraphNode_YapDialogue::AddFragment(int32 InsertionIndex)
{
	if (GetYapDialogueNode()->Fragments.Num() >= 99)
	{
		Yap::Editor::PostNotificationInfo_Warning(LOCTEXT("MaxFragmentLimitWarning_Title", "Maximum fragment limit reached!"), LOCTEXT("MaxFragmentLimitWarning_Description", "There is a limit of 99 fragments per node."));
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
}

inline void UFlowGraphNode_YapDialogue::DestroyNode()
{
	TArray<FGameplayTag> GameplayTags = GatherAllGameplayTags();

	for (FGameplayTag& Tag : GameplayTags)
	{
		UYapEditorSubsystem::AddTagPendingDeletion(Tag);
	}
	
	Super::DestroyNode();
}

TArray<FGameplayTag> UFlowGraphNode_YapDialogue::GatherAllGameplayTags()
{
	TArray<FGameplayTag> GameplayTags;

	const UFlowNode_YapDialogue* Node = GetYapDialogueNode();
	
	if (Node->GetDialogueTag().IsValid())
	{
		GameplayTags.Add(GetYapDialogueNode()->GetDialogueTag());
	}

	for (const FYapFragment& Fragment : Node->GetFragments())
	{
		if (Fragment.GetFragmentTag().IsValid())
		{
			GameplayTags.Add(Fragment.GetFragmentTag());
		}
	}

	return GameplayTags;
}

#undef LOCTEXT_NAMESPACE
