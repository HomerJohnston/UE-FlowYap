#include "Yap/DetailCustomizations/DetailCustomization_YapCharacter.h"

#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "Yap/YapCharacter.h"

void FDetailCustomization_YapCharacter::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> Objects;

	DetailBuilder.GetObjectsBeingCustomized(Objects);

	if (Objects.Num() == 1)
	{
		CharacterBeingCustomized = Cast<UYapCharacter>(Objects[0].Get());
	}
	else
	{
		return;
	}
	
	// Add a button we can click on to open the documentation
	IDetailCategoryBuilder& HelpCategory = DetailBuilder.EditCategory("Yap Character");
	HelpCategory.AddCustomRow(INVTEXT("Test"))
	[
		SNew(SButton)
		.Cursor(EMouseCursor::Default)
		.Text(this, &FDetailCustomization_YapCharacter::Text_RefreshMoodKeysButton)
		.ToolTipText(this, &FDetailCustomization_YapCharacter::ToolTipText_RefreshMoodKeysButton)
		.HAlign(HAlign_Center)
		.OnClicked(this, &FDetailCustomization_YapCharacter::OnClicked_RefreshMoodKeysButton)
		.IsEnabled(this, &FDetailCustomization_YapCharacter::IsEnabled_RefreshMoodKeysButton)
	];

	auto fuckyouunreal = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UYapCharacter, Portraits));

	fuckyouunreal->MarkHiddenByCustomization();

	HelpCategory.AddProperty(fuckyouunreal);
}

FText FDetailCustomization_YapCharacter::Text_RefreshMoodKeysButton() const
{
	return INVTEXT("Refresh Portrait List");
}

FText FDetailCustomization_YapCharacter::ToolTipText_RefreshMoodKeysButton() const
{
	return INVTEXT("Will process the portraits list, removing entries which are no longer present in project settings, and adding missing entries.");
}

FReply FDetailCustomization_YapCharacter::OnClicked_RefreshMoodKeysButton()
{
	CharacterBeingCustomized->RefreshPortraitList();
	
	return FReply::Handled();
}

bool FDetailCustomization_YapCharacter::IsEnabled_RefreshMoodKeysButton() const
{
	return true;
}
