#pragma once

#include "FlowYapEditorSubsystem.generated.h"

class UFlowYapCharacter;

UCLASS()
class UFlowYapEditorSubsystem : public UEditorSubsystem
{
	GENERATED_BODY()

#if WITH_EDITORONLY_DATA
private:
	UPROPERTY()
	TMap<FName, UTexture2D*> PortraitKeyIcons;
#endif


#if WITH_EDITOR
public:
	void UpdatePortraitKeyIconsMap();
	
	UTexture2D* GetPortraitKeyIcon(FName PortraitKey);
#endif

public:
	void Initialize(FSubsystemCollectionBase& Collection) override;

	void Deinitialize() override;
};