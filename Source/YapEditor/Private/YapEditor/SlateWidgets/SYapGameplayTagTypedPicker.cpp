#include "YapEditor/SlateWidgets/SYapGameplayTagTypedPicker.h"

#include "GameplayTagsEditorModule.h"
#include "GameplayTagsManager.h"
#include "SGameplayTagPicker.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "Yap/YapLog.h"
#include "Yap/Globals/YapEditorWarning.h"
#include "Yap/Globals/YapFileUtilities.h"
#include "YapEditor/YapColors.h"
#include "YapEditor/YapEditorEvents.h"
#include "YapEditor/YapEditorStyle.h"
#include "YapEditor/YapTransactions.h"
#include "YapEditor/Globals/YapEditorFuncs.h"

#define LOCTEXT_NAMESPACE "YapEditor"

// ------------------------------------------------------------------------------------------------

SYapGameplayTagTypedPicker::SYapGameplayTagTypedPicker()
	: Tag(*this)
{
}

// ------------------------------------------------------------------------------------------------

SLATE_IMPLEMENT_WIDGET(SYapGameplayTagTypedPicker)
void SYapGameplayTagTypedPicker::PrivateRegisterAttributes(struct FSlateAttributeDescriptor::FInitializer& AttributeInitializer)
{
	SLATE_ADD_MEMBER_ATTRIBUTE_DEFINITION_WITH_NAME(AttributeInitializer, "Tag", Tag, EInvalidateWidgetReason::Layout);
}

// ------------------------------------------------------------------------------------------------

void SYapGameplayTagTypedPicker::Construct(const FArguments& InArgs)
{
	Asset = InArgs._Asset;
	
	Tag.Assign(*this, InArgs._Tag);

	Filter = InArgs._Filter;
	
	TagIniFileName = InArgs._TagIniFileName;

	bReadOnly = InArgs._ReadOnly;

	bEnableNavigation = InArgs._EnableNavigation;

	OnTagChanged = InArgs._OnTagChanged;
	
	ChildSlot
	.Padding(0)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.Padding(2, 0, 2, 0)
		.AutoWidth()
		.VAlign(EVerticalAlignment::VAlign_Center)
		[
			SAssignNew(TextEditor, SEditableText)
			.SelectAllTextWhenFocused(true)
			.Text(this, &ThisClass::Text_TagValue)
			.ColorAndOpacity(this, &ThisClass::ColorAndOpacity_TagText)
			.OnTextCommitted(this, &ThisClass::OnTextCommitted)
			.Font(FCoreStyle::GetDefaultFontStyle("Normal", 10))
			.HintText(INVTEXT("\u2014"))
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(EVerticalAlignment::VAlign_Center)
		[
			SAssignNew(ComboButton, SComboButton)
			.ComboButtonStyle(FYapEditorStyle::Get(), YapStyles.ComboButtonStyle_YapGameplayTagTypedPicker)
			.Cursor(EMouseCursor::Default)
			.HasDownArrow(false)
			.ContentPadding(FMargin(3, 0, 3, 0))
			.Clipping(EWidgetClipping::OnDemand)
			.OnMenuOpenChanged(this, &ThisClass::OnMenuOpenChanged)
			.OnGetMenuContent(this, &ThisClass::OnGetMenuContent)
			.ButtonContent()
			[
				SNew(SImage)
				.DesiredSizeOverride(FVector2D(16, 16))
				.Image(FYapEditorStyle::GetImageBrush(YapBrushes.Icon_Tag))
				.ColorAndOpacity(this, &ThisClass::ColorAndOpacity_TagIcon)
			]
		]
	];
}

// ------------------------------------------------------------------------------------------------

bool SYapGameplayTagTypedPicker::ShowClearButton() const
{
	return true;
}

// ------------------------------------------------------------------------------------------------

