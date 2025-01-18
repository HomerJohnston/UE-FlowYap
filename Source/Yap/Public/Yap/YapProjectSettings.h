// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once

#include "YapTimeMode.h"
#include "GameplayTagContainer.h"
#include "Engine/DeveloperSettings.h"
#include "Yap/YapTextCalculator.h"
#include "YapProjectSettings.generated.h"

class UYapBroker;
enum class EYapMaturitySetting : uint8;
class UYapConversationListener;
enum class EYapMissingAudioErrorLevel : uint8;

#define LOCTEXT_NAMESPACE "Yap"

enum class EYap_TagFilter : uint8
{
	Conditions,
	Prompts,
};

#define YAP_BGETTER(VAR) static bool Get##VAR() { return Get()-> b##VAR; }
#define YAP_GETTER(TYPE, VAR) static TYPE Get##VAR() { return Get()-> VAR; }

UCLASS(Config = Game, DefaultConfig, DisplayName = "Yap")
class YAP_API UYapProjectSettings : public UDeveloperSettings
{
	GENERATED_BODY()

#if WITH_EDITOR
	friend class FDetailCustomization_YapProjectSettings;
#endif
	
public:
	UYapProjectSettings();

	static UYapProjectSettings& Get()
	{
		return *StaticClass()->GetDefaultObject<UYapProjectSettings>();
	}

	// ------------------------------------------
	// SETTINGS
	
protected:
	
	// =================
	// CORE
	
	/** You must create a Yap Broker class (C++ or blueprint) and set it here for Yap to work. */
	UPROPERTY(Config, EditAnywhere, Category = "Core")
	TSoftClassPtr<UYapBroker> BrokerClass;
	
	/** Yap comes with a simple text calculator to determine the word count of dialogue. You can optionally supply your own subclass using different logic here (this may be necessary for languages other than English!).  */
	UPROPERTY(Config, EditAnywhere, Category = "Core")
	TSoftClassPtr<UYapTextCalculator> TextCalculatorClass;
	
	/** What type of classes are allowable to use for dialogue assets (sounds). */
	UPROPERTY(Config, EditAnywhere, Category = "Core", meta = (AllowAbstract))
	TArray<TSoftClassPtr<UObject>> AudioAssetClasses;

	// =================
	// MOOD TAGS

	/**  */
	UPROPERTY(Config, EditAnywhere, Category = "Mood Tags")
	FGameplayTag MoodTagsParent;

	/**  */
	UPROPERTY(Config, EditAnywhere, Category = "Mood Tags")
	FGameplayTag DefaultMoodTag;

	/** Where to look for portrait key icons. Path should start in the project's root folder, i.e. to use a folder like "...\ProjectName\\Resources\\MoodKeys", simply type "Resources\\MoodKeys". If unspecified, will use the "...ProjectName\\Plugins\\FlowYap\\Resources\\MoodKeys" folder.*/
	UPROPERTY(Config, EditAnywhere, Category = "Mood Tags")
	FDirectoryPath MoodTagIconPath;



	
	
	UPROPERTY(Config, EditAnywhere, Category = "Settings")
	FSlateBrush MissingPortraitBrush;
	
	/** Time mode to use by default. */
	UPROPERTY(Config, EditAnywhere, Category = "Dialogue Playback")
	EYapTimeMode DefaultTimeModeSetting;

	/** Controls how missing audio fields are handled.
	 * - OK: Missing audio falls back to using text time without issue.
	 * - Warning: Missing audio falls back to using text time, but nodes show with warnings on Flow Graph, and warning logs on play.
	 * - Error: Missing audio will not pass package validation. */ // TODO make it not package
	UPROPERTY(Config, EditAnywhere, Category = "Settings", DisplayName = "Missing Audio", meta = (EditCondition = "DefaultTimeModeSetting == EYapTimeMode::AudioTime", EditConditionHides))
	EYapMissingAudioErrorLevel MissingAudioErrorLevel;
	
	/** Controls whether dialogue playback can be interrupted (skipped) by default. Can be overridden by individual nodes. */
	UPROPERTY(Config, EditAnywhere, Category = "Dialogue Playback")
	bool bDefaultSkippableSetting;

	/** After each dialogue is finished being spoken, a brief extra pause can be inserted before moving onto the next node. This is the default value. Can be overridden by individual fragments. */
	UPROPERTY(Config, EditAnywhere, Category = "Dialogue Playback", meta = (Units = "s", UIMin = 0.0, UIMax = 5.0, Delta = 0.01, EditCondition = "bUseDefaultFragmentPaddingTime", EditConditionHides))
	float DefaultFragmentPaddingTime = 0.25f;

