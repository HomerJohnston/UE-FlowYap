// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once

#include "YapTimeMode.h"
#include "GameplayTagContainer.h"
#include "YapLog.h"
#include "Engine/DeveloperSettings.h"
#include "Yap/YapBroker.h"
#include "YapProjectSettings.generated.h"

enum class EYapDialogueSkippable : uint8;
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
	
	// ============================================================================================
	// CONSTRUCTION
	// ============================================================================================
public:
	UYapProjectSettings();

protected:
	static UYapProjectSettings& Get()
	{
		return *StaticClass()->GetDefaultObject<UYapProjectSettings>();
	}

	// ============================================================================================
	// SETTINGS
	// ============================================================================================
protected:
	
	// - - - - - CORE - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	
	/** You must create a Yap Broker class (C++ or blueprint) and set it here for Yap to work. */
	UPROPERTY(Config, EditAnywhere, Category = "Core")
	TSoftClassPtr<UYapBroker> BrokerClass = nullptr;
	
	/** What type of classes are allowable to use for dialogue assets (sounds). If unset, defaults to Unreal's USoundBase. */
	UPROPERTY(Config, EditAnywhere, Category = "Core", meta = (AllowAbstract))
	TArray<TSoftClassPtr<UObject>> OverrideAudioAssetClasses;

#if WITH_EDITORONLY_DATA
	// Do not expose this for editing; only hard-coded
	UPROPERTY() 
	TArray<TSoftClassPtr<UObject>> DefaultAssetAudioClasses;
#endif
	
	// - - - - - MOOD TAGS - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

	/** Parent tag to use for mood tags. All sub-tags of this parent will be used as mood tags! */
	UPROPERTY(Config, EditAnywhere, Category = "Mood Tags")
	FGameplayTag MoodTagsParent;

	/** Optional default mood tag to use, for dialogue fragments which do not have a mood tag set. */
	UPROPERTY(Config, EditAnywhere, Category = "Mood Tags")
	FGameplayTag DefaultMoodTag;

	/** Where to look for portrait key icons. If unspecified, will use the default "Plugins/FlowYap/Resources/MoodKeys" folder.*/
	UPROPERTY(Config, EditAnywhere, Category = "Mood Tags")
	FDirectoryPath MoodTagIconPath;

	// - - - - - DIALOGUE PLAYBACK - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  
	
	/** Time mode to use by default. */
	UPROPERTY(Config, EditAnywhere, Category = "Dialogue Playback")
	EYapTimeMode DefaultTimeModeSetting;

	/** Controls whether dialogue playback can be interrupted (skipped) by default. Can be overridden by individual nodes. */
	UPROPERTY(Config, EditAnywhere, Category = "Dialogue Playback", meta = (ValidEnumValues = "Skippable, NotSkippable"))
	EYapDialogueSkippable DefaultSkippableSetting;

	/** After each dialogue is finished being spoken, a brief extra pause can be inserted before moving onto the next node. This is the default value. Can be overridden by individual fragments. */
	UPROPERTY(Config, EditAnywhere, Category = "Dialogue Playback", meta = (Units = "s", UIMin = 0.0, UIMax = 5.0, Delta = 0.01))
	float DefaultFragmentPaddingTime = 0.25f;

	/** Controls how fast dialogue plays. Only useful for word-based playtime. */
	UPROPERTY(Config, EditAnywhere, Category = "Dialogue Playback", meta = (ClampMin = 1, ClampMax = 1000, UIMin = 60, UIMax = 180, Delta = 5))
	int32 TextWordsPerMinute = 120;

	/** When speaking time is calculated from text, this sets the minimum speaking time. */
	UPROPERTY(Config, EditAnywhere, Category = "Dialogue Playback", meta = (ClampMin = 0.0, UIMin = 0.0, UIMax = 5.0, Delta = 0.1))
	double MinimumAutoTextTimeLength = 1.0;

	/** When speaking time is calculated from the length of an audio asset, this sets the minimum speaking time. */
	UPROPERTY(Config, EditAnywhere, Category = "Dialogue Playback", meta = (ClampMin = 0.0, UIMin = 0.0, UIMax = 5.0, Delta = 0.1))
	double MinimumAutoAudioTimeLength = 0.5;

	/** Total minimum speaking time (overrides both auto length minimums above). Should be fairly low; intended mostly to only handle accidental "0" time values. */
	UPROPERTY(Config, EditAnywhere, Category = "Dialogue Playback", meta = (ClampMin = 0.1, UIMin = 0.1, UIMax = 5.0, Delta = 0.01))
	double MinimumFragmentTime = 0.25;

	/** Controls the scaling of the small padding time indicator on each fragment. */
	UPROPERTY(Config, EditFixedSize, EditAnywhere, Category = "Dialogue Playback", meta = (ClampMin = 0.1, UIMin = 0.1, UIMax = 5.0, Delta = 0.01))
	float FragmentPaddingSliderMax = 2.0;
	
	// - - - - - EDITOR - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	
	/** Controls how missing audio fields are handled. */ // TODO make it not package
	UPROPERTY(Config, EditAnywhere, Category = "Editor", DisplayName = "Missing Audio", meta = (EditCondition = "DefaultTimeModeSetting == EYapTimeMode::AudioTime", EditConditionHides))
	EYapMissingAudioErrorLevel MissingAudioErrorLevel;

	/** Normally, when assigning dialogue text, Yap will parse the text and attempt to cache a word count to use for determine text time length. Set this to prevent that. */
	UPROPERTY(Config, EditAnywhere, Category = "Editor")
	bool bPreventCachingWordCount = false;

	/** Normally, when assigning an audio length, Yap will read the audio asset and set the speaking time based on it. Set this to prevent that. */
	UPROPERTY(Config, EditAnywhere, Category = "Editor")
	bool bPreventCachingAudioLength = false;
	
	/** If enabled, will show title text on normal talk nodes as well as player prompt nodes. */
	UPROPERTY(Config, EditAnywhere, Category = "Editor")
	bool bShowTitleTextOnTalkNodes = false;
	
	/** Filters dialogue and fragment tags. */
	UPROPERTY(Config, EditAnywhere, Category = "Dialogue Tags")
	FGameplayTag DialogueTagsParent;

	// ============================================================================================
	// STATE
	// ============================================================================================

