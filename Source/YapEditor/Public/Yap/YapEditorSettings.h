// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license.

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
	/** Adjusts the width of all dialogue nodes in graph grid units (16 px). */
	UPROPERTY(Config, EditAnywhere, Category = "Settings", meta = (ClampMin = -6, ClampMax = +100, UIMin = -6, UIMax = 20, Delta = 1))
	int32 DialogueWidthAdjustment = 0;

	/** Controls how large the portrait widgets are in the graph. Sizes smaller than 64 will result in some odd slate snapping. */
	UPROPERTY(Config, EditAnywhere, Category = "Settings", meta = (ClampMin = 64, ClampMax = 128, UIMin = 32, UIMax = 128, Multiple = 16))
	int32 PortraitSize = 64;

	/** Controls how bright the portrait borders are in the graph. */
	UPROPERTY(Config, EditAnywhere, Category = "Settings", meta = (ClampMin = 0.0, ClampMax = 1.0, UIMin = 0.0, UIMax = 1.0, Delta = 0.01))
	float PortraitBorderAlpha = 1.0f;

	/** Controls the length of the time progress line on the dialogue widget (left side, for time of the running dialogue). */
	UPROPERTY(Config, EditAnywhere, Category = "Settings", meta = (ClampMin = 0.0, ClampMax = 60.0, UIMin = 0.0, UIMax = 10.0, Delta = 0.01))
	float DialogueTimeSliderMax = 5.0f;
	
	/** Controls the length of the time progress line on the dialogue widget (right side, for delay to next action). */
	UPROPERTY(Config, EditAnywhere, Category = "Settings", meta = (ClampMin = 0.0, ClampMax = 60.0, UIMin = 0.0, UIMax = 10.0, Delta = 0.01))
	float PaddingTimeSliderMax = 2.0f;
	
public:
	int32 GetDialogueWidthAdjustment() const { return DialogueWidthAdjustment; };

	int32 GetPortraitSize() const { return PortraitSize; }

	float GetPortraitBorderAlpha() const { return PortraitBorderAlpha; }

	float GetDialogueTimeSliderMax() const { return DialogueTimeSliderMax; }

	float GetPaddingTimeSliderMax() const { return PaddingTimeSliderMax; }

public:
	virtual FName GetCategoryName() const override { return FName("Yap"); }

	virtual FText GetSectionText() const override { return INVTEXT("Graph Settings"); }
};
