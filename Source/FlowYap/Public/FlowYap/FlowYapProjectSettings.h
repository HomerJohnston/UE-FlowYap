#pragma once

#include "FlowYapAudioTimeCacher.h"
#include "FlowYapTimeMode.h"
#include "Engine/DeveloperSettings.h"

#include "FlowYapProjectSettings.generated.h"

class UFlowYapTextCalculator;

UCLASS(Config = Game, DefaultConfig, DisplayName = "Yap")
class FLOWYAP_API UFlowYapProjectSettings : public UDeveloperSettings
{
	GENERATED_BODY()
public:
	UFlowYapProjectSettings();

public:
	TMulticastDelegate<void()> OnPortraitKeysChanged;

	// ------------------------------------------
	// SETTINGS
protected:
	// TODO: editing this needs to tell you that you need to restart the editor somehow. Details customization with a big warning bar in slate surrounding these settings after they're modified??
	UPROPERTY(Config, EditAnywhere, Category = "Settings")
	TArray<FName> PortraitKeys;

	UPROPERTY(Config, EditAnywhere, Category = "Settings")
	EFlowYapTimeMode DefaultTimeModeSetting;

	UPROPERTY(Config, EditAnywhere, Category = "Settings")
	EFlowYapTimeMode AudioTimeFallbackTimedMode;

	UPROPERTY(Config, EditAnywhere, Category = "Settings")
	bool bDefaultInterruptibleSetting;
	
	/** After each dialogue is finished being spoken, a brief extra pause can be inserted before moving onto the next node. */
	UPROPERTY(Config, EditAnywhere, Category = "Settings", meta = (Units = "s", UIMin = 0, UIMax = 4))
	float DialoguePaddingTime = 0.5f;

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

	UPROPERTY(Config, EditAnywhere, Category = "Settings")
	TSubclassOf<UFlowYapTextCalculator> TextCalculator;
	
	UPROPERTY(Config, EditAnywhere, Category = "Settings")
	TSubclassOf<UFlowYapAudioTimeCacher> AudioTimeCacher;
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
	const EFlowYapTimeMode GetDefaultTimeModeSetting() const { return DefaultTimeModeSetting; }

	bool GetDefaultInterruptibleSetting() const { return bDefaultInterruptibleSetting; }
	
	EFlowYapTimeMode GetMissingAudioFallbackTimeMode() const;
	
	UClass* GetDialogueAssetClass() const;

	int32 GetDialogueWidthAdjustment() const;

	bool GetHideTitleTextOnNPCDialogueNodes() const;

	int32 GetTextWordsPerMinute() const;

	double GetMinimumAutoTextTimeLength() const;
	
	double GetMinimumAutoAudioTimeLength() const;

#if WITH_EDITOR
	TSubclassOf<UFlowYapTextCalculator> GetTextCalculator() const { return TextCalculator; } // TODO should this be available in game runtime?
	
	TSubclassOf<UFlowYapAudioTimeCacher> GetAudioTimeCacheClass() const { return AudioTimeCacher; };
#endif
};