	/** Controls how fast dialogue plays. Only useful for word-based playtime. */ // TODO I need some way for users to overide this within game settings
	UPROPERTY(Config, EditAnywhere, Category = "Dialogue Playback", meta = (ClampMin = 1, ClampMax = 1000, UIMin = 60, UIMax = 180, Delta = 5))
	int32 TextWordsPerMinute = 120;

	/**  */
	UPROPERTY(Config, EditAnywhere, Category = "Settings")
	bool bCacheFragmentWordCount = true;

	/**  */
	UPROPERTY(Config, EditAnywhere, Category = "Settings")
	bool bCacheFragmentAudioLength = true;
	
	/**  */
	UPROPERTY(Config, EditAnywhere, Category = "Dialogue Playback", meta = (ClampMin = 0.0, UIMin = 0.0, UIMax = 20.0, Delta = 0.1))
	double MinimumAutoTextTimeLength = 1.5;

	/**  */
	UPROPERTY(Config, EditAnywhere, Category = "Dialogue Playback", meta = (ClampMin = 0.0, UIMin = 0.0, UIMax = 20.0, Delta = 0.1))
	double MinimumAutoAudioTimeLength = 0.5;

	/** Master minimum time for all fragments ever. Should be set fairly low; intended mostly to only handle accidental "0" time values. */
	UPROPERTY(Config, EditAnywhere, Category = "Dialogue Playback", meta = (ClampMin = 0.1, UIMin = 0.1, UIMax = 5.0, Delta = 0.01))
	double MinimumFragmentTime = 2.0;

	UPROPERTY(Config, EditFixedSize, EditAnywhere, Category = "Dialogue Playback", meta = (ClampMin = 0.1, UIMin = 0.1, UIMax = 5.0, Delta = 0.01))
	float FragmentPaddingSliderMax;

	/**  */
	UPROPERTY(Config, EditAnywhere, Category = "Settings")
	bool bSuppressDefaultMatureWarning = false;

	UPROPERTY(Config, EditAnywhere, Category = "Settings")
	EYapMaturitySetting DefaultMaturitySetting;

	// ============================================================================================
	// EDITOR SETTINGS
#if WITH_EDITORONLY_DATA
	/** If set, enables nicer filtering of condition tags display */
	UPROPERTY(Config/*, EditAnywhere, Category = "Tags"*/)
	FGameplayTag ConditionTagsParent;

	UPROPERTY(Config, EditAnywhere, Category = "Dialogue Tags")
	FGameplayTag DialogueTagsParent;

	// ============================================================================================
	// STATE
public:
	TMulticastDelegate<void()> OnMoodTagsChanged;

protected:
	TMap<EYap_TagFilter, FGameplayTag*> TagContainers;
	
	/** If enabled, will show title text on normal talk nodes as well as player prompt nodes. */
	UPROPERTY(Config, EditAnywhere, Category = "Settings")
	bool bShowTitleTextOnTalkNodes = false;

	/** Turn off to hide the On Start / On End pin-buttons, useful if you want a simpler graph without these features. */
	UPROPERTY(Config, EditAnywhere, Category = "Settings")
	bool bShowPinEnableButtons = true;
	
	// A registered property name (FName) will get bound to a map of classes and the type of tag filter to use for it
	TMultiMap<FName, TMap<UClass*, EYap_TagFilter>> TagFilterSubscriptions;

	/**  */
	UPROPERTY(Config, EditAnywhere, Category = "Settings")
	FString DefaultTextNamespace = "Yap";

	/** Adjusts the width of all dialogue nodes in graph grid units (16 px). */
	UPROPERTY(Config, EditAnywhere, Category = "Flow Graph Appearance", meta = (ClampMin = -6, ClampMax = +100, UIMin = -6, UIMax = 20, Delta = 1))
	int32 DialogueWidthAdjustment = 0;

	/** Controls how large the portrait widgets are in the graph. Sizes smaller than 64 will result in some odd slate snapping. */
	UPROPERTY(Config, EditAnywhere, Category = "Flow Graph Appearance", meta = (ClampMin = 64, ClampMax = 128, UIMin = 32, UIMax = 128, Multiple = 16))
	int32 PortraitSize = 64;

