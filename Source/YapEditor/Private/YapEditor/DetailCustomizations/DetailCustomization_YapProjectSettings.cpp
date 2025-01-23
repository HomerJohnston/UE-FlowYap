// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license. 

#include "YapEditor/DetailCustomizations/DetailCustomization_YapProjectSettings.h"

#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "GameplayTagsEditorModule.h"
#include "SGameplayTagPicker.h"
#include "YapEditor/YapTransactions.h"
#include "Yap/YapProjectSettings.h"
#include "YapEditor/YapColors.h"
#include "YapEditor/YapEditorStyle.h"
#include "YapEditor/YapEditorSubsystem.h"

#define LOCTEXT_NAMESPACE "YapEditor"

FText FDetailCustomization_YapProjectSettings::GetMoodTags() const
{
	const UYapProjectSettings* ProjectSettings = GetDefault<UYapProjectSettings>();
	
	const FGameplayTag& ParentTag = ProjectSettings->GetMoodTagsParent();

	if (!ParentTag.IsValid())
	{
		return LOCTEXT("None_Label", "<None>");
	}
	
	FGameplayTagContainer MoodTags = ProjectSettings->GetMoodTags();
	
	FString TagString;
	bool bFirst = true;
	int32 ParentTagLen = ParentTag.ToString().Len() + 1;
	
	for (const FGameplayTag& Tag : MoodTags)
	{
		if (bFirst)
		{			
			bFirst = false;
		}
		else
		{
			TagString += "\n";
		}
		
		TagString += Tag.ToString().RightChop(ParentTagLen);
	}

	if (TagString.IsEmpty())
	{
		TagString = "<None>";
	}
	
	return FText::FromString(TagString);
}

const FSlateBrush* FDetailCustomization_YapProjectSettings::TODOBorderImage() const
{
	return FAppStyle::Get().GetBrush("SCSEditor.Background");
	//return FAppStyle::GetBrush("Menu.Background");
}

void CustomSortYapProjectSettingsCategories(const TMap<FName, IDetailCategoryBuilder*>&  AllCategoryMap )
{
	(*AllCategoryMap.Find(FName("Core")))->SetSortOrder(0);
	(*AllCategoryMap.Find(FName("Mood Tags")))->SetSortOrder(1);
	(*AllCategoryMap.Find(FName("Dialogue Tags")))->SetSortOrder(2);
	(*AllCategoryMap.Find(FName("Dialogue Playback")))->SetSortOrder(3);
	(*AllCategoryMap.Find(FName("Editor")))->SetSortOrder(4);
	(*AllCategoryMap.Find(FName("Flow Graph Appearance")))->SetSortOrder(5);
}

void FDetailCustomization_YapProjectSettings::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{	
	DetailBuilder.SortCategories(&CustomSortYapProjectSettingsCategories);
	
	IDetailCategoryBuilder& MoodTagsCategory = DetailBuilder.EditCategory("Mood Tags");
	IDetailCategoryBuilder& DialogueTagsCategory = DetailBuilder.EditCategory("Dialogue Tags");

	ProcessCategory(MoodTagsCategory, &FDetailCustomization_YapProjectSettings::ProcessMoodTagsProperty);
	ProcessCategory(DialogueTagsCategory, &FDetailCustomization_YapProjectSettings::ProcessDialogueTagsCategoryProperty);
}

void FDetailCustomization_YapProjectSettings::ProcessCategory(IDetailCategoryBuilder& Category, void(FDetailCustomization_YapProjectSettings::*Func)(IDetailCategoryBuilder&, TSharedPtr<IPropertyHandle>))
{
	TArray<TSharedRef<IPropertyHandle>> Properties;
	Category.GetDefaultProperties(Properties, true, true);

	for (TSharedPtr<IPropertyHandle> PropertyHandle : Properties)
	{
		Category.AddProperty(PropertyHandle);
		(this->*Func)(Category, PropertyHandle);
	}
}

