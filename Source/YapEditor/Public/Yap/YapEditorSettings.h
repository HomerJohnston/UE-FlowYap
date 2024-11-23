#pragma once

#include "YapEditorSettings.generated.h"

UCLASS(Config = Editor, DefaultConfig, DisplayName = "Flow Yap")
class UYapEditorSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UYapEditorSettings();
	
	static UYapEditorSettings* Get() { return StaticClass()->GetDefaultObject<UYapEditorSettings>(); }

protected:
	UPROPERTY(Config, EditAnywhere, Category = "Settings", meta = (ClampMin = -200, ClampMax = 2000, UIMin = -200, UIMax = 1000, Delta = 10))
	int32 DialogueWidthAdjustment;
		
	UPROPERTY(Config, EditAnywhere, Category = "Settings", meta = (ClampMin = 64, ClampMax = 128, UIMin = 64, UIMax = 128, Multiple = 32))
	int32 PortraitSize = 64;
	
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
	int32 GetDialogueWidthAdjustment() const { return DialogueWidthAdjustment; };

	int32 GetPortraitSize() const { return PortraitSize; }

public:
	virtual FName GetCategoryName() const override { return FName("Flow Yap"); }
	virtual FText GetSectionText() const override { return INVTEXT("Editor Settings"); }
};
