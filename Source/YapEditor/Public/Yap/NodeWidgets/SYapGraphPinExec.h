// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license.

#pragma once

#include "Graph/Widgets/SFlowGraphNode.h"

class SYapGraphPinExec : public SFlowGraphPinExec
{
public:
	
	void Construct(const FArguments& InArgs, UEdGraphPin* InPin);

	const FSlateBrush* GetPinIcon() const override;

	FSlateBrush Test;
};
