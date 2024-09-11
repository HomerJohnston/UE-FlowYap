#pragma once

#include "IDetailCustomization.h"

class UFlowNode_YapDialogue;

class FDetailCustomization_FlowYapDialogueNode : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance()
	{
		return MakeShareable(new FDetailCustomization_FlowYapDialogueNode());
	}
	
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

	TWeakObjectPtr<UFlowNode_YapDialogue> Dialogue = nullptr;
	
	void GenerateFragmentArray(TSharedRef<IPropertyHandle> PropertyHandle, int32 ArrayIndex, IDetailChildrenBuilder& ChildrenBuilder);
};
