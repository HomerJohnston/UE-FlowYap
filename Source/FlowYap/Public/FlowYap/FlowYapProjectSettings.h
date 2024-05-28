#pragma once

#include "GameplayTagContainer.h"
#include "Engine/DeveloperSettings.h"

#include "FlowYapProjectSettings.generated.h"

UCLASS(Config = Game, DefaultConfig, DisplayName = "Yap")
class FLOWYAP_API UFlowYapProjectSettings : public UDeveloperSettings
{
	GENERATED_BODY()
public:
	UFlowYapProjectSettings();
	
protected:
	// TODO: editing this needs to tell you that you need to restart the editor somehow. Details customization with a big warning bar in slate surrounding these settings after they're modified??
	UPROPERTY(Config, EditAnywhere, Category = "Settings")
	TArray<FName> PortraitKeys;

	UPROPERTY(EditAnywhere)
	bool bTimed = true;

	UPROPERTY(EditAnywhere, meta = (EditCondition="bTimed"))
	bool bUseAutoTime = true;

	/** WARNING: This setting will supersede UseAutoTime when an audio asset is assigned. \n\nIf you are using audio assets that don't include the full dialogue, like just playing a generic laugh or groan on top of dialogue text, you may want to turn this off. */
	UPROPERTY(EditAnywhere, meta = (EditCondition="bTimed"))
	bool bUseAudioAssetLength = true;

	/** After each dialogue is finished being spoken, a brief extra pause can be inserted before moving onto the next node. */
	UPROPERTY(EditAnywhere)
	float EndPaddingTime = 0.25f;
	
	UPROPERTY(EditAnywhere)
	bool bUserInterruptible = true;
public:
	TMulticastDelegate<void()> OnPortraitKeysChanged;
	
#if WITH_EDITORONLY_DATA
protected:
	/** Where to look for portrait key icons. Path should start in the project's root folder, i.e. to use a folder like "...\ProjectName\\Resources\\PortraitKeys", simply type "Resources\\PortraitKeys". If unspecified, will use the "...ProjectName\\Plugins\\FlowYap\\Resources\\PortraitKeys" folder.*/
	UPROPERTY(Config, EditAnywhere, Category = "Settings")
	FDirectoryPath PortraitKeyIconPath;
#endif
	
#if WITH_EDITOR
public:
	virtual FName GetCategoryName() const override { return FName("Flow Yap"); }

	virtual FText GetSectionText() const override { return INVTEXT("Settings"); }

	FString GetPortraitIconPath(FName Key) const;

	const TArray<FName>& GetPortraitKeys() const;

	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};