#pragma once

#include "Textures/SlateIcon.h"

#include "FlowYapEditorSubsystem.generated.h"

class UFlowYapCharacter;
class FFlowYapInputTracker;

struct FCheckBoxStyles
{
	FCheckBoxStyle ToggleButtonCheckBox_Red;
	FCheckBoxStyle ToggleButtonCheckBox_Green;
	FCheckBoxStyle ToggleButtonCheckBox_Blue;
	FCheckBoxStyle ToggleButtonCheckBox_Orange;
	FCheckBoxStyle ToggleButtonCheckBox_White;	
};

UCLASS()
class UFlowYapEditorSubsystem : public UEditorSubsystem
{
	GENERATED_BODY()

private:
	UPROPERTY(Transient)
	TMap<FName, UTexture2D*> PortraitKeyIconTextures;
	TMap<FName, TSharedPtr<FSlateBrush>> PortraitKeyIconBrushes; // TODO is this safe? I fucking suck at memory management
	
	UPROPERTY(Transient)
	UTexture2D* TimerIcon;
	FSlateBrush TimerBrush;

	UPROPERTY(Transient)
	UTexture2D* UserInterruptIcon;
	FSlateBrush UserInterruptBrush;

	UPROPERTY(Transient)
	UTexture2D* TextTimeIcon;
	FSlateBrush TextTimeBrush;
	
	UPROPERTY(Transient)
	UTexture2D* AudioTimeIcon;
	FSlateBrush AudioTimeBrush;

protected:
	static FCheckBoxStyles CheckBoxStyles;

	// STATE
	TSharedPtr<FFlowYapInputTracker> InputTracker;
	
public:
	void UpdatePortraitKeyIconsMap();
	
	UTexture2D* GetPortraitKeyIcon(FName PortraitKey);

	const FSlateBrush* GetPortraitKeyBrush(FName Name);

	const FSlateBrush* GetUserInterruptBrush() { return &UserInterruptBrush; }
	
	const FSlateBrush* GetTimerBrush() { return &TimerBrush; }

	const FSlateBrush* GetTextTimeBrush() { return &TextTimeBrush; }

	const FSlateBrush* GetAudioTimeBrush() { return &AudioTimeBrush; }

public:
	void Initialize(FSubsystemCollectionBase& Collection) override;

	void Deinitialize() override;
	
	void LoadIcon(FString LocalResourcePath, UTexture2D*& Texture, FSlateBrush& Brush, int32 XYSize = 16);
	
	static const FCheckBoxStyles& GetCheckBoxStyles();

	FFlowYapInputTracker* GetInputTracker();
};