	/** Controls the length of the time progress line on the dialogue widget (left side, for time of the running dialogue). */
	UPROPERTY(Config, EditAnywhere, Category = "Flow Graph Appearance", meta = (ClampMin = 0.0, ClampMax = 60.0, UIMin = 0.0, UIMax = 10.0, Delta = 0.01))
	float DialogueTimeSliderMax = 5.0f;
	
	/** Controls the length of the time progress line on the dialogue widget (right side, for delay to next action). */
	UPROPERTY(Config, EditAnywhere, Category = "Flow Graph Appearance", meta = (ClampMin = 0.0, ClampMax = 60.0, UIMin = 0.0, UIMax = 10.0, Delta = 0.01))
	float PaddingTimeSliderMax = 2.0f;
	
#endif
	

	// ------------------------------------------
	// API
	
#if WITH_EDITOR
public:
	static FName CategoryName;
	
	FName GetCategoryName() const override { return CategoryName; }

	FText GetSectionText() const override { return LOCTEXT("Settings", "Settings"); }
	
	FText GetSectionDescription() const override { return LOCTEXT("YapProjectSettingsDescription", "Project-specific settings for Yap"); }
	
	static FString GetMoodKeyIconPath(FGameplayTag Key, FString FileExtension);

	static const FGameplayTag& GetMoodTagsParent() { return Get().MoodTagsParent; }
	
	static const FGameplayTag& GetDialogueTagsParent() { return Get().DialogueTagsParent; };
	
	static FGameplayTagContainer GetMoodTags();
	
	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	
	void PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent) override;

	static bool GetSuppressDefaultMatureWarning() { return Get().bSuppressDefaultMatureWarning; }
#endif

public:
	static FGameplayTag GetDefaultMoodTag() { return Get().DefaultMoodTag; }
	
	static EYapTimeMode GetDefaultTimeModeSetting() { return Get().DefaultTimeModeSetting; }

	static bool GetDialogueSkippableByDefault() { return Get().bDefaultSkippableSetting; }
	
	static EYapMaturitySetting GetDefaultMaturitySetting() { return Get().DefaultMaturitySetting; }
	
public:
	static const TSoftClassPtr<UYapBroker>& GetConversationBrokerClass() { return Get().BrokerClass; }
	
	static const TArray<TSoftClassPtr<UObject>>& GetDialogueAssetClasses() { return Get().AudioAssetClasses; }

	static bool GetShowTitleTextOnTalkNodes() { return Get().bShowTitleTextOnTalkNodes; }

	static int32 GetTextWordsPerMinute() { return Get().TextWordsPerMinute; }

	static double GetMinimumAutoTextTimeLength() { return Get().MinimumAutoTextTimeLength; };
	
	static double GetMinimumAutoAudioTimeLength() { return Get().MinimumAutoAudioTimeLength; }
	
	static double GetMinimumFragmentTime() { return Get().MinimumFragmentTime; }

	static bool CacheFragmentWordCount() { return Get().bCacheFragmentWordCount; }
	
	static bool CacheFragmentAudioLength() { return Get().bCacheFragmentAudioLength; }
	
	static double GetDefaultFragmentPaddingTime() { return Get().DefaultFragmentPaddingTime; }
	
	static EYapMissingAudioErrorLevel GetMissingAudioBehavior() { return Get().MissingAudioErrorLevel; }

	static TSoftClassPtr<UYapTextCalculator> GetTextCalculator() { return Get().TextCalculatorClass; }

	static const FSlateBrush& GetMissingPortraitBrush() { return Get().MissingPortraitBrush; };

	static const FString& GetMoodKeyIconPath();
	
#if WITH_EDITOR
public:
	static int32 GetDialogueWidthAdjustment() { return Get().DialogueWidthAdjustment; };

	static int32 GetPortraitSize() { return Get().PortraitSize; }


	static float GetDialogueTimeSliderMax() { return Get().DialogueTimeSliderMax; }

	static float GetFragmentPaddingSliderMax() { return Get().PaddingTimeSliderMax; }

public:
	static bool ShowPinEnableButtons()  { return Get().bShowPinEnableButtons; }
	
	static void RegisterTagFilter(UObject* ClassSource, FName PropertyName, EYap_TagFilter Filter);

	static FString GetTrimmedGameplayTagString(EYap_TagFilter Filter, const FGameplayTag& PropertyTag);

protected:
	void OnGetCategoriesMetaFromPropertyHandle(TSharedPtr<IPropertyHandle> PropertyHandle, FString& MetaString) const;

	#endif
};

#undef LOCTEXT_NAMESPACE