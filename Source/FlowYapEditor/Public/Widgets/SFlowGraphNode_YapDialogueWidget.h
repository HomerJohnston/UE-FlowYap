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

protected:
	
	TSharedRef<SBox> GetAdditionalOptionsWidget() override;

protected:
	ECheckBoxState GetUserInterruptibleEnabled() const;

	bool GetTimeEntryEnabled() const;

	bool GetUseAutoTimeEnabled() const;

	bool GetUseAudioLengthEnabled() const;

	ECheckBoxState GetTimed() const;
	
	TOptional<double> GetTime() const;
	
	ECheckBoxState GetUseAutoTime() const;

	ECheckBoxState GetUseAudioLength() const;

protected:
	void HandleInterruptibleChanged(ECheckBoxState CheckBoxState);

	void HandleTimedChanged(ECheckBoxState CheckBoxState);

	void HandleTimeChanged(double NewValue, ETextCommit::Type CommitType);

	void HandleUseAutoTimeChanged(ECheckBoxState CheckBoxState);

	void HandleUseAudioLengthChanged(ECheckBoxState CheckBoxState);

	FCheckBoxStyle Style;
};