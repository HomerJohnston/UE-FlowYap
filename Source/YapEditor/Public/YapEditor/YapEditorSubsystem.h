// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license.

#pragma once

#include "Textures/SlateIcon.h"
#include "GameplayTagContainer.h"

#include "YapEditorSubsystem.generated.h"

class UYapCharacter;
class FYapInputTracker;
struct FYapFragment;

#define LOCTEXT_NAMESPACE "YapEditor"

struct FCheckBoxStyles
{
	// Generic check boxes
	FCheckBoxStyle ToggleButtonCheckBox_White;
	FCheckBoxStyle ToggleButtonCheckBox_Transparent;

	// Custom check boxes
	FCheckBoxStyle ToggleButtonCheckBox_PlayerPrompt;
	FCheckBoxStyle ToggleButtonCheckBox_DialogueInterrupt;
};

UCLASS()
class UYapEditorSubsystem : public UEditorSubsystem, public FTickableEditorObject
{
	GENERATED_BODY()

public:
	static UYapEditorSubsystem* Get()
	{
		if (GEditor)
		{
			return GEditor->GetEditorSubsystem<UYapEditorSubsystem>();
		}

		return nullptr;
	}
	
private:
	TMap<FGameplayTag, TSharedPtr<FSlateImageBrush>> MoodTagIconBrushes;

protected:
	// STATE
	TSharedPtr<FYapInputTracker> InputTracker;

	FDelegateHandle FragmentTagFilterDelegateHandle;

	TMap<TObjectKey<UTexture2D>, TSharedPtr<FSlateImageBrush>> CharacterPortraitBrushes;

	FGameplayTagContainer CachedMoodTags;

	bool bMoodTagsDirty;

	UPROPERTY(Transient)
	TObjectPtr<UTexture2D> MissingPortraitTexture;

	TWeakObjectPtr<UAudioComponent> PreviewSoundComponent;
public:
	void UpdateMoodTagBrushesIfRequired();

	void UpdateMoodTagBrushes();
	
protected:
	void BuildIcon(const FGameplayTag& MoodTag);

public:
	TSharedPtr<FSlateImageBrush> GetMoodTagIcon(FGameplayTag MoodTag);

	const FSlateBrush* GetMoodTagBrush(FGameplayTag Name);

	static TSharedPtr<FSlateImageBrush> GetCharacterPortraitBrush(const UYapCharacter* Character, const FGameplayTag& MoodTag);

public:
	void Initialize(FSubsystemCollectionBase& Collection) override;
	
	void Deinitialize() override;
	
	FYapInputTracker* GetInputTracker();

	TMap<TWeakPtr<FYapFragment>, TArray<FName>> FragmentPins;
	
	void SetupGameplayTagFiltering();

	void OnGetCategoriesMetaFromPropertyHandle(TSharedPtr<IPropertyHandle> PropertyHandle, FString& String) const;

	bool IsMoodTagProperty(TSharedPtr<IPropertyHandle> PropertyHandle) const;

public:
	static bool bLiveCodingInProgress;

	static TArray<TWeakObjectPtr<UObject>> OpenedAssets;

	void UpdateLiveCodingState(bool bNewState);
	
	void ReOpenAssets();

	void Tick(float DeltaTime) override;

	static bool GetMoodTagsDirty();

	TStatId GetStatId() const override;
	
	void OnPatchComplete();
	
	FDelegateHandle OnPatchCompleteHandle;
};

#undef LOCTEXT_NAMESPACE