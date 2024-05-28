#pragma once

#include "FlowNode_YapSpeechBase.h"
#include "FlowYap/FlowYapFragment.h"
#include "FlowNode_YapDialogue.generated.h"

class UFlowYapCharacter;

enum class EFlowNode_YapDialogue_Settings : uint8;

UCLASS(NotBlueprintable, meta = (DisplayName = "Dialogue", Keywords = "event"))
class FLOWYAP_API UFlowNode_YapDialogue : public UFlowNode_YapSpeechBase
{
	GENERATED_BODY()
public:
	UFlowNode_YapDialogue();
};