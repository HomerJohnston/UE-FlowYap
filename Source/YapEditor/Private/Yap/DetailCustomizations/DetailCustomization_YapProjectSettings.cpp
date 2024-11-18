#include "Yap/DetailCustomizations/DetailCustomization_YapProjectSettings.h"

#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "GameplayTagsEditorModule.h"
#include "SGameplayTagCombo.h"
#include "SGameplayTagContainerCombo.h"
#include "SGameplayTagPicker.h"
#include "Yap/FlowYapTransactions.h"
#include "Yap/YapProjectSettings.h"

FText FDetailCustomization_YapProjectSettings::GetMoodTags() const
{
	const UYapProjectSettings* ProjectSettings = GetDefault<UYapProjectSettings>();
	
	FGameplayTag ParentTag = ProjectSettings->MoodTagsParent;

	if (!ParentTag.IsValid())
	{
		return INVTEXT("<None>");
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

		FGameplayTag ParentTag = ProjectSettings->MoodTagsParent;
		
		for (TSharedPtr<IPropertyHandle> PropertyHandle : Properties)
		{
			FProperty* Property = PropertyHandle->GetProperty();
			
			Category.AddProperty(PropertyHandle);

			if (Property == DialogueTagsParentProperty)
			{
				Category.AddCustomRow(INVTEXT("TODO Test"))
				.ValueContent()
				[
					SNew(SButton)
					.VAlign(VAlign_Center)
					.HAlign(HAlign_Center)
					.ToolTipText(INVTEXT("Open tags manager"))
					.OnClicked_Lambda([this]()
					{
						FGameplayTagManagerWindowArgs Args;
						Args.Title = INVTEXT("Dialogue Tags");
						Args.bRestrictedTags = false;
						Args.Filter = GetDefault<UYapProjectSettings>()->DialogueTagsParent.ToString();
						UE::GameplayTags::Editor::OpenGameplayTagManager(Args);
						return FReply::Handled();
					})
					[
						SNew(SHorizontalBox)
						+SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(FMargin(0,0,4,0))
						[
							SNew(STextBlock)
							.Text(INVTEXT("Edit Dialogue Tags"))
						]
					]
				];
			}
			else if (Property == MoodTagsParentProperty)
			{
				Category.AddCustomRow(INVTEXT("TODO Test"))
				.NameContent()
				[
					SNew(STextBlock)
					.Text(INVTEXT("Mood Tags"))
				]
				.ValueContent()
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
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
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(SBox)
						.Visibility(this, &FDetailCustomization_YapProjectSettings::Visibility_AddDefaultMoodTags)
						[
							SNew(SButton)
							.OnClicked(this, &FDetailCustomization_YapProjectSettings::OnClicked_AddDefaultMoodTags)
							.ToolTipText(INVTEXT("Adds the following tags:\nAngry\nCalm\nConfused\nDisgusted\nDoubtful\nHappy\nInjured\nLaughing\nPanicked\nSad\nScared\nStressed\nSurprised"))
							[
								SNew(STextBlock)
								.Text(INVTEXT("Add default values..."))
							]
						]
					]
				];

				Category.AddCustomRow(INVTEXT("TODO Test"))
				.ValueContent()
				[
					SNew(SButton)
					.VAlign(VAlign_Center)
					.HAlign(HAlign_Center)
					.ToolTipText(INVTEXT("Open tags manager"))
					.OnClicked_Lambda([this]()
					{
						FGameplayTagManagerWindowArgs Args;
						Args.Title = INVTEXT("Mood Tags");
						Args.bRestrictedTags = false;
						Args.Filter = GetDefault<UYapProjectSettings>()->MoodTagsParent.ToString();
						UE::GameplayTags::Editor::OpenGameplayTagManager(Args);
						return FReply::Handled();
					})
					[
						SNew(SHorizontalBox)
						+SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(FMargin(0,0,4,0))
						[
							SNew(STextBlock)
							.Text(INVTEXT("Edit Mood Tags"))
						]
					]
				];
			}
		}
	}
}

EVisibility FDetailCustomization_YapProjectSettings::Visibility_AddDefaultMoodTags() const
{
	return (GetDefault<UYapProjectSettings>()->GetMoodTags().Num() == 0) ? EVisibility::Visible : EVisibility::Collapsed;
}

FReply FDetailCustomization_YapProjectSettings::OnClicked_AddDefaultMoodTags() const
{
	FFlowYapTransactions::BeginModify(INVTEXT("YapProjectSettings"), GetMutableDefault<UYapProjectSettings>());
	
	IGameplayTagsEditorModule::Get().AddNewGameplayTagToINI("Yap.Mood.Angry", "", "YapGameplayTags.ini");
	IGameplayTagsEditorModule::Get().AddNewGameplayTagToINI("Yap.Mood.Angry", "", "YapGameplayTags.ini");
	IGameplayTagsEditorModule::Get().AddNewGameplayTagToINI("Yap.Mood.Calm", "", "YapGameplayTags.ini");
	IGameplayTagsEditorModule::Get().AddNewGameplayTagToINI("Yap.Mood.Confused", "", "YapGameplayTags.ini");
	IGameplayTagsEditorModule::Get().AddNewGameplayTagToINI("Yap.Mood.Disgusted", "", "YapGameplayTags.ini");
	IGameplayTagsEditorModule::Get().AddNewGameplayTagToINI("Yap.Mood.Doubtful", "", "YapGameplayTags.ini");
	IGameplayTagsEditorModule::Get().AddNewGameplayTagToINI("Yap.Mood.Happy", "", "YapGameplayTags.ini");
	IGameplayTagsEditorModule::Get().AddNewGameplayTagToINI("Yap.Mood.Injured", "", "YapGameplayTags.ini");
	IGameplayTagsEditorModule::Get().AddNewGameplayTagToINI("Yap.Mood.Laughing", "", "YapGameplayTags.ini");
	IGameplayTagsEditorModule::Get().AddNewGameplayTagToINI("Yap.Mood.Panicked", "", "YapGameplayTags.ini");
	IGameplayTagsEditorModule::Get().AddNewGameplayTagToINI("Yap.Mood.Sad", "", "YapGameplayTags.ini");
	IGameplayTagsEditorModule::Get().AddNewGameplayTagToINI("Yap.Mood.Scared", "", "YapGameplayTags.ini");
	IGameplayTagsEditorModule::Get().AddNewGameplayTagToINI("Yap.Mood.Stressed", "", "YapGameplayTags.ini");
	IGameplayTagsEditorModule::Get().AddNewGameplayTagToINI("Yap.Mood.Surprised", "", "YapGameplayTags.ini");

	FFlowYapTransactions::EndModify();
	
	return FReply::Handled();
}
