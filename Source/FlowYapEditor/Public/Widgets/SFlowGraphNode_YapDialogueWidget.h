#pragma once

#pragma once

#include "CoreMinimal.h"
#include "SFlowGraphNode_YapSpeechBaseWidget.h"
#include "GraphNodes/FlowGraphNode_YapResponse.h"
#include "Graph/Widgets/SFlowGraphNode.h"
#include "Widgets/SCompoundWidget.h"

class UFlowNode_YapDialogue;
enum class EFlowNode_YapDialogue_Settings : uint8;

class SFlowGraphNode_YapDialogueWidget : public SFlowGraphNode_YapSpeechBaseWidget
{
protected:
	UFlowNode_YapDialogue* FlowNode_YapDialogue = nullptr;

public:
	void Construct(const FArguments& InArgs, UFlowGraphNode* InNode);

public:
	TSharedRef<SBox> GetAdditionalOptionsWidget() override;

	bool GetTimeEntryEnabled() const;

	ECheckBoxState GetTimedEnabled() const;

	bool GetUseAudioLengthEnabled() const;

	ECheckBoxState GetUseAudioLength() const;

	ECheckBoxState GetUserInterruptibleEnabled() const;

	TOptional<double> GetTime() const;
	
	void HandleTimedChanged(ECheckBoxState CheckBoxState);

	void HandleInterruptibleChanged(ECheckBoxState CheckBoxState);

	void HandleTimeChanged(double NewValue, ETextCommit::Type CommitType);

	void HandleUseAudioLengthChanged(ECheckBoxState CheckBoxState);

};