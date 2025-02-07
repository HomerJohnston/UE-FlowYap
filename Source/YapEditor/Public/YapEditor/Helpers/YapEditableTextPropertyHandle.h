// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once

#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Engine/GameViewportClient.h"
#include "PropertyHandle.h"
#include "STextPropertyEditableTextBox.h"

struct FYapText;
class UFlowGraphNode;

#define LOCTEXT_NAMESPACE "YapEditor"

/** Allows STextPropertyEditableTextBox to edit a property handle */
class FYapEditableTextPropertyHandle : public IEditableTextProperty
{
private:
	/** The actual text being edited */
	FYapText& Text;

	/** Graph node. Used to find the dialogue node as well as the flow asset. */
	UFlowGraphNode* FlowGraphNode_YapDialogue;

public:
    FYapEditableTextPropertyHandle(FYapText& InText, UFlowGraphNode* InYapDialogueNode);

    bool IsMultiLineText() const override;

    bool IsPassword() const override;

    bool IsReadOnly() const override;

    bool IsDefaultValue() const override;

    FText GetToolTipText() const override;

    int32 GetNumTexts() const override;

    FText GetText(const int32 InIndex) const override;

    void SetText(const int32 InIndex, const FText& InText) override;

    bool IsValidText(const FText& InText, FText& OutErrorMsg) const override;

#if USE_STABLE_LOCALIZATION_KEYS
    void GetStableTextId(const int32 InIndex, const ETextPropertyEditAction InEditAction, const FString& InTextSource, const FString& InProposedNamespace, const FString& InProposedKey, FString& OutStableNamespace, FString& OutStableKey) const override;
#endif // USE_STABLE_LOCALIZATION_KEYS

};

#undef LOCTEXT_NAMESPACE