#if WITH_EDITORONLY_DATA
protected:
	TMap<EYap_TagFilter, FGameplayTag*> TagContainers;
	
	// A registered property name (FName) will get bound to a map of classes and the type of tag filter to use for it
	TMultiMap<FName, TMap<UClass*, EYap_TagFilter>> TagFilterSubscriptions;
#endif

	/** If set, you will not be warned when Yap is falling back to default maturity settings. Turn this on if you  */
	UPROPERTY(Config, EditAnywhere, Category = "Settings")
	bool bSuppressDefaultMatureWarning = false;
	
	UPROPERTY(Config, EditAnywhere, Category = "Settings")
	EYapMaturitySetting DefaultMaturitySetting;

	UPROPERTY(Config, EditAnywhere, Category = "Settings")
	TSoftObjectPtr<UTexture2D> MissingPortraitTexture;

#if WITH_EDITORONLY_DATA
	/** Turn off to hide the On Start / On End pin-buttons, useful if you want a simpler graph without these features. */
	UPROPERTY(Config, EditAnywhere, Category = "Settings")
	bool bHidePinEnableButtons = false;
	
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

	UPROPERTY(Transient)
	TObjectPtr<UTexture2D> MissingPortraitTexture_Loaded; // TODO move this to the subsystem, I don't want my settings class to contain mutable state
	
	// ------------------------------------------
	// UObject overrides
#if WITH_EDITOR
public:
	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	
	void PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent) override;
#endif
	
	// ------------------------------------------
	// UDeveloperSettings overrides
#if WITH_EDITOR
public:
	static FName CategoryName;
	
	FName GetCategoryName() const override { return CategoryName; }

	FText GetSectionText() const override { return LOCTEXT("Settings", "Settings"); }
	
	FText GetSectionDescription() const override { return LOCTEXT("YapProjectSettingsDescription", "Project-specific settings for Yap"); }
