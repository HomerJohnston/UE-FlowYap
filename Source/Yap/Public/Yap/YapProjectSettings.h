#pragma once

#include "YapAudioTimeCacher.h"
#include "YapTimeMode.h"
#include "GameplayTagContainer.h"
#include "Engine/DeveloperSettings.h"
#include "Yap/YapTextCalculator.h"
#include "YapProjectSettings.generated.h"

class UYapConversationListener;
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

	static UYapProjectSettings* Get()
	{
		return StaticClass()->GetDefaultObject<UYapProjectSettings>();
	}

	// ------------------------------------------
	// SETTINGS
protected:
	
	/** You can point to any class you make for this, but it MUST implement the Yap Conversation Listener interface (C++ IYapConversationListenerInterface). */
	UPROPERTY(Config, EditAnywhere, Category = "Core")
	TSoftClassPtr<UObject> ConversationBrokerClass;

	/** What type of class to use for dialogue assets (sounds). */
	UPROPERTY(Config, EditAnywhere, Category = "Core")
	TSoftClassPtr<UObject> DialogueAssetClass;

	UPROPERTY(Config, EditAnywhere, Category = "Core")
	TSoftClassPtr<UYapAudioTimeCacher> AudioTimeCacherClass;
	
	UPROPERTY(Config, EditAnywhere, Category = "Core")
	TSoftClassPtr<UYapTextCalculator> TextCalculatorClass;

	UPROPERTY(Config, EditAnywhere, Category = "Settings")
	FSlateBrush MissingPortraitBrush;
	
	/** Time mode to use by default. */
	UPROPERTY(Config, EditAnywhere, Category = "Settings")
	EYapTimeMode DefaultTimeModeSetting;

	/** Controls how missing audio fields are handled.
	 * - OK: Missing audio falls back to using text time.
	 * - Warning: Missing audio falls back to using text time, but nodes show with warnings on Flow.
	 * - Error: Missing audio will not package. */ // TODO make it not package
	UPROPERTY(Config, EditAnywhere, Category = "Settings", meta = (EditCondition = "DefaultTimeModeSetting == EYapTimeMode::AudioTime", EditConditionHides))
	EYapErrorLevel MissingAudioErrorLevel;
	
	/** Controls whether dialogue playback can be interrupted (skipped) by default. Can be overridden by individual nodes. */
	UPROPERTY(Config, EditAnywhere, Category = "Settings")
	bool bDefaultSkippableSetting;

	/** Turn this on if you want to completely disable padding time (delays after each fragment of dialogue). */
	UPROPERTY(Config, EditAnywhere, Category = "Settings")
	bool bUseDefaultFragmentPaddingTime = true;
	
	/** After each dialogue is finished being spoken, a brief extra pause can be inserted before moving onto the next node. This is the default value. Can be overridden by individual fragments. */
	UPROPERTY(Config, EditAnywhere, Category = "Settings", meta = (Units = "s", UIMin = 0.0, UIMax = 5.0, Delta = 0.01, EditCondition = "bUseDefaultFragmentPaddingTime", EditConditionHides))
	float DefaultFragmentPaddingTime = 0.25f;

	/** Controls how fast dialogue plays. Only useful for word-based playtime. */ // TODO I need some way for users to overide this within game settings
	UPROPERTY(Config, EditAnywhere, Category = "Settings", meta = (ClampMin = 1, ClampMax = 1000, UIMin = 60, UIMax = 180, Delta = 5))
	int32 TextWordsPerMinute = 120;

	/**  */
	UPROPERTY(Config, EditAnywhere, Category = "Settings")
	bool bCacheFragmentWordCount = true;

	/**  */
	UPROPERTY(Config, EditAnywhere, Category = "Settings")
	bool bCacheFragmentAudioLength = true;
	
	/**  */
	UPROPERTY(Config, EditAnywhere, Category = "Settings", meta = (ClampMin = 0.0, UIMin = 0.0, UIMax = 20.0, Delta = 0.1))
	double MinimumAutoTextTimeLength = 2.0;

	/**  */
	UPROPERTY(Config, EditAnywhere, Category = "Settings", meta = (ClampMin = 0.0, UIMin = 0.0, UIMax = 20.0, Delta = 0.1))
	double MinimumAutoAudioTimeLength = 1.0;

	/** Master minimum time for all fragments ever. Should be set fairly low; intended mostly to only handle accidental "0" time values. */
	UPROPERTY(Config, EditAnywhere, Category = "Settings", meta = (ClampMin = 0.1, UIMin = 0.1, UIMax = 5.0, Delta = 0.01))
	double MinimumFragmentTime = 2.0;

	UPROPERTY(Config, EditFixedSize, EditAnywhere, Category = "Settings", meta = (ClampMin = 0.1, UIMin = 0.1, UIMax = 5.0, Delta = 0.01))
	float FragmentPaddingSliderMax;
	
	UPROPERTY(Config, EditAnywhere, Category = "Tags") // TODO this should all be protected with getters
	FGameplayTag DefaultMoodTag;

	UPROPERTY(Config, EditAnywhere, Category = "Tags") // TODO this should all be protected with getters
	bool bSuppressMatureWarning = false;
	
