#pragma once

#include "FlowYap/FlowYapFragment.h"
#include "Textures/SlateIcon.h"

#include "FlowYapEditorSubsystem.generated.h"

class UFlowYapCharacter;

UCLASS()
class UFlowYapEditorSubsystem : public UEditorSubsystem
{
	GENERATED_BODY()

#if WITH_EDITORONLY_DATA
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
	
#endif

#if WITH_EDITOR
public:
	void UpdatePortraitKeyIconsMap();
	
	UTexture2D* GetPortraitKeyIcon(FName PortraitKey);

	const FSlateBrush* GetPortraitKeyBrush(FName Name);

	const FSlateBrush* GetUserInterruptBrush() { return &UserInterruptBrush; }
	
	const FSlateBrush* GetTimerBrush() { return &TimerBrush; }

	const FSlateBrush* GetTextTimeBrush() { return &TextTimeBrush; }

	const FSlateBrush* GetAudioTimeBrush() { return &AudioTimeBrush; }
	

	
#endif

public:
	void Initialize(FSubsystemCollectionBase& Collection) override;

	void LoadIcon(FString LocalResourcePath, UTexture2D*& Texture, FSlateBrush& Brush, int32 XYSize = 16);
	
	void Deinitialize() override;
};