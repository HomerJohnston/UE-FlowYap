// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license.

#pragma once

#include "Textures/SlateIcon.h"
#include "GameplayTagContainer.h"
#include "Yap/YapCondition.h"

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
	TMap<FGameplayTag, TSharedPtr<FSlateImageBrush>> MoodKeyIconBrushes;

protected:
	// STATE
	TSharedPtr<FYapInputTracker> InputTracker;

	FDelegateHandle FragmentTagFilterDelegateHandle;

public:
	void UpdateMoodKeyBrushes();
	void BuildIcon(const FGameplayTag& MoodKey);

	TSharedPtr<FSlateImageBrush> GetMoodKeyIcon(FGameplayTag MoodKey);

	const FSlateBrush* GetMoodKeyBrush(FGameplayTag Name);

public:
	void Initialize(FSubsystemCollectionBase& Collection) override;

	void Deinitialize() override;
	
	void LoadIcon(FString LocalResourcePath, UTexture2D*& Texture, FSlateBrush& Brush, int32 XYSize = 16);
	
	FYapInputTracker* GetInputTracker();

	TMap<TWeakPtr<FYapFragment>, TArray<FName>> FragmentPins;
	
	void SetupGameplayTagFiltering();

	void OnGetCategoriesMetaFromPropertyHandle(TSharedPtr<IPropertyHandle> PropertyHandle, FString& String) const;

	bool IsMoodKeyProperty(TSharedPtr<IPropertyHandle> PropertyHandle) const;

public:
	static bool bLiveCodingInProgress;

	static TArray<TWeakObjectPtr<UObject>> OpenedAssets;

	void UpdateLiveCodingState(bool bNewState);
	
	void ReOpenAssets();

	void Tick(float DeltaTime) override;

	TStatId GetStatId() const override;
	
	void OnPatchComplete();
	
	FDelegateHandle OnPatchCompleteHandle;
};

#undef LOCTEXT_NAMESPACE