void FDetailCustomization_YapProjectSettings::ProcessMoodTagsProperty(IDetailCategoryBuilder& Category, TSharedPtr<IPropertyHandle> Property)
{
	if (Property->GetProperty()->GetFName() == GET_MEMBER_NAME_CHECKED(UYapProjectSettings, MoodTagsParent))
	{
		float VerticalPadding = 3.0;

		// Make the widget able to hold all of the default tags
		float TagLineHeight = 15.0; // This is the height of a single tag name in pixels
		float LineHeightPercentage = 18.0 / TagLineHeight; // Desired row height divided by actual height
		float TotalHeight = FMath::RoundFromZero(DefaultMoodTags.Num() * TagLineHeight * LineHeightPercentage + VerticalPadding * 2.0);

		TSharedRef<SButton> Button = SNew(SButton)
		.ButtonStyle(FYapEditorStyle::Get(), YapStyles.ButtonStyle_HoverHintOnly)
		.IsEnabled(this, &FDetailCustomization_YapProjectSettings::IsTagPropertySet, Property)
		.OnClicked(this, &FDetailCustomization_YapProjectSettings::OnClicked_OpenMoodTagsManager)
		.VAlign(VAlign_Fill)
		.HAlign(HAlign_Fill)
		//.Text(LOCTEXT("EditMoodTags", "Edit mood tags"))
		.ToolTipText(LOCTEXT("OpenTagsManager_ToolTip", "Click to edit mood tags"))
		.ContentPadding(FMargin(0, 0, 0, -2))
		[
			SNew(SBox)
			.MaxDesiredHeight(TotalHeight)
			[
				SNew(SScrollBox)
				+ SScrollBox::Slot()
				[
					SNew(SBorder)
					.BorderImage(this, &FDetailCustomization_YapProjectSettings::TODOBorderImage)
					.BorderBackgroundColor(YapColor::DarkGray_Glass)
					.Padding(4, 2, 4, 2)
					[
						SNew(STextBlock)
						.Text(this, &FDetailCustomization_YapProjectSettings::GetMoodTags)
						.LineHeightPercentage(LineHeightPercentage)
					]
				]
			]
		];
		
		Category.AddCustomRow(LOCTEXT("MoodTags_Header", "Mood Tags"))
		.NameContent()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("MoodTags", "Mood tags"))
			.Font(IDetailLayoutBuilder::GetDetailFont())
		]
		.ValueContent()
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, VerticalPadding)
			[
				SNew(SScrollBox)
				+ SScrollBox::Slot()
				[
					SNew(SBorder)
					.BorderImage(this, &FDetailCustomization_YapProjectSettings::TODOBorderImage)
					.Padding(4, 2, 4, 2)
					[
						SNew(STextBlock)
						.Text(this, &FDetailCustomization_YapProjectSettings::GetMoodTags)
						.LineHeightPercentage(LineHeightPercentage)
					]
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, VerticalPadding)
			[
				SNew(SButton)
				.IsEnabled(this, &FDetailCustomization_YapProjectSettings::IsTagPropertySet, Property)
				.OnClicked(this, &FDetailCustomization_YapProjectSettings::OnClicked_OpenMoodTagsManager)
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Center)
				.Text(LOCTEXT("EditMoodTags", "Edit mood tags"))
				.ToolTipText(LOCTEXT("OpenTagsManager_ToolTip", "Open tags manager"))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, VerticalPadding)
			[
				SNew(SButton)
				.IsEnabled(this, &FDetailCustomization_YapProjectSettings::IsTagPropertySet, Property)
				.OnClicked(this, &FDetailCustomization_YapProjectSettings::OnClicked_ResetDefaultMoodTags)
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Center)
				.Text(LOCTEXT("ResetMoodTags_Button", "Reset to defaults..."))
				.ToolTipText(this, &FDetailCustomization_YapProjectSettings::ToolTipText_DefaultMoodTags)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, VerticalPadding)
			[
				SNew(SButton)
				.IsEnabled(this, &FDetailCustomization_YapProjectSettings::IsTagPropertySet, Property)
				.OnClicked(this, &FDetailCustomization_YapProjectSettings::OnClicked_DeleteAllMoodTags)
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Center)
				.Text(LOCTEXT("DeleteMoodTags_Button", "Delete all..."))
				.ToolTipText(LOCTEXT("DeleteMoodTags_ToolTip", "Attempts to delete all tags"))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, VerticalPadding)
			[
				SNew(SButton)
				.Visibility(EVisibility::Collapsed) // TODO -- this does NOT work! Keep it hidden until I can figure out why. For some reason FSlateSVGRasterizer will NOT attempt to load a file that didn't exist on startup?
				.IsEnabled(this, &FDetailCustomization_YapProjectSettings::IsTagPropertySet, Property)
				.OnClicked(this, &FDetailCustomization_YapProjectSettings::OnClicked_RefreshMoodTagIcons)
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Center)
				.Text(LOCTEXT("RefreshMoodTagIcons_Button", "Refresh Icons"))
				.ToolTipText(LOCTEXT("RefreshMoodTagIcons_ToolTip", "Reloads mood tag icons (.svg or .png)"))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, VerticalPadding)
			[
				SNew(STextBlock)
				.Visibility_Lambda([](){return UYapEditorSubsystem::GetMoodTagsDirty() ? EVisibility::Visible : EVisibility::Collapsed;})
				.Font(YapFonts.Font_WarningText)
				.Text(LOCTEXT("MoodTagIconsRequiresRestart_Warning", "You must restart Unreal\nto rebuild mood tag icons."))
				.ColorAndOpacity(YapColor::LightYellow)
			]
		];
	}
}

