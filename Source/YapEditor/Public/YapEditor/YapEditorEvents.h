// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license.

#pragma once

#define LOCTEXT_NAMESPACE "YapEditor"

#define YAP_EVT_NAME(NAME) inline const FName NAME = "NAME"

namespace YapEditor
{
	namespace Events
	{
		namespace DialogueNode
		{
			YAP_EVT_NAME(Test);
			//FName ChangeDialogueNodeSequencing = "ChangeDialogueNodeSequencing";
		};
	}
}

#undef LOCTEXT_NAMESPACE