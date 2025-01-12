// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license.

#pragma once

#include "YapDeveloperSettings.generated.h"

#define LOCTEXT_NAMESPACE "YapEditor"

UCLASS(Config = "Editor")
class UYapDeveloperSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	static UYapDeveloperSettings* Get() { return StaticClass()->GetDefaultObject<UYapDeveloperSettings>(); }

protected:
	UPROPERTY(Config, EditAnywhere, Category = "Settings", meta = (ClampMin = 0.0, ClampMax = 600.0, UIMin = 0.0, UIMax = 600.0, Delta = 10))
	float ConditionDetailsWidth = 400;

	UPROPERTY(Config, EditAnywhere, Category = "Settings", meta = (ClampMin = 0.0, ClampMax = 600.0, UIMin = 0.0, UIMax = 600.0, Delta = 10))
	float ConditionDetailsHeight = 400;

public:
	static float GetConditionDetailsWidth() { return Get()->ConditionDetailsWidth; }
	
	static float GetConditionDetailsHeight() { return Get()->ConditionDetailsHeight; }

public:
	virtual FName GetCategoryName() const override { return FName("Yap"); }

	virtual FText GetSectionText() const override { return LOCTEXT("DeveloperSettings", "Developer Settings"); }
};

#undef LOCTEXT_NAMESPACE