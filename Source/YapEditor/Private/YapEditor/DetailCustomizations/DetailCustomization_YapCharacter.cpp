// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "YapEditor/DetailCustomizations/DetailCustomization_YapCharacter.h"

#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "ISettingsModule.h"
#include "SSimpleButton.h"
#include "Yap/YapCharacter.h"
#include "Yap/YapGlobals.h"
#include "YapEditor/YapEditorStyle.h"
#include "YapEditor/SlateWidgets/SYapHyperlink.h"

#define LOCTEXT_NAMESPACE "YapEditor"

void FDetailCustomization_YapCharacter::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	//DetailBuilderWeakPtr = DetailBuilder;
	
	TArray<TWeakObjectPtr<UObject>> Objects;

	PortraitsProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UYapCharacter, Portraits));

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


	PortraitsProperty->MarkHiddenByCustomization();

	CharacterCategory.AddProperty(PortraitsProperty);

	FDetailWidgetRow X = CharacterCategory.AddCustomRow(LOCTEXT("MoodTags", "Mood Tags"))
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Center)
		.Padding(0, 12, 0, 4)
		[
			SNew(SBox)
			.Visibility(this, &FDetailCustomization_YapCharacter::Visibility_EmptyPortraitsMapInfo)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(STextBlock)
					.Font(YapFonts.Font_WarningText)
					.Text(LOCTEXT("CharacterPortraitsEmpty_Info", "You need to create mood tags. Go to "))//"Once finished, use the button below to build the character's portrait images list. Open "))
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SYapHyperlink)
					//.Font(YapFonts.Font_WarningText)
					.Text(LOCTEXT("CharacterPortraitsEmpty_OpenProjectSettings", "Yap Project Settings"))
					.OnNavigate_Lambda( [] () { Yap::OpenProjectSettings(); } )
				]
			]
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Center)
		.Padding(0, 4, 0, 4)
		[
			SNew(SBox)
			.Padding(0, 8)
			.MaxDesiredWidth(150)
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

EVisibility FDetailCustomization_YapCharacter::Visibility_EmptyPortraitsMapInfo() const
{
	FProperty* Val; PortraitsProperty->GetValue(Val);

	TArray<void*> RawData;

	PortraitsProperty->AccessRawData(RawData);

	const TMap<FName, TObjectPtr<UTexture2D>>* Map = reinterpret_cast<const TMap<FName, TObjectPtr<UTexture2D>>*>(RawData[0]);

	return Map->Num() == 0 ? EVisibility::Visible : EVisibility::Collapsed;
	//return EVisibility::Visible;
}

#undef LOCTEXT_NAMESPACE