#endif

	// ------------------------------------------
	// Custom API overrides
public:

#if WITH_EDITOR
	static FString GetMoodKeyIconPath(FGameplayTag Key, FString FileExtension);

	static const FGameplayTag& GetMoodTagsParent() { return Get().MoodTagsParent; }
	
	static const FGameplayTag& GetDialogueTagsParent() { return Get().DialogueTagsParent; };
	
	static FGameplayTagContainer GetMoodTags();
#endif

	static bool GetSuppressDefaultMatureWarning() { return Get().bSuppressDefaultMatureWarning; }

	static FGameplayTag GetDefaultMoodTag() { return Get().DefaultMoodTag; }
	
	static EYapTimeMode GetDefaultTimeModeSetting() { return Get().DefaultTimeModeSetting; }

	static EYapDialogueSkippable GetDefaultSkippableSetting() { return Get().DefaultSkippableSetting; }
	
	static EYapMaturitySetting GetDefaultMaturitySetting() { return Get().DefaultMaturitySetting; }
	
	static const TSoftClassPtr<UYapBroker>& GetBrokerClass() { return Get().BrokerClass; }

#if WITH_EDITOR
	static const UYapBroker* GetEditorBrokerDefault()
	{ 
		TSoftClassPtr<UYapBroker> BrokerClass = UYapProjectSettings::GetBrokerClass();

		if (BrokerClass.IsNull())
		{
			UE_LOG(LogYap, Error, TEXT("No broker class set! Set a Yap Broker class in project settings."));
			return nullptr;
		}

		return BrokerClass.LoadSynchronous()->GetDefaultObject<UYapBroker>();
	}
	
	static const TArray<TSoftClassPtr<UObject>>& GetAudioAssetClasses();
#endif

	static bool HasCustomAudioAssetClasses() { return Get().OverrideAudioAssetClasses.Num() > 0; };
	
	static bool GetShowTitleTextOnTalkNodes() { return Get().bShowTitleTextOnTalkNodes; }

	static int32 GetTextWordsPerMinute() { return Get().TextWordsPerMinute; }

	static double GetMinimumAutoTextTimeLength() { return Get().MinimumAutoTextTimeLength; };
	
	static double GetMinimumAutoAudioTimeLength() { return Get().MinimumAutoAudioTimeLength; }
	
	static double GetMinimumFragmentTime() { return Get().MinimumFragmentTime; }

	static bool CacheFragmentWordCount() { return !Get().bPreventCachingWordCount; }
	
	static bool CacheFragmentAudioLength() { return !Get().bPreventCachingAudioLength; }
	
	static double GetDefaultFragmentPaddingTime() { return Get().DefaultFragmentPaddingTime; }
	
	static EYapMissingAudioErrorLevel GetMissingAudioBehavior() { return Get().MissingAudioErrorLevel; }

	static const UTexture2D* GetMissingPortraitTexture();
	
#if WITH_EDITOR
public:
	static const FString& GetMoodKeyIconPath();
	
	static int32 GetDialogueWidthAdjustment() { return Get().DialogueWidthAdjustment; };

	static int32 GetPortraitSize() { return Get().PortraitSize; }


	static float GetDialogueTimeSliderMax() { return Get().DialogueTimeSliderMax; }

	static float GetFragmentPaddingSliderMax() { return Get().PaddingTimeSliderMax; }

	static bool ShowPinEnableButtons()  { return !Get().bHidePinEnableButtons; }
	
	static void RegisterTagFilter(UObject* ClassSource, FName PropertyName, EYap_TagFilter Filter);

	static FString GetTrimmedGameplayTagString(EYap_TagFilter Filter, const FGameplayTag& PropertyTag);

protected:
	void OnGetCategoriesMetaFromPropertyHandle(TSharedPtr<IPropertyHandle> PropertyHandle, FString& MetaString) const;
#endif
};

#undef LOCTEXT_NAMESPACE
