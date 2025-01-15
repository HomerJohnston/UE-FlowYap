// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license.

#pragma once

#include "IDetailCustomization.h"

class UYapCharacter;
struct FGameplayTag;
class IDetailCategoryBuilder;

#define LOCTEXT_NAMESPACE "YapEditor"

class FDetailCustomization_YapCharacter : public IDetailCustomization
{
private:
	TWeakObjectPtr<UYapCharacter> CharacterBeingCustomized;
	
public:
	static TSharedRef<IDetailCustomization> MakeInstance()
	{
		return MakeShareable(new FDetailCustomization_YapCharacter());
	}


	void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

	FText Text_PortraitsList() const;

	FText Text_RefreshMoodKeysButton() const;
	FText ToolTipText_RefreshMoodKeysButton() const;
	FReply OnClicked_RefreshMoodKeysButton();
	bool IsEnabled_RefreshMoodKeysButton() const;

	TSharedPtr<IPropertyHandle> PortraitsProperty;
};

#undef LOCTEXT_NAMESPACE