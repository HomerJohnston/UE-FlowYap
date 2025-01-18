// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once

#include "IDetailCustomization.h"

class SGameplayTagPicker;
struct FGameplayTag;
class IDetailCategoryBuilder;

#define LOCTEXT_NAMESPACE "YapEditor"

class FDetailCustomization_YapProjectSettings : public IDetailCustomization
{
private:
	TArray<FString> DefaultMoodTags
	{
		"Angry",
		"Calm",
		"Confused",
		"Disgusted",
		"Happy",
		"Injured",
		"Laughing",
		"Panicked",
		"Sad",
		"Scared",
		"Smirking",
		"Stressed",
		"Surprised",
		"Thinking",
		"Tired"
	};
	
public:
	static TSharedRef<IDetailCustomization> MakeInstance()
	{
		return MakeShareable(new FDetailCustomization_YapProjectSettings());
	}

protected:
	FText GetMoodTags() const;

	const FSlateBrush* TODOBorderImage() const;


	void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

	FReply OnClicked_ResetDefaultMoodTags() const;

	FReply OnClicked_DeleteAllMoodTags() const;

	FReply OnClicked_OpenMoodTagsManager();

	FReply OnClicked_OpenDialogueTagsManager();

	FText ToolTipText_DefaultMoodTags() const;

	bool IsMoodTagsParentSet() const;

	TSharedPtr<IPropertyHandle> MoodTagsParentPropertyHandle;
};

#undef LOCTEXT_NAMESPACE