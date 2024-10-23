#pragma once

#include "FlowYapAudioTimeCacher.h"
#include "FlowYapTimeMode.h"
#include "GameplayTagContainer.h"
#include "Engine/DeveloperSettings.h"

#include "FlowYapProjectSettings.generated.h"

class UFlowYapTextCalculator;
enum class EFlowYapErrorLevel : uint8;

enum class EFlowYap_TagFilter : uint8
{
	Conditions,
	Prompts,
};

UCLASS(Config = Game, DefaultConfig, DisplayName = "Yap")
class FLOWYAP_API UFlowYapProjectSettings : public UDeveloperSettings
{
	GENERATED_BODY()
	
public:
	UFlowYapProjectSettings();

	static UFlowYapProjectSettings* Get() { return StaticClass()->GetDefaultObject<UFlowYapProjectSettings>(); }

	// ------------------------------------------
	// SETTINGS
protected:
	// TODO: editing this needs to tell you that you need to restart the editor somehow. Details customization with a big warning bar in slate surrounding these settings after they're modified??
	UPROPERTY(Config, EditAnywhere, Category = "Settings")
	TArray<FGameplayTag> MoodKeys;

	/** Time mode to use by default. */
	UPROPERTY(Config, EditAnywhere, Category = "Settings")
	EFlowYapTimeMode DefaultTimeModeSetting;

	/** Controls how missing audio fields are handled.
	 * - OK: Missing audio falls back to using text time.
	 * - Warning: Missing audio falls back to using text time, but nodes show with warnings on Flow.
	 * - Error: Missing audio will not package. */
	UPROPERTY(Config, EditAnywhere, Category = "Settings", meta = (EditCondition = "DefaultTimeModeSetting == EFlowYapTimeMode::AudioTime", EditConditionHides))
	EFlowYapErrorLevel MissingAudioErrorLevel;
	
	/**  */
	UPROPERTY(Config, EditAnywhere, Category = "Settings")
	bool bDefaultInterruptibleSetting;
	
	/** After each dialogue is finished being spoken, a brief extra pause can be inserted before moving onto the next node. */
	UPROPERTY(Config, EditAnywhere, Category = "Settings", meta = (Units = "s", UIMin = 0, UIMax = 4))
	float FragmentPaddingTime = 0.5f;

	UPROPERTY(Config, EditAnywhere, Category = "Settings", meta = (ClampMin = 1, ClampMax = 1000, UIMin = 60, UIMax = 180))
	int32 TextWordsPerMinute;

	UPROPERTY(Config, EditAnywhere, Category = "Settings", meta = (ClampMin = 0.0, UIMin = 0.0, UIMax = 20.0))
	double MinimumAutoTextTimeLength;

	/**  */
	UPROPERTY(Config, EditAnywhere, Category = "Settings", meta = (ClampMin = 0.0, UIMin = 0.0, UIMax = 20.0))
	double MinimumAutoAudioTimeLength;

	/** Master minimum time for all fragments ever. Should be set fairly low; intended mostly to only handle accidental "0" time values. */
	UPROPERTY(Config, EditAnywhere, Category = "Settings", meta = (ClampMin = 0.1, UIMin = 0.1, UIMax = 20.0))
	double MinimumFragmentTime;
	
	UPROPERTY(Config, EditAnywhere, Category = "Settings")
	UClass* DialogueAssetClass;

	UPROPERTY(Config, EditAnywhere, Category = "Settings")
	TArray<float> CommonFragmentPaddings;

	UPROPERTY(Config, EditFixedSize, EditAnywhere, Category = "Settings")
	float FragmentPaddingSliderMax;

#if WITH_EDITORONLY_DATA
public:
	/** If set, enables nicer filtering of condition tags display */
	UPROPERTY(Config, EditAnywhere)
	FGameplayTag ConditionTagsParent;

	UPROPERTY(Config, EditAnywhere)
	FGameplayTag DialogueTagsParent;

	UPROPERTY(Config, EditAnywhere)
	FGameplayTag MoodTagsParent;
	
	TMap<EFlowYap_TagFilter, FGameplayTag*> TagContainers;
	
