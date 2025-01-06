// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#pragma once

#include "IDetailCustomization.h"

class SGameplayTagPicker;
struct FGameplayTag;
class IDetailCategoryBuilder;

class FDetailCustomization_YapProjectSettings : public IDetailCustomization
{
private:
	TArray<FString> DefaultMoodTags
	{
		"Yap.Mood.Angry",
		"Yap.Mood.Calm",
		"Yap.Mood.Confused",
		"Yap.Mood.Disgusted",
		"Yap.Mood.Happy",
		"Yap.Mood.Injured",
		"Yap.Mood.Laughing",
		"Yap.Mood.Panicked",
		"Yap.Mood.Sad",
		"Yap.Mood.Scared",
		"Yap.Mood.Smirking",
		"Yap.Mood.Stressed",
		"Yap.Mood.Surprised",
		"Yap.Mood.Thinking",
		"Yap.Mood.Tired"
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

	FReply OnClicked_OpenTagsManager(FText Title, FString Filter);
};
