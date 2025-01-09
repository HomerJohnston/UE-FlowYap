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

#define LOCTEXT_NAMESPACE "YapEditor"

FText FDetailCustomization_YapProjectSettings::GetMoodTags() const
{
	const UYapProjectSettings* ProjectSettings = GetDefault<UYapProjectSettings>();
	
	FGameplayTag ParentTag = ProjectSettings->MoodTagsParent;

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
}

void FDetailCustomization_YapProjectSettings::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	IDetailCategoryBuilder& Category = DetailBuilder.EditCategory("Tags");
	{
		const UYapProjectSettings* ProjectSettings = GetDefault<UYapProjectSettings>();
		
		TSharedPtr<IPropertyHandle> DialogueTagsParentPropertyHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UYapProjectSettings, DialogueTagsParent)); 
		FProperty* DialogueTagsParentProperty = DialogueTagsParentPropertyHandle->GetProperty();
		
		TSharedPtr<IPropertyHandle> MoodTagsParentPropertyHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UYapProjectSettings, MoodTagsParent)); 
		FProperty* MoodTagsParentProperty = MoodTagsParentPropertyHandle->GetProperty();
		
		DialogueTagsParentPropertyHandle->MarkHiddenByCustomization();
		MoodTagsParentPropertyHandle->MarkHiddenByCustomization();

		TArray<TSharedRef<IPropertyHandle>> Properties;
		Category.GetDefaultProperties(Properties, true, true);

		for (TSharedPtr<IPropertyHandle> PropertyHandle : Properties)
		{
			FProperty* Property = PropertyHandle->GetProperty();
			
			Category.AddProperty(PropertyHandle);

			if (Property == DialogueTagsParentProperty)
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
					.OnClicked(this, &FDetailCustomization_YapProjectSettings::OnClicked_OpenTagsManager, LOCTEXT("DialogueTags", "Dialogue Tags"), ProjectSettings->DialogueTagsParent.ToString())
				];

				continue;
			}
			
			if (Property == MoodTagsParentProperty)
			{
				float VerticalPadding = 3;

				FString DefaultMoodTagsToolTip = "Sets the following tags:\n" + FString::Join(DefaultMoodTags, TEXT("\n"));
				
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
					.MaxHeight(200)
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
								.LineHeightPercentage(1.25)
							]
						]
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0, VerticalPadding)
					[
						SNew(SButton)
						.OnClicked(this, &FDetailCustomization_YapProjectSettings::OnClicked_ResetDefaultMoodTags)
						.Text(LOCTEXT("ResetMoodTags_Button", "Reset to defaults..."))
						.ToolTipText(FText::FromString(DefaultMoodTagsToolTip))
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0, VerticalPadding)
					[
						SNew(SButton)
						.OnClicked(this, &FDetailCustomization_YapProjectSettings::OnClicked_DeleteAllMoodTags)
						.Text(LOCTEXT("DeleteMoodTags_Button", "Delete all..."))
						.ToolTipText(LOCTEXT("DeleteMoodTags_ToolTip", "Attempts to delete all tags"))
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0, VerticalPadding)
					[
						SNew(SButton)
						.VAlign(VAlign_Center)
						.HAlign(HAlign_Center)
						.ToolTipText(LOCTEXT("OpenTagsManager_ToolTip", "Open tags manager"))
						.OnClicked(this, &FDetailCustomization_YapProjectSettings::OnClicked_OpenTagsManager, LOCTEXT("MoodTags", "Mood Tags"), ProjectSettings->MoodTagsParent.ToString())
						.Text(LOCTEXT("EditMoodTags", "Edit mood tags"))
					]
				];

				continue;
			}
		}
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

	FString DefaultTagParent = "Yap.Mood";
	
	FName IniFile = "YapGameplayTags.ini";

	// Remove any tags that should not be present
	for (FGameplayTag ExistingTag : ProjectSettings->GetMoodTags())// DefaultTags.CreateIterator(); It; ++It)
	{
		if (!ExistingTag.IsValid())
		{
			continue;
		}
		
		FString ExistingTagString = ExistingTag.ToString();
		
		if (!DefaultMoodTags.Contains(ExistingTagString))
		{
			TSharedPtr<FGameplayTagNode> ExistingTagNode = UGameplayTagsManager::Get().FindTagNode(ExistingTag);
			IGameplayTagsEditorModule::Get().DeleteTagFromINI(ExistingTagNode);
		}
	}
	
	// Make sure all of the default tags exist
	for (const FString& Tag : DefaultMoodTags)
	{
		FGameplayTag ExistingTag = UGameplayTagsManager::Get().RequestGameplayTag(FName(Tag), false);

		if (!ExistingTag.IsValid())
		{
			IGameplayTagsEditorModule::Get().AddNewGameplayTagToINI(Tag, "", IniFile);
		}
	}

	FYapTransactions::EndModify();
	
	return FReply::Handled();
}

FReply FDetailCustomization_YapProjectSettings::OnClicked_DeleteAllMoodTags() const
{
	if ( EAppReturnType::Yes != FMessageDialog::Open(EAppMsgType::YesNo, LOCTEXT("AreYouSure_Question", "Are you sure?")) )
	{
		return FReply::Handled();
	}
	
	UYapProjectSettings* ProjectSettings = GetMutableDefault<UYapProjectSettings>();

	FYapTransactions::BeginModify(LOCTEXT("DeleteMoodTags", "Delete mood tags"), ProjectSettings);

	for (FGameplayTag ExistingTag : ProjectSettings->GetMoodTags())// DefaultTags.CreateIterator(); It; ++It)
	{
		TSharedPtr<FGameplayTagNode> ExistingTagNode = UGameplayTagsManager::Get().FindTagNode(ExistingTag);
		IGameplayTagsEditorModule::Get().DeleteTagFromINI(ExistingTagNode);
	}
	
	FYapTransactions::EndModify();

	return FReply::Handled();
}

FReply FDetailCustomization_YapProjectSettings::OnClicked_OpenTagsManager(FText Title, FString Filter)
{
	FGameplayTagManagerWindowArgs Args;
	Args.Title = Title;
	Args.bRestrictedTags = false;
	Args.Filter = Filter;

	/*CurrentYapTagPicker = */UE::GameplayTags::Editor::OpenGameplayTagManager(Args);

	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE