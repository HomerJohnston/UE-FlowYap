#pragma once

#include "Textures/SlateIcon.h"

#include "FlowYapEditorSubsystem.generated.h"

class UFlowYapCharacter;

UCLASS()
class UFlowYapEditorSubsystem : public UEditorSubsystem
{
	GENERATED_BODY()

#if WITH_EDITORONLY_DATA
private:
	// TODO make this an FSlateBrush to avoid making tons of new brushes in my graph
	// TODO caveat: I need to store the loaded UTexture2D in a UPROPERTY
	UPROPERTY()
	TMap<FName, UTexture2D*> PortraitKeyIconTextures;

	TMap<FName, FSlateIcon> PortraitKeyIcons;
	
	UPROPERTY()
	UTexture2D* DialogueTimerIco;

	UPROPERTY()
	UTexture2D* DialogueUserInterruptIco;
	
#endif

#if WITH_EDITOR
public:
	void UpdatePortraitKeyIconsMap();
	
	UTexture2D* GetPortraitKeyIcon(FName PortraitKey);

	UTexture2D* GetDialogueTimerIco();
	
	UTexture2D* GetDialogueUserInterruptIco();
#endif

public:
	void Initialize(FSubsystemCollectionBase& Collection) override;

	void Deinitialize() override;
};