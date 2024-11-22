

#pragma once

#include "GameplayTagContainer.h"
#include "IDetailCustomization.h"

struct FGameplayTag;
class IDetailCategoryBuilder;

class FDetailCustomization_YapProjectSettings : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance()
	{
		return MakeShareable(new FDetailCustomization_YapProjectSettings());
	}

	FText GetMoodTags() const;

	const FSlateBrush* TODOBorderImage() const;
	
	void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

	EVisibility Visibility_AddDefaultMoodTags() const;
	
	FReply OnClicked_AddDefaultMoodTags() const;
};