void FDetailCustomization_YapProjectSettings::ProcessDialogueTagsCategoryProperty(IDetailCategoryBuilder& Category, TSharedPtr<IPropertyHandle> Property)
{
	if (Property->GetProperty()->GetFName() == GET_MEMBER_NAME_CHECKED(UYapProjectSettings, DialogueTagsParent))
	{
		Category.AddCustomRow(LOCTEXT("DialogueTags_Header", "Dialogue Tags"))
		.NameContent()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("DialogueTags", "Dialogue Tags"))
			.Font(IDetailLayoutBuilder::GetDetailFont())
		]
		.ValueContent()
		[
			SNew(SButton)
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Center)
			.ToolTipText(LOCTEXT("OpenTagsManager", "Open tags manager"))
			.Text(LOCTEXT("EditDialogueTags", "Edit dialogue tags"))
			.OnClicked(this, &FDetailCustomization_YapProjectSettings::OnClicked_OpenDialogueTagsManager)
		];
	}
}

FReply FDetailCustomization_YapProjectSettings::OnClicked_ResetDefaultMoodTags() const
{
	if ( EAppReturnType::Yes != FMessageDialog::Open(EAppMsgType::YesNo, LOCTEXT("AreYouSure_Prompt", "Are you sure?")) )
	{
		return FReply::Handled();
	}
	
	UYapProjectSettings* ProjectSettings = GetMutableDefault<UYapProjectSettings>();
	
	FYapTransactions::BeginModify(LOCTEXT("ResetMoodTags", "Reset mood tags"), ProjectSettings);

	FString DefaultMoodTagsParentString = "Yap.Mood";
	
	FName IniFile = "YapGameplayTags.ini";

	// Remove any tags that should not be present
	for (FGameplayTag ExistingTag : ProjectSettings->GetMoodTags())// DefaultTags.CreateIterator(); It; ++It)
	{
		bool bKeepTag = true;

		if (!ExistingTag.IsValid())
		{
			bKeepTag = false;
		}
		else
		{
			FString ExistingTagString = ExistingTag.ToString();

			if (!DefaultMoodTags.ContainsByPredicate(
				[&DefaultMoodTagsParentString, &ExistingTagString]
				(const FString& DefaultTagString)
				{
					FString FullString = DefaultMoodTagsParentString + "." + DefaultTagString;
					return FullString == ExistingTagString;
				}))
			{
				bKeepTag = false;
			}
		}
		
		if (!bKeepTag)
		{
			TSharedPtr<FGameplayTagNode> ExistingTagNode = UGameplayTagsManager::Get().FindTagNode(ExistingTag);
			IGameplayTagsEditorModule::Get().DeleteTagFromINI(ExistingTagNode);
		}
	}
	
	// Make sure all of the default tags exist
	for (const FString& DefaultTagString : DefaultMoodTags)
	{
		FString DefaultTagFullString = DefaultMoodTagsParentString + "." + DefaultTagString;

		FGameplayTag ExistingTag = UGameplayTagsManager::Get().RequestGameplayTag(FName(DefaultTagFullString), false);

		if (!ExistingTag.IsValid())
		{
			IGameplayTagsEditorModule::Get().AddNewGameplayTagToINI(DefaultTagFullString, "", IniFile);
		}
	}

	FYapTransactions::EndModify();
	
	return FReply::Handled();
}

