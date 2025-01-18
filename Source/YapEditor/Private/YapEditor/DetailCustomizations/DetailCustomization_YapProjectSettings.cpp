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
}

void FDetailCustomization_YapProjectSettings::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	IDetailCategoryBuilder& Category = DetailBuilder.EditCategory("Mood Tags");
	{
		const UYapProjectSettings* ProjectSettings = GetDefault<UYapProjectSettings>();
		
		TSharedPtr<IPropertyHandle> DialogueTagsParentPropertyHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UYapProjectSettings, DialogueTagsParent)); 
		FProperty* DialogueTagsParentProperty = DialogueTagsParentPropertyHandle->GetProperty();
		
		MoodTagsParentPropertyHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UYapProjectSettings, MoodTagsParent)); 
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
					.OnClicked(this, &FDetailCustomization_YapProjectSettings::OnClicked_OpenDialogueTagsManager)
				];

				continue;
			}
			
			if (Property == MoodTagsParentProperty)
			{
				float VerticalPadding = 3.0;

				float TagLineHeight = 15.0; // This is the height of a single tag name in pixels
				float LineHeightPercentage = 18.0 / TagLineHeight; // Desired row height divided by actual height
				float TotalHeight = FMath::RoundFromZero(DefaultMoodTags.Num() * TagLineHeight * LineHeightPercentage + VerticalPadding * 2.0);
				
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
					.MaxHeight(TotalHeight) 
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
						.IsEnabled(this, &FDetailCustomization_YapProjectSettings::IsMoodTagsParentSet)
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
						.IsEnabled(this, &FDetailCustomization_YapProjectSettings::IsMoodTagsParentSet)
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
						.IsEnabled(this, &FDetailCustomization_YapProjectSettings::IsMoodTagsParentSet)
						.OnClicked(this, &FDetailCustomization_YapProjectSettings::OnClicked_OpenMoodTagsManager)
						.VAlign(VAlign_Center)
						.HAlign(HAlign_Center)
						.Text(LOCTEXT("EditMoodTags", "Edit mood tags"))
						.ToolTipText(LOCTEXT("OpenTagsManager_ToolTip", "Open tags manager"))
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

	for (FGameplayTag ExistingTag : ProjectSettings->GetMoodTags())// DefaultTags.CreateIterator(); It; ++It)
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

bool FDetailCustomization_YapProjectSettings::IsMoodTagsParentSet() const
{
	TArray<void*> RawData;

	MoodTagsParentPropertyHandle->AccessRawData(RawData);

	const FGameplayTag* Tag = reinterpret_cast<const FGameplayTag*>(RawData[0]);

	return Tag->IsValid();
}

#undef LOCTEXT_NAMESPACE
