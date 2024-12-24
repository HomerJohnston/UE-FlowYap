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
	UPROPERTY(Config, EditAnywhere, Category = "Settings", meta = (ClampMin = -200, ClampMax = 2000, UIMin = -200, UIMax = 200, Delta = 10))
	int32 DialogueWidthAdjustment = 0;
		
	UPROPERTY(Config, EditAnywhere, Category = "Settings", meta = (ClampMin = 48, ClampMax = 128, UIMin = 64, UIMax = 128, Multiple = 16))
	int32 PortraitSize = 64;

	UPROPERTY(Config, EditAnywhere, Category = "Settings", meta = (ClampMin = 0.0, ClampMax = 1.0, UIMin = 0.0, UIMax = 1.0, Delta = 0.01))
	float PortraitBorderAlpha = 1.0f;

public:
	int32 GetDialogueWidthAdjustment() const { return DialogueWidthAdjustment; };

	int32 GetPortraitSize() const { return PortraitSize; }

	float GetPortraitBorderAlpha() const { return PortraitBorderAlpha; }
	
public:
	virtual FName GetCategoryName() const override { return FName("Yap"); }

	virtual FText GetSectionText() const override { return INVTEXT("Graph Settings"); }
};
