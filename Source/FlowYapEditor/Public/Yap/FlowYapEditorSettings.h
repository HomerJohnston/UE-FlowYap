#pragma once

#include "FlowYapEditorSettings.generated.h"

UCLASS(Config = Editor, DefaultConfig, DisplayName = "Flow Yap")
class UFlowYapEditorSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UFlowYapEditorSettings();
	
	static UFlowYapEditorSettings* Get() { return StaticClass()->GetDefaultObject<UFlowYapEditorSettings>(); }

protected:
	UPROPERTY(EditAnywhere, Config, Category = "Editor Settings", meta = (ClampMin = 0, UIMin = 0, UIMax = 4))
	uint8 DialogueRowSpacing;

public:
	uint8 GetDialogueRowSpacing() const { return DialogueRowSpacing; }
	
public:
	virtual FName GetCategoryName() const override { return FName("Flow Yap"); }
	virtual FText GetSectionText() const override { return INVTEXT("Graph Settings"); }
};
