// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "YapEditor/Helpers/YapEditableTextPropertyHandle.h"

#include "FlowAsset.h"
#include "Graph/Nodes/FlowGraphNode.h"
#include "Nodes/FlowNodeBase.h"
#include "Yap/YapText.h"
#include "YapEditor/YapTransactions.h"

#define LOCTEXT_NAMESPACE "YapEditor"

FYapEditableTextPropertyHandle::FYapEditableTextPropertyHandle(FYapText& InText, UFlowGraphNode* InYapDialogueNode)
	: Text(InText)
	, FlowGraphNode_YapDialogue(InYapDialogueNode)
{
}

bool FYapEditableTextPropertyHandle::IsMultiLineText() const
{
	return true;
}

bool FYapEditableTextPropertyHandle::IsPassword() const
{
	return false;
}

bool FYapEditableTextPropertyHandle::IsReadOnly() const
{
	return false; // TODO make dialogue elements lockable?
	//return !PropertyHandle->IsValidHandle() || PropertyHandle->IsEditConst();
}

bool FYapEditableTextPropertyHandle::IsDefaultValue() const
{
	return Text.Get().IsEmpty();
}

FText FYapEditableTextPropertyHandle::GetToolTipText() const
{
	return Text.Get(); // TODO
}

int32 FYapEditableTextPropertyHandle::GetNumTexts() const
{
	return 1;
}

FText FYapEditableTextPropertyHandle::GetText(const int32 InIndex) const
{
	return Text.Get();
}

void FYapEditableTextPropertyHandle::SetText(const int32 InIndex, const FText& InText)
{
	UFlowAsset* Asset = FlowGraphNode_YapDialogue->GetFlowAsset();

	FString AssetName = Asset->GetName();
	
	FYapScopedTransaction Transaction("TODO", LOCTEXT("EditTextProperties", "Edit Text Properties"), FlowGraphNode_YapDialogue->GetFlowNodeBase());
	Text = InText;
}

bool FYapEditableTextPropertyHandle::IsValidText(const FText& InText, FText& OutErrorMsg) const
{
	return true;
}

#if USE_STABLE_LOCALIZATION_KEYS
void FYapEditableTextPropertyHandle::GetStableTextId(const int32 InIndex, const ETextPropertyEditAction InEditAction, const FString& InTextSource, const FString& InProposedNamespace, const FString& InProposedKey, FString& OutStableNamespace, FString& OutStableKey) const
{
	check(InIndex == 0);
	StaticStableTextId(FlowGraphNode_YapDialogue->GetFlowNodeBase(), InEditAction, InTextSource, InProposedNamespace, InProposedKey, OutStableNamespace, OutStableKey);
}
#endif // USE_STABLE_LOCALIZATION_KEYS

#undef LOCTEXT_NAMESPACE