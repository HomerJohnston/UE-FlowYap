#pragma once

#include "GameplayTagContainer.h"
#include "IDetailCustomization.h"

class UYapCharacter;
struct FGameplayTag;
class IDetailCategoryBuilder;

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

	FText Text_RefreshMoodKeysButton() const;
	FText ToolTipText_RefreshMoodKeysButton() const;
	FReply OnClicked_RefreshMoodKeysButton();
	bool IsEnabled_RefreshMoodKeysButton() const;
};
