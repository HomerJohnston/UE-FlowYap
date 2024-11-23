#pragma once

#include "YapEditorSettings.generated.h"

UCLASS(Config = Editor, DefaultConfig, DisplayName = "Flow Yap")
class UYapEditorSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UYapEditorSettings();
	
	static UYapEditorSettings* Get() { return StaticClass()->GetDefaultObject<UYapEditorSettings>(); }

protected:
	UPROPERTY(Config, EditAnywhere, Category = "Settings", meta = (ClampMin = -200, ClampMax = 2000, UIMin = -200, UIMax = 1000, Delta = 10))
	int32 DialogueWidthAdjustment = 0;
		
	UPROPERTY(Config, EditAnywhere, Category = "Settings", meta = (ClampMin = 48, ClampMax = 128, UIMin = 64, UIMax = 128, Multiple = 16))
	int32 PortraitSize = 64;
	
public:
	int32 GetDialogueWidthAdjustment() const { return DialogueWidthAdjustment; };

	int32 GetPortraitSize() const { return PortraitSize; }

public:
	virtual FName GetCategoryName() const override { return FName("Yap"); }
	virtual FText GetSectionText() const override { return INVTEXT("Graph Settings"); }
};
