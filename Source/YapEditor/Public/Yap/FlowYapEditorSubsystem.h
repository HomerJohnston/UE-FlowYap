#pragma once

#include "Textures/SlateIcon.h"

#include "GameplayTagContainer.h"

#include "FlowYapEditorSubsystem.generated.h"

class UYapCharacter;
class FFlowYapInputTracker;
struct FYapFragment;

struct FCheckBoxStyles
{
	// Generic check boxes
	FCheckBoxStyle ToggleButtonCheckBox_Red;
	FCheckBoxStyle ToggleButtonCheckBox_Green;
	FCheckBoxStyle ToggleButtonCheckBox_Blue;
	FCheckBoxStyle ToggleButtonCheckBox_Orange;
	FCheckBoxStyle ToggleButtonCheckBox_White;
	FCheckBoxStyle ToggleButtonCheckBox_Transparent;

	// Custom check boxes
	FCheckBoxStyle ToggleButtonCheckBox_PlayerPrompt;
	FCheckBoxStyle ToggleButtonCheckBox_DialogueInterrupt;
};

UCLASS()
class UFlowYapEditorSubsystem : public UEditorSubsystem
{
	GENERATED_BODY()

private:
	UPROPERTY(Transient)
	TMap<FGameplayTag, UTexture2D*> MoodKeyIconTextures;
	TMap<FGameplayTag, TSharedPtr<FSlateBrush>> MoodKeyIconBrushes;

protected:
	static FCheckBoxStyles CheckBoxStyles;

	// STATE
	TSharedPtr<FFlowYapInputTracker> InputTracker;

	FDelegateHandle FragmentTagFilterDelegateHandle;

public:
	void UpdateMoodKeyIconsMap();
	
	UTexture2D* GetMoodKeyIcon(FGameplayTag MoodKey);

	const FSlateBrush* GetMoodKeyBrush(FGameplayTag Name);

public:
	void Initialize(FSubsystemCollectionBase& Collection) override;

	void Deinitialize() override;
	
	void LoadIcon(FString LocalResourcePath, UTexture2D*& Texture, FSlateBrush& Brush, int32 XYSize = 16);
	
	static const FCheckBoxStyles& GetCheckBoxStyles();

	FFlowYapInputTracker* GetInputTracker();

	TMap<TWeakPtr<FYapFragment>, TArray<FName>> FragmentPins;
	
	void SetupGameplayTagFiltering();

	void OnGetCategoriesMetaFromPropertyHandle(TSharedPtr<IPropertyHandle> PropertyHandle, FString& String) const;

	bool IsMoodKeyProperty(TSharedPtr<IPropertyHandle> PropertyHandle) const;
};