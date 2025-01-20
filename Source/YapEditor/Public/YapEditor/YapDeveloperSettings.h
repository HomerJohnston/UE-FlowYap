﻿// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license.

#pragma once

#include "YapDeveloperSettings.generated.h"

#define LOCTEXT_NAMESPACE "YapEditor"

UCLASS(Config = "Editor")
class UYapDeveloperSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	static UYapDeveloperSettings& Get() { return *StaticClass()->GetDefaultObject<UYapDeveloperSettings>(); }

protected:
	UPROPERTY(Config, EditAnywhere, Category = "Settings", meta = (ClampMin = 0.0, ClampMax = 600.0, UIMin = 0.0, UIMax = 600.0, Delta = 10))
	float ConditionDetailsWidth = 400;

	UPROPERTY(Config, EditAnywhere, Category = "Settings", meta = (ClampMin = 0.0, ClampMax = 600.0, UIMin = 0.0, UIMax = 600.0, Delta = 10))
	float ConditionDetailsHeight = 400;

	/** Controls how bright the portrait borders are in the graph. */
	UPROPERTY(Config, EditAnywhere, Category = "Flow Graph Appearance", meta = (ClampMin = 0.0, ClampMax = 1.0, UIMin = 0.0, UIMax = 1.0, Delta = 0.01))
	float PortraitBorderAlpha = 1.0f;

	/** This is for internal development only. This helps to reduce crashes from reloading slate styles after live coding. */
	UPROPERTY(Config, EditAnywhere, Category = "Development")
	bool bCloseAndReopenAssetsOnLiveCoding = false;

public:
	static float GetConditionDetailsWidth() { return Get().ConditionDetailsWidth; }
	
	static float GetConditionDetailsHeight() { return Get().ConditionDetailsHeight; }

	static float GetPortraitBorderAlpha() { return Get().PortraitBorderAlpha; }

	static bool GetCloseAndReopenAssetsOnLiveCoding() { return Get().bCloseAndReopenAssetsOnLiveCoding; }
	
public:
	FName GetCategoryName() const override { return FName("Yap"); }

	FText GetSectionText() const override { return LOCTEXT("DeveloperSettings", "Developer Settings"); }
	
	FText GetSectionDescription() const override { return LOCTEXT("YapDeveloperSettingsDescription", "Local user settings for Yap"); }
	
};

#undef LOCTEXT_NAMESPACE