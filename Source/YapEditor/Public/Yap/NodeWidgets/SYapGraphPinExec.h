#pragma once

#include "SGraphPin.h"
#include "Graph/Widgets/SFlowGraphNode.h"

class SYapGraphPinExec : public SFlowGraphPinExec
{
public:
	
	void Construct(const FArguments& InArgs, UEdGraphPin* InPin);

	const FSlateBrush* GetPinIcon() const override;

	FSlateBrush Test;
};
