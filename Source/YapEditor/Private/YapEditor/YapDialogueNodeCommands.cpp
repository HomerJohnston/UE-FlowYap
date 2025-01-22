﻿// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license.

#include "YapEditor/YapDialogueNodeCommands.h"

#include "Graph/FlowGraphEditor.h"
#include "Graph/FlowGraphUtils.h"
#include "YapEditor/YapLogEditor.h"
#include "YapEditor/GraphNodes/FlowGraphNode_YapDialogue.h"

#define LOCTEXT_NAMESPACE "YapEditor"

FYapDialogueNodeCommands::FYapDialogueNodeCommands()
	: TCommands<FYapDialogueNodeCommands>("Yap", LOCTEXT("TODO", "TODO"), NAME_None, FFlowEditorStyle::GetStyleSetName())
{
}

void FYapDialogueNodeCommands::RegisterCommands()
{
	UI_COMMAND(RecalculateText, "Recalculate Text", "Updates the text time for all fragments of selected nodes", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE
