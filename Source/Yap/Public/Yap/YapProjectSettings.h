#pragma once

#include "YapAudioTimeCacher.h"
#include "YapTimeMode.h"
#include "GameplayTagContainer.h"
#include "Engine/DeveloperSettings.h"

#include "YapProjectSettings.generated.h"

class UYapTextCalculator;
enum class EYapErrorLevel : uint8;

enum class EYap_TagFilter : uint8
{
	Conditions,
	Prompts,
};

UCLASS(Config = Game, DefaultConfig, DisplayName = "Yap")
class YAP_API UYapProjectSettings : public UDeveloperSettings
{
	GENERATED_BODY()
	
public:
	UYapProjectSettings();

	static UYapProjectSettings* Get() { return StaticClass()->GetDefaultObject<UYapProjectSettings>(); }

	// ------------------------------------------
	// SETTINGS
protected:
	/** Time mode to use by default. */
	UPROPERTY(Config, EditAnywhere, Category = "Settings")
	EYapTimeMode DefaultTimeModeSetting;

	/** Controls how missing audio fields are handled.
	 * - OK: Missing audio falls back to using text time.
	 * - Warning: Missing audio falls back to using text time, but nodes show with warnings on Flow.
	 * - Error: Missing audio will not package. */
	UPROPERTY(Config, EditAnywhere, Category = "Settings", meta = (EditCondition = "DefaultTimeModeSetting == EYapTimeMode::AudioTime", EditConditionHides))
	EYapErrorLevel MissingAudioErrorLevel;
	
	/**  */
	UPROPERTY(Config, EditAnywhere, Category = "Settings")
	bool bDefaultInterruptibleSetting;

	UPROPERTY(Config, EditAnywhere, Category = "Settings")
	bool bDisableDefaultFragmentPaddingTime = false;
	
	/** After each dialogue is finished being spoken, a brief extra pause can be inserted before moving onto the next node. */
	UPROPERTY(Config, EditAnywhere, Category = "Settings", meta = (Units = "s", UIMin = 0.0, UIMax = 5.0, Delta = 0.01, EditCondition = "!bDisableDefaultFragmentPaddingTime", EditConditionHides))
	float DefaultFragmentPaddingTime = 0.25f;

	UPROPERTY(Config, EditAnywhere, Category = "Settings", meta = (ClampMin = 1, ClampMax = 1000, UIMin = 60, UIMax = 180, Delta = 5))
	int32 TextWordsPerMinute;

	UPROPERTY(Config, EditAnywhere, Category = "Settings", meta = (ClampMin = 0.0, UIMin = 0.0, UIMax = 20.0, Delta = 0.1))
	double MinimumAutoTextTimeLength;

	/**  */
	UPROPERTY(Config, EditAnywhere, Category = "Settings", meta = (ClampMin = 0.0, UIMin = 0.0, UIMax = 20.0, Delta = 0.1))
	double MinimumAutoAudioTimeLength;

	/** Master minimum time for all fragments ever. Should be set fairly low; intended mostly to only handle accidental "0" time values. */
	UPROPERTY(Config, EditAnywhere, Category = "Settings", meta = (ClampMin = 0.1, UIMin = 0.1, UIMax = 5.0, Delta = 0.01))
	double MinimumFragmentTime;

	// TODO needs to be soft
	UPROPERTY(Config, EditAnywhere, Category = "Settings")
	UClass* DialogueAssetClass;

	UPROPERTY(Config, EditFixedSize, EditAnywhere, Category = "Settings", meta = (ClampMin = 0.1, UIMin = 0.1, UIMax = 5.0, Delta = 0.01))
	float FragmentPaddingSliderMax;

	// TODO needs to be soft
	UPROPERTY(Config, EditAnywhere, Category = "Settings")
	TSubclassOf<UYapTextCalculator> TextCalculator;

	// TODO needs to be soft
	UPROPERTY(Config, EditAnywhere, Category = "Settings")
	TSubclassOf<UYapAudioTimeCacher> AudioTimeCacher;
	
#if WITH_EDITORONLY_DATA
public:
	/** If set, enables nicer filtering of condition tags display */
	UPROPERTY(Config/*, EditAnywhere, Category = "Tags"*/)
	FGameplayTag ConditionTagsParent;

	UPROPERTY(Config, EditAnywhere, Category = "Tags")
	FGameplayTag DialogueTagsParent;

	UPROPERTY(Config, EditAnywhere, Category = "Tags")
	FGameplayTag MoodTagsParent;
		
	TMap<EYap_TagFilter, FGameplayTag*> TagContainers;
	
	TMulticastDelegate<void()> OnMoodTagsChanged;
	
protected:
	/** Where to look for portrait key icons. Path should start in the project's root folder, i.e. to use a folder like "...\ProjectName\\Resources\\MoodKeys", simply type "Resources\\MoodKeys". If unspecified, will use the "...ProjectName\\Plugins\\FlowYap\\Resources\\MoodKeys" folder.*/
	UPROPERTY(Config, EditAnywhere, Category = "Settings")
	FDirectoryPath MoodKeyIconPath;
	
	UPROPERTY(Config, EditAnywhere, Category = "Settings")
	bool bHideTitleTextOnNPCDialogueNodes = true;

	/** Turn off to hide the quick pin-enabling buttons, useful if you want smaller graph nodes, requires graph refresh */
	UPROPERTY(Config, EditAnywhere, Category = "Settings")
	bool bShowPinEnableButtons = true;
	
	// A registered property name (FName) will get bound to a map of classes and the type of tag filter to use for it
	TMultiMap<FName, TMap<UClass*, EYap_TagFilter>> TagFilterSubscriptions;
#endif
	
#if WITH_EDITOR
public:
	virtual FName GetCategoryName() const override { return FName("Flow Yap"); }

	virtual FText GetSectionText() const override { return INVTEXT("Settings"); }

	FString GetPortraitIconPath(FGameplayTag Key) const;

	FGameplayTagContainer GetMoodTags() const;

	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	
	void PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent) override;
#endif

public:
	const EYapTimeMode GetDefaultTimeModeSetting() const { return DefaultTimeModeSetting; }

	bool GetDialogueInterruptibleByDefault() const { return bDefaultInterruptibleSetting; }
	
	UClass* GetDialogueAssetClass() const;

public:
	bool GetHideTitleTextOnNPCDialogueNodes() const;

	int32 GetTextWordsPerMinute() const;

	double GetMinimumAutoTextTimeLength() const;
	
	double GetMinimumAutoAudioTimeLength() const;

	double GetMinimumFragmentTime();

	bool IsDefaultFragmentPaddingTimeDisabled() const { return bDisableDefaultFragmentPaddingTime; }
	
	double GetDefaultFragmentPaddingTime() const { return DefaultFragmentPaddingTime; }
	
	EYapErrorLevel GetMissingAudioErrorLevel() const { return MissingAudioErrorLevel; }

	float GetFragmentPaddingSliderMax() const { return FragmentPaddingSliderMax; }

	TSubclassOf<UYapTextCalculator> GetTextCalculator() const { return TextCalculator; }

	TSubclassOf<UYapAudioTimeCacher> GetAudioTimeCacheClass() const { return AudioTimeCacher; }

#if WITH_EDITOR
public:
	bool ShowPinEnableButtons() const { return bShowPinEnableButtons; }
	
	

	static void RegisterTagFilter(UObject* ClassSource, FName PropertyName, EYap_TagFilter Filter);

	static FString GetTrimmedGameplayTagString(EYap_TagFilter Filter, const FGameplayTag& PropertyTag);

protected:
	void OnGetCategoriesMetaFromPropertyHandle(TSharedPtr<IPropertyHandle> PropertyHandle, FString& MetaString) const;

	#endif
};