#if WITH_EDITORONLY_DATA
public:
	/** If set, enables nicer filtering of condition tags display */
	UPROPERTY(Config/*, EditAnywhere, Category = "Tags"*/)
	FGameplayTag ConditionTagsParent;

	UPROPERTY(Config, EditAnywhere, Category = "Tags")
	FGameplayTag DialogueTagsParent;

	UPROPERTY(Config, EditAnywhere, Category = "Tags") // TODO this should all be protected with getters
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

	// ------------------------------------------
	// API
	
#if WITH_EDITOR
public:
	static FName CategoryName;
	
	virtual FName GetCategoryName() const override { return CategoryName; }

	virtual FText GetSectionText() const override { return INVTEXT("Settings"); }

	FString GetPortraitIconPath(FGameplayTag Key) const;

	FGameplayTagContainer GetMoodTags() const;


	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	
	void PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent) override;

	bool GetSuppressMatureWarning() const { return bSuppressMatureWarning; }
#endif

public:
	FGameplayTag GetDefaultMoodTag() const { return DefaultMoodTag; }
	
	const EYapTimeMode GetDefaultTimeModeSetting() const { return DefaultTimeModeSetting; }

	bool GetDialogueSkippableByDefault() const { return bDefaultSkippableSetting; }
	
	TSoftClassPtr<UObject> GetDialogueAssetClass() const { return DialogueAssetClass; }
	
	TSoftClassPtr<UObject>  GetConversationBrokerClass() const { return ConversationBrokerClass; }

public:
	bool GetHideTitleTextOnNPCDialogueNodes() const;

	int32 GetTextWordsPerMinute() const;

	double GetMinimumAutoTextTimeLength() const;
	
	double GetMinimumAutoAudioTimeLength() const;

	double GetMinimumFragmentTime();

	bool IsDefaultFragmentPaddingTimeEnabled() const { return bUseDefaultFragmentPaddingTime; }

	bool CacheFragmentWordCount() const { return bCacheFragmentWordCount; }
	
	bool CacheFragmentAudioLength() const { return bCacheFragmentAudioLength; }
	
	double GetDefaultFragmentPaddingTime() const { return DefaultFragmentPaddingTime; }
	
	EYapErrorLevel GetMissingAudioErrorLevel() const { return MissingAudioErrorLevel; }

	float GetFragmentPaddingSliderMax() const { return FragmentPaddingSliderMax; }

	TSoftClassPtr<UYapTextCalculator> GetTextCalculator() const { return TextCalculatorClass; }

	TSoftClassPtr<UYapAudioTimeCacher> GetAudioTimeCacheClass() const { return AudioTimeCacherClass; }

	FSlateBrush& GetMissingPortraitBrush() { return MissingPortraitBrush; };

#if WITH_EDITOR
public:
	bool ShowPinEnableButtons() const { return bShowPinEnableButtons; }
	
	static void RegisterTagFilter(UObject* ClassSource, FName PropertyName, EYap_TagFilter Filter);

	static FString GetTrimmedGameplayTagString(EYap_TagFilter Filter, const FGameplayTag& PropertyTag);

protected:
	void OnGetCategoriesMetaFromPropertyHandle(TSharedPtr<IPropertyHandle> PropertyHandle, FString& MetaString) const;

	#endif
};