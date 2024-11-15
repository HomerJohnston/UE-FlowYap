#pragma once

#include "FlowYapEditorSettings.generated.h"

UCLASS(Config = Editor, DefaultConfig, DisplayName = "Flow Yap")
class UFlowYapEditorSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UFlowYapEditorSettings();
	
	static UFlowYapEditorSettings* Get() { return StaticClass()->GetDefaultObject<UFlowYapEditorSettings>(); }

protected:
	UPROPERTY(EditAnywhere, Config, Category = "Graph Settings", meta = (ClampMin = 0, UIMin = 0, UIMax = 4))
	uint8 DialogueRowSpacing;
	
	/*
	UPROPERTY(EditAnywhere, meta = (RelativePath))
	FDirectoryPath DirectoryPath_RelativePath;

	UPROPERTY(EditAnywhere, meta = (ContentDir))
	FDirectoryPath DirectoryPath_ContentDir;
	
	UPROPERTY(EditAnywhere, meta = (RelativeToGameContentDir))
	FDirectoryPath DirectoryPath_RelativeToGameContentDir;
	
	UPROPERTY(EditAnywhere, meta = (RelativeToGameDir))
	FDirectoryPath DirectoryPath_RelativeToGameDir;
	
	UPROPERTY(EditAnywhere, meta = (RelativePath))
	FFilePath FilePath_RelativePath;
	*/
	
public:
	uint8 GetDialogueRowSpacing() const { return DialogueRowSpacing; }   
	
public:
	virtual FName GetCategoryName() const override { return FName("Flow Yap"); }
	virtual FText GetSectionText() const override { return INVTEXT("Editor Settings"); }
};