	TMulticastDelegate<void()> OnMoodKeysChanged;
	
protected:
	/** Where to look for portrait key icons. Path should start in the project's root folder, i.e. to use a folder like "...\ProjectName\\Resources\\MoodKeys", simply type "Resources\\MoodKeys". If unspecified, will use the "...ProjectName\\Plugins\\FlowYap\\Resources\\MoodKeys" folder.*/
	UPROPERTY(Config, EditAnywhere, Category = "Settings")
	FDirectoryPath MoodKeyIconPath;

	UPROPERTY(Config, EditAnywhere, Category = "Settings", meta = (ClampMin = -200, UIMin = -200, UIMax = 1000))
	int32 DialogueWidthAdjustment;

	UPROPERTY(Config, EditAnywhere, Category = "Settings", meta = (ClampMin = 64, ClampMax = 128, UIMin = 64, UIMax = 128, Multiple = 32))
	int32 PortraitSize = 64;
	
	UPROPERTY(Config, EditAnywhere, Category = "Settings")
	bool bHideTitleTextOnNPCDialogueNodes = true;

	UPROPERTY(Config, EditAnywhere, Category = "Settings")
	TSubclassOf<UFlowYapTextCalculator> TextCalculator;
	
	UPROPERTY(Config, EditAnywhere, Category = "Settings")
	TSubclassOf<UFlowYapAudioTimeCacher> AudioTimeCacher;

	/** Turn off to hide the quick pin-enabling buttons, useful if you want smaller graph nodes, requires graph refresh */
	UPROPERTY(Config, EditAnywhere, Category = "Settings")
	bool bShowPinEnableButtons = true;
	
	// A registered property name (FName) will get bound to a map of classes and the type of tag filter to use for it
	TMultiMap<FName, TMap<UClass*, EFlowYap_TagFilter>> TagFilterSubscriptions;
#endif
	
#if WITH_EDITOR
public:
	virtual FName GetCategoryName() const override { return FName("Flow Yap"); }

	virtual FText GetSectionText() const override { return INVTEXT("Settings"); }

	FString GetPortraitIconPath(FGameplayTag Key) const;

	const TArray<FGameplayTag>& GetMoodKeys() const;

	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	
	void PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent) override;
#endif

public:
	const EFlowYapTimeMode GetDefaultTimeModeSetting() const { return DefaultTimeModeSetting; }

	bool GetDialogueInterruptibleByDefault() const { return bDefaultInterruptibleSetting; }
	
	UClass* GetDialogueAssetClass() const;

	int32 GetDialogueWidthAdjustment() const;

#if WITH_EDITOR
	int32 GetPortraitSize() const { return PortraitSize; }
#endif
	
	bool GetHideTitleTextOnNPCDialogueNodes() const;

	int32 GetTextWordsPerMinute() const;

	double GetMinimumAutoTextTimeLength() const;
	
	double GetMinimumAutoAudioTimeLength() const;

	double GetMinimumFragmentTime();

	double GetFragmentPaddingTime() const { return FragmentPaddingTime; }
	
	EFlowYapErrorLevel GetMissingAudioErrorLevel() const { return MissingAudioErrorLevel; }

	const TArray<float>& GetCommonFragmentPaddings() const { return CommonFragmentPaddings; }

	float GetFragmentPaddingSliderMax() const { return FragmentPaddingSliderMax; }

	bool ShowPinEnableButtons() const { return bShowPinEnableButtons; }
	
#if WITH_EDITOR
public:
	TSubclassOf<UFlowYapTextCalculator> GetTextCalculator() const { return TextCalculator; } // TODO should this be available in game runtime? What if I replace text on a node??? need to recalculate it. So probably yes.
	
	TSubclassOf<UFlowYapAudioTimeCacher> GetAudioTimeCacheClass() const { return AudioTimeCacher; }

	static void RegisterTagFilter(UObject* ClassSource, FName PropertyName, EFlowYap_TagFilter Filter);

	static FString GetTrimmedGameplayTagString(EFlowYap_TagFilter Filter, const FGameplayTag& PropertyTag);

protected:
	void OnGetCategoriesMetaFromPropertyHandle(TSharedPtr<IPropertyHandle> PropertyHandle, FString& MetaString) const;

	#endif
};