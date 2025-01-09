﻿// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "YapEditor/DetailCustomizations/DetailCustomization_YapCharacter.h"

#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "Yap/YapCharacter.h"

#define LOCTEXT_NAMESPACE "YapEditor"

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
	
	DetailBuilder.SortCategories([](const TMap<FName, IDetailCategoryBuilder*>& CategoryMap)
	{
		const TMap<FName, int32> SortOrder =
		{
			{ TEXT("YapEntity"), 0},
			{ TEXT("YapCharacter"), 1}
		};

		for (const TPair<FName, int32>& SortPair : SortOrder)
		{
			if (CategoryMap.Contains(SortPair.Key))
			{
				CategoryMap[SortPair.Key]->SetSortOrder(SortPair.Value);
			}
		}
	});
	
	IDetailCategoryBuilder& CharacterCategory = DetailBuilder.EditCategory("YapCharacter");
	
	auto PortraitsProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UYapCharacter, Portraits));

	PortraitsProperty->MarkHiddenByCustomization();

	CharacterCategory.AddProperty(PortraitsProperty);
	
	CharacterCategory.AddCustomRow(LOCTEXT("MoodKeys", "Mood Keys"))
	[
		SNew(SBox)
		.Padding(0, 8)
		[
			SNew(SButton)
			.ContentPadding(FMargin(0, 4))
			.Cursor(EMouseCursor::Default)
			.VAlign(VAlign_Center)
			.Text(this, &FDetailCustomization_YapCharacter::Text_RefreshMoodKeysButton)
			.ToolTipText(this, &FDetailCustomization_YapCharacter::ToolTipText_RefreshMoodKeysButton)
			.HAlign(HAlign_Center)
			.OnClicked(this, &FDetailCustomization_YapCharacter::OnClicked_RefreshMoodKeysButton)
			.IsEnabled(this, &FDetailCustomization_YapCharacter::IsEnabled_RefreshMoodKeysButton)	
		]
	];

}

FText FDetailCustomization_YapCharacter::Text_RefreshMoodKeysButton() const
{
	return LOCTEXT("RefreshPortraitList", "Refresh Portrait List");
}

FText FDetailCustomization_YapCharacter::ToolTipText_RefreshMoodKeysButton() const
{
	return LOCTEXT("RefreshMoodKeys_ToolTIp", "Will process the portraits list, removing entries which are no longer present in project settings, and adding missing entries.");
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

#undef LOCTEXT_NAMESPACE