FText SYapGameplayTagTypedPicker::Text_TagValue() const
{
	if (!Tag.Get().IsValid())
	{
		return FText::GetEmpty();
	}
	
	FString TagAsString = Tag.Get().ToString();

	if (!Filter.IsEmpty() && TagAsString.StartsWith(Filter))
	{
		TagAsString = TagAsString.RightChop(Filter.Len() + 1);
	}
	
	return FText::FromString(TagAsString);
}

// ------------------------------------------------------------------------------------------------

void SYapGameplayTagTypedPicker::OnMenuOpenChanged(bool bOpen)
{
	if (bOpen && TagPicker.IsValid())
	{
		const FGameplayTag TagToHilight = Tag.Get();
		TagPicker->RequestScrollToView(TagToHilight);
							
		ComboButton->SetMenuContentWidgetToFocus(TagPicker->GetWidgetToFocusOnOpen());
	}
}

// ------------------------------------------------------------------------------------------------

TSharedRef<SWidget> SYapGameplayTagTypedPicker::OnGetMenuContent()
{
	// If property is not set, well put the edited tag into a container and use that for picking.
	TArray<FGameplayTagContainer> TagContainers;
	
	const FGameplayTag TagToEdit = Tag.Get();
	TagContainers.Add(FGameplayTagContainer(TagToEdit));

	TagPicker = SNew(SGameplayTagPicker)
		.Filter(Filter)
		.SettingsName(TagIniFileName)
		.ShowMenuItems(true)
		.MaxHeight(350.0f)
		.MultiSelect(false)
		.OnTagChanged(this, &ThisClass::OnTagSelected)
		.Padding(2)
		.TagContainers(TagContainers);

	if (TagPicker->GetWidgetToFocusOnOpen())
	{
		ComboButton->SetMenuContentWidgetToFocus(TagPicker->GetWidgetToFocusOnOpen());
	}

	return TagPicker.ToSharedRef();
}

// ------------------------------------------------------------------------------------------------

FSlateColor SYapGameplayTagTypedPicker::ColorAndOpacity_TagText() const
{
	if (TextEditor->HasKeyboardFocus())
	{
		return YapColor::White;
	}

	/*
	if (!Tag.Get().IsValid())
	{
		return YapColor::DarkGray;
	}
*/
	
	return YapColor::Gray;
}

// ------------------------------------------------------------------------------------------------

void SYapGameplayTagTypedPicker::OnTextCommitted(const FText& NewTag, ETextCommit::Type CommitType)
{
	if (CommitType != ETextCommit::OnEnter)
	{
		return;
	}

	FString NewTagName = NewTag.ToString();
	
	if (!Filter.IsEmpty() && !NewTag.IsEmpty())
	{
		NewTagName = Filter + "." + NewTagName; 
	}

	if (NewTagName == Tag.Get().ToString())
	{
		return;
	}

	ChangeTag(NewTagName);
}

// ------------------------------------------------------------------------------------------------

FSlateColor SYapGameplayTagTypedPicker::ColorAndOpacity_TagIcon() const
{
	return IsValueValid() ? YapColor::Gray_SemiTrans : YapColor::Gray_Glass;
}

// ------------------------------------------------------------------------------------------------

bool SYapGameplayTagTypedPicker::IsValueValid() const
{
	FGameplayTag TempTag = Tag.Get();

	if (TempTag.IsValid())
	{
		return true;
	}

	return false;
}

// ------------------------------------------------------------------------------------------------

void SYapGameplayTagTypedPicker::OnTagSelected(const TArray<FGameplayTagContainer>& TagContainers)
{
	FGameplayTag NewTag = TagContainers[0].First();
	
	ChangeTag(NewTag.ToString());
}

// ------------------------------------------------------------------------------------------------