FReply FDetailCustomization_YapProjectSettings::OnClicked_DeleteAllMoodTags() const
{
	if ( EAppReturnType::Yes != FMessageDialog::Open(EAppMsgType::YesNo, LOCTEXT("AreYouSure_Prompt", "Are you sure?")) )
	{
		return FReply::Handled();
	}
	
	UYapProjectSettings* ProjectSettings = GetMutableDefault<UYapProjectSettings>();

	FYapTransactions::BeginModify(LOCTEXT("DeleteMoodTags", "Delete mood tags"), ProjectSettings);

	for (FGameplayTag ExistingTag : ProjectSettings->GetMoodTags())
	{
		TSharedPtr<FGameplayTagNode> ExistingTagNode = UGameplayTagsManager::Get().FindTagNode(ExistingTag);
		IGameplayTagsEditorModule::Get().DeleteTagFromINI(ExistingTagNode);
	}
	
	FYapTransactions::EndModify();

	return FReply::Handled();
}

FReply FDetailCustomization_YapProjectSettings::OnClicked_OpenMoodTagsManager()
{
	FGameplayTagManagerWindowArgs Args;
	Args.Title = LOCTEXT("MoodTags", "Mood Tags");
	Args.bRestrictedTags = false;
	Args.Filter = UYapProjectSettings::GetMoodTagsParent().ToString();

	UE::GameplayTags::Editor::OpenGameplayTagManager(Args);

	return FReply::Handled();
}

FReply FDetailCustomization_YapProjectSettings::OnClicked_OpenDialogueTagsManager()
{
	FGameplayTagManagerWindowArgs Args;
	Args.Title = LOCTEXT("DialogueTags", "Dialogue Tags");
	Args.bRestrictedTags = false;
	Args.Filter = UYapProjectSettings::GetDialogueTagsParent().ToString();

	UE::GameplayTags::Editor::OpenGameplayTagManager(Args);

	return FReply::Handled();
}

FReply FDetailCustomization_YapProjectSettings::OnClicked_RefreshMoodTagIcons()
{
	UYapEditorSubsystem::Get()->UpdateMoodTagBrushes();

	return FReply::Handled();
}

FText FDetailCustomization_YapProjectSettings::ToolTipText_DefaultMoodTags() const
{
	const FGameplayTag& ParentTag = UYapProjectSettings::GetMoodTagsParent();

	if (!ParentTag.IsValid())
	{
		return LOCTEXT("MissingParentTag_Warning", "Parent tag needs to be set!");
	}
	
	FString ParentTagString = ParentTag.ToString();

	FString DefaultTagsAsString;
	
	for (int32 i = 0; i < DefaultMoodTags.Num(); ++i)
	{
		const FString& Tag = DefaultMoodTags[i];
					
		DefaultTagsAsString = DefaultTagsAsString + ParentTagString + "." + Tag;

		if (i < DefaultMoodTags.Num() - 1)
		{
			DefaultTagsAsString += "\n";
		}
	}
	
	return FText::Format(LOCTEXT("SetDefaultTags_ToolTip", "Sets the following tags:\n{0}"), FText::FromString(DefaultTagsAsString));
}

bool FDetailCustomization_YapProjectSettings::IsTagPropertySet(TSharedPtr<IPropertyHandle> TagPropertyHandle) const
{
	TArray<void*> RawData;

	TagPropertyHandle->AccessRawData(RawData);

	const FGameplayTag* Tag = reinterpret_cast<const FGameplayTag*>(RawData[0]);

	return Tag->IsValid();
}

#undef LOCTEXT_NAMESPACE
