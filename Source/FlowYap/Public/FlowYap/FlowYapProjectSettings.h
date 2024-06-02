#pragma once

#include "FlowYapFragmentTimeSettings.h"
#include "GameplayTagContainer.h"
#include "Engine/DeveloperSettings.h"

#include "FlowYapProjectSettings.generated.h"

UCLASS(Config = Game, DefaultConfig, DisplayName = "Yap")
class FLOWYAP_API UFlowYapProjectSettings : public UDeveloperSettings
{
	GENERATED_BODY()
public:
	UFlowYapProjectSettings();

public:
	TMulticastDelegate<void()> OnPortraitKeysChanged;

protected:
	// TODO: editing this needs to tell you that you need to restart the editor somehow. Details customization with a big warning bar in slate surrounding these settings after they're modified??
	UPROPERTY(Config, EditAnywhere, Category = "Settings")
	TArray<FName> PortraitKeys;

	UPROPERTY(Config, EditAnywhere, Category = "Settings")
	FFlowYapFragmentTimeSettings DefaultTimeSettings;

	UPROPERTY(Config, EditAnywhere, Category = "Settings")
	EFlowYapTimedMode AudioTimeFallbackTimedMode;

	UPROPERTY(Config, EditAnywhere, Category = "Settings", meta = (ClampMin = 1, ClampMax = 1000, UIMin = 60, UIMax = 180))
	int32 TextWordsPerMinute;

	UPROPERTY(Config, EditAnywhere, Category = "Settings", meta = (ClampMin = 0.0, UIMin = 0.0, UIMax = 20.0))
	double MinimumAutoTextTimeLength;

	UPROPERTY(Config, EditAnywhere, Category = "Settings", meta = (ClampMin = 0.0, UIMin = 0.0, UIMax = 20.0))
	double MinimumAutoAudioTimeLength;
	
	UPROPERTY(Config, EditAnywhere, Category = "Settings")
	UClass* DialogueAssetClass;
	
#if WITH_EDITORONLY_DATA
protected:
	/** Where to look for portrait key icons. Path should start in the project's root folder, i.e. to use a folder like "...\ProjectName\\Resources\\PortraitKeys", simply type "Resources\\PortraitKeys". If unspecified, will use the "...ProjectName\\Plugins\\FlowYap\\Resources\\PortraitKeys" folder.*/
	UPROPERTY(Config, EditAnywhere, Category = "Settings")
	FDirectoryPath PortraitKeyIconPath;

	UPROPERTY(Config, EditAnywhere, Category = "Settings", meta = (ClampMin = -50, ClampMax = +200, UIMin = -50, UIMax = +200))
	int32 DialogueWidthAdjustment;

	UPROPERTY(Config, EditAnywhere, Category = "Settings")
	bool bHideTitleTextOnNPCDialogueNodes = true;
	
#endif
	
#if WITH_EDITOR
public:
	virtual FName GetCategoryName() const override { return FName("Flow Yap"); }

	virtual FText GetSectionText() const override { return INVTEXT("Settings"); }

	FString GetPortraitIconPath(FName Key) const;

	const TArray<FName>& GetPortraitKeys() const;

	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

public:
	const FFlowYapFragmentTimeSettings& GetDefaultTimeSettings() const;

	EFlowYapTimedMode GetAudioTimeFallbackTimedMode() const;
	
	UClass* GetDialogueAssetClass() const;

	int32 GetDialogueWidthAdjustment() const;

	bool GetHideTitleTextOnNPCDialogueNodes() const;

	int32 GetTextWordsPerMinute() const;

	double GetMinimumAutoTextTimeLength() const;
	
	double GetMinimumAutoAudioTimeLength() const;
};