void SYapGameplayTagTypedPicker::ChangeTag(const FString& NewTagString)
{
	if (!Asset.IsValid())
	{
		// TODO
		UE_LOG(LogYap, Warning, TEXT("SYapGameplayTagTypedPicker has no Asset assigned, this isn't supported!"));
		return;
	}

	// ---- PROBLEM: the stupid asset system doesn't update references immediately upon saving package for some reason?? I need to call a timer to finish off this function.
	// ---- This causes two issues: 1) I can't transact all of it in one shot, 2) I can't as intelligently inform the user what's going on and what their options are before doing it
	// Current order of operation:
	// Start a transaction
	// Make the new tag if it doesn't already exist
	// Set the property to the new tag
	// End the first transaction
	// Save the asset
	// ... delay
	// Check if the tag is still referenced, if so, offer to set up a redirect, if not, offer to delete the old tag
	
	FText EmptyTagText = LOCTEXT("Tag_None", "<None>");
	FText NewTagText = NewTagString.IsEmpty() ? EmptyTagText : FText::FromString(NewTagString);

	FString OldTagString = (Tag.Get().IsValid()) ? Tag.Get().ToString() : "";
	
	{
		FYapScopedTransaction Transaction("TODO", INVTEXT("TODO"), nullptr);

		if (!NewTagString.IsEmpty())
		{
			FGameplayTag NewTagSource = UGameplayTagsManager::Get().RequestGameplayTag(FName(NewTagString), false);

			if (!NewTagSource.IsValid())
			{
				IGameplayTagsEditorModule::Get().AddNewGameplayTagToINI(NewTagString, "", Yap::FileUtilities::GetTagConfigFileName());
			}

			TSharedPtr<FGameplayTagNode> Node = UGameplayTagsManager::Get().FindTagNode(*NewTagString);
			(void)OnTagChanged.ExecuteIfBound(Node->GetCompleteTag());
		}
		else
		{
			(void)OnTagChanged.ExecuteIfBound(FGameplayTag::EmptyTag);
		}
	}

	Yap::EditorFuncs::SaveAsset(Asset.Get());

	FTimerHandle DummyHandle;
	FTimerDelegate Delegate = FTimerDelegate::CreateRaw(this, &ThisClass::PostChangeTag, OldTagString, NewTagString);
	GEditor->GetTimerManager()->SetTimer(DummyHandle, Delegate, 0.1, false); // SHOOT ME FOR THIS
}

// ------------------------------------------------------------------------------------------------

void SYapGameplayTagTypedPicker::PostChangeTag(FString OldTagString, FString NewTagString)
{
	EYapGameplayTagTypedPickerResponse UserChoice = EYapGameplayTagTypedPickerResponse::Unspecified;

	TArray<FAssetIdentifier> References = Yap::EditorFuncs::FindTagReferences(FName(OldTagString));

	if (References.Num() == 0)
	{
		RequestTagDeletion(NewTagString, OldTagString, UserChoice);
	}
	else
	{
		RequestTagRedirect(NewTagString, OldTagString, UserChoice);
	}
	
	switch (UserChoice)
	{
		case EYapGameplayTagTypedPickerResponse::RedirectOldTag:
		{
			IGameplayTagsEditorModule& Module = IGameplayTagsEditorModule::Get();

			if (!Module.RenameTagInINI(OldTagString, NewTagString))
			{
				Yap::EditorFuncs::PostNotificationInfo_Warning(LOCTEXT("GameplayTagRedirectFailure_Title", "Failed to Redirect Tag"), LOCTEXT("GameplayTagRedirectFailure_Description", "Unknown error"));
			}

			return;
		}
		case EYapGameplayTagTypedPickerResponse::DeleteOldTag:
		{
			TSharedPtr<FGameplayTagNode> OldTagNode = UGameplayTagsManager::Get().FindTagNode(Tag.Get());

			if (!OldTagNode.IsValid())
			{
				Yap::EditorFuncs::PostNotificationInfo_Warning(LOCTEXT("GameplayTagDeleteFailure_InvalidNode_Title", "Failed to Delete Tag"), LOCTEXT("GameplayTagDeleteFailure_InvalidNode_Description", "Old tag node was not found"));
				return;
			}

			if (!IGameplayTagsEditorModule::Get().DeleteTagFromINI(OldTagNode))
			{
				Yap::EditorFuncs::PostNotificationInfo_Warning(LOCTEXT("GameplayTagDeleteFailure_Title", "Failed to Delete Tag"), LOCTEXT("GameplayTagDeleteFailure_Description", "Unknown error"));
			}
			
			return;
		}
		default:
		{
			return;
		}
	}
}

// ------------------------------------------------------------------------------------------------

bool SYapGameplayTagTypedPicker::VerifyNewTagString(const FString& NewTagString) const
{
	FText ErrorMsg;
	
	if (!NewTagString.IsEmpty() && !UGameplayTagsManager::Get().IsValidGameplayTagString(NewTagString, &ErrorMsg))
	{
		Yap::Editor::PostNotificationInfo_Warning(LOCTEXT("Tag_Error", "Tag Error"), ErrorMsg);
		return false;
	}
	
	return true;
}

// ------------------------------------------------------------------------------------------------

void SYapGameplayTagTypedPicker::RequestTagRedirect(const FString& NewTagString, const FString& OldTagString, EYapGameplayTagTypedPickerResponse& Response) const
{
	// We can't redirect to or from an empty tag!
	if (NewTagString.IsEmpty() || OldTagString.IsEmpty())
	{
		return;
	}
	
	FText EmptyTagText = LOCTEXT("Tag_None", "<None>");
	
	FText NewTagText = NewTagString.IsEmpty() ? EmptyTagText : FText::FromString(NewTagString);
	FText OldTagText = OldTagString.IsEmpty() ? EmptyTagText : FText::FromString(OldTagString);

	FText RedirectText = FText::Format(LOCTEXT("DeleteOldTag_Prompt", "{0}\n\nWould you like to add a redirect from this old tag to the new one?\n\n*** This will require an editor restart! ***"), OldTagText);
	FText TitleText = LOCTEXT("RedirectTagPrompt_Title", "Redirect Old Gameplay Tag?");

	EAppReturnType::Type RequestRedirectResponse = FMessageDialog::Open(EAppMsgType::YesNo, RedirectText, TitleText);

	switch (RequestRedirectResponse)
	{
		case EAppReturnType::Yes:
		{			
			Response = EYapGameplayTagTypedPickerResponse::RedirectOldTag;
			return;
		}
		default:
		{
			return;
		}
	}
}

// ------------------------------------------------------------------------------------------------

void SYapGameplayTagTypedPicker::RequestTagDeletion(const FString& NewTagString, const FString& OldTagString, EYapGameplayTagTypedPickerResponse& Response) const
{
	// We can't delete an empty tag!
	if (OldTagString.IsEmpty())
	{
		return;
	}

	FText EmptyTagText = LOCTEXT("Tag_None", "<None>");
	
	FText NewTagText = NewTagString.IsEmpty() ? EmptyTagText : FText::FromString(NewTagString);

	FText RedirectText = FText::Format(LOCTEXT("DeleteOldTag_Prompt", "{0}\n\nThis tag isn't referenced anywhere anymore. Would you like to delete it?"), FText::FromString(OldTagString));
	FText TitleText = LOCTEXT("DeleteTagPrompt_Title", "Delete Old Gameplay Tag?");

	EAppReturnType::Type RequestRedirectResponse = FMessageDialog::Open(EAppMsgType::YesNo, RedirectText, TitleText);
	
	FGameplayTag OldTag = Tag.Get();
	
	switch (RequestRedirectResponse)
	{
		case EAppReturnType::Yes:
		{
			Response = EYapGameplayTagTypedPickerResponse::DeleteOldTag;
			return;
		}
		default:
		{
			return;
		}
	}
}

void SYapGameplayTagTypedPicker::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	
}

#undef LOCTEXT_NAMESPACE
