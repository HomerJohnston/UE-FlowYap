#include "YapEditor/SlateWidgets/SYapGameplayTagTypedPicker.h"

#include "GameplayTagsEditorModule.h"
#include "GameplayTagsManager.h"
#include "SGameplayTagPicker.h"
#include "Yap/Globals/YapEditorWarning.h"
#include "Yap/Globals/YapFileUtilities.h"
#include "YapEditor/YapColors.h"
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

void SYapGameplayTagTypedPicker::DeleteTagFromINI()
{
	TSharedPtr<FGameplayTagNode> OldTagNode = UGameplayTagsManager::Get().FindTagNode(Tag.Get());
	
	IGameplayTagsEditorModule::Get().DeleteTagFromINI(OldTagNode);
}

// ------------------------------------------------------------------------------------------------

void SYapGameplayTagTypedPicker::ChangeTag(const FString& NewTagString)
{	
	FText EmptyTagText = LOCTEXT("Tag_None", "<None>");
	FText NewTagText = NewTagString.IsEmpty() ? EmptyTagText : FText::FromString(NewTagString);

	EYapGameplayTagTypedPickerResponse Response = EYapGameplayTagTypedPickerResponse::Unspecified;
	
	if (!RequestTagRedirect(NewTagString, Response).IsEventHandled())
	{
		RequestTagDeletion(NewTagString, Response);
	}

	if (Response == EYapGameplayTagTypedPickerResponse::Cancel)
	{
		return;
	}
	
	FGameplayTag NewTagSource = UGameplayTagsManager::Get().RequestGameplayTag(FName(NewTagString), false);

	if (!NewTagSource.IsValid())
	{
		IGameplayTagsEditorModule::Get().AddNewGameplayTagToINI(NewTagString, "", Yap::FileUtilities::GetTagConfigFileName());
	}

	switch (Response)
	{
		case EYapGameplayTagTypedPickerResponse::RedirectOldTag:
		{
			IGameplayTagsEditorModule& Module = IGameplayTagsEditorModule::Get();

			const FString TagToRename = Tag.Get().ToString();

			if (Module.RenameTagInINI(TagToRename, NewTagString))
			{
				TSharedPtr<FGameplayTagNode> Node = UGameplayTagsManager::Get().FindTagNode(*NewTagString);
				(void)OnTagChanged.ExecuteIfBound(Node->GetCompleteTag());
			}

			break;
		}
		case EYapGameplayTagTypedPickerResponse::DeleteOldTag:
		{
			if (Asset.IsValid())
			{
				FGameplayTag OldTag = Tag.Get();
				
				TSharedPtr<FGameplayTagNode> Node = UGameplayTagsManager::Get().FindTagNode(*NewTagString);
				TSharedPtr<FGameplayTagNode> OldTagNode = UGameplayTagsManager::Get().FindTagNode(OldTag);
				
				(void)OnTagChanged.ExecuteIfBound(Node->GetCompleteTag());
			
				Yap::EditorFuncs::SaveAsset(Asset.Get());

				FTimerHandle TH;
				
				GEditor->GetTimerManager()->SetTimer(TH, [this] ()
				{
					TSharedPtr<FGameplayTagNode> OldTagNode = UGameplayTagsManager::Get().FindTagNode(Tag.Get());
					IGameplayTagsEditorModule::Get().DeleteTagFromINI(OldTagNode);
				}, 0.001, false);
			}

			break;
		}
		default:
		{
			TSharedPtr<FGameplayTagNode> Node = UGameplayTagsManager::Get().FindTagNode(*NewTagString);

			if (Node.IsValid())
			{
				(void)OnTagChanged.ExecuteIfBound(Node->GetCompleteTag());
			}
			
			break;
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

FReply SYapGameplayTagTypedPicker::RequestTagRedirect(const FString& NewTagString, EYapGameplayTagTypedPickerResponse& Response) const
{
	// We can't redirect to or from an empty tag!
	if (NewTagString.IsEmpty() || !Tag.Get().IsValid())
	{
		return FReply::Unhandled();
	}
	
	FText EmptyTagText = LOCTEXT("Tag_None", "<None>");
	
	FText NewTagText = NewTagString.IsEmpty() ? EmptyTagText : FText::FromString(NewTagString);

	FText RedirectText = FText::Format(LOCTEXT("DeleteOldTag_Prompt", "Setting new tag: [{0}]\n\nWould you like to add a redirect from the old tag, [{1}]?\n\nThis will require a restart for the redirect to apply everywhere!"), NewTagText, FText::FromString(Tag.Get().ToString()));

	EAppReturnType::Type RequestRedirectResponse = FMessageDialog::Open(EAppMsgType::YesNoCancel, RedirectText);

	switch (RequestRedirectResponse)
	{
		case EAppReturnType::Yes:
		{			
			Response = EYapGameplayTagTypedPickerResponse::RedirectOldTag;
			return FReply::Handled();
		}
		case EAppReturnType::Cancel:
		{
			Response = EYapGameplayTagTypedPickerResponse::Cancel;
			return FReply::Handled();
		}
		default:
		{
			return FReply::Unhandled();
		}
	}
}

// ------------------------------------------------------------------------------------------------

void SYapGameplayTagTypedPicker::RequestTagDeletion(const FString& NewTagString, EYapGameplayTagTypedPickerResponse& Response) const
{
	// We can't delete an empty tag!
	if (!Tag.Get().IsValid())
	{
		return;
	}

	FText EmptyTagText = LOCTEXT("Tag_None", "<None>");
	
	FText NewTagText = NewTagString.IsEmpty() ? EmptyTagText : FText::FromString(NewTagString);

	FText RedirectText = FText::Format(LOCTEXT("DeleteOldTag_Prompt", "Would you like to attempt to delete the old tag?"), NewTagText, FText::FromString(Tag.Get().ToString()));

	EAppReturnType::Type RequestRedirectResponse = FMessageDialog::Open(EAppMsgType::YesNoCancel, RedirectText);
	
	FGameplayTag OldTag = Tag.Get();
	
	TSharedPtr<FGameplayTagNode> OldTagNode = UGameplayTagsManager::Get().FindTagNode(OldTag);

	switch (RequestRedirectResponse)
	{
		case EAppReturnType::Yes:
		{
			Response = EYapGameplayTagTypedPickerResponse::DeleteOldTag;
			break;
		}
		case EAppReturnType::Cancel:
		{
			Response = EYapGameplayTagTypedPickerResponse::Cancel;
			break;
		}
		default:
		{
			break;
		}
	}
}

void SYapGameplayTagTypedPicker::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	
}

#undef LOCTEXT_NAMESPACE




	/*
	// If we are changing from an old tag to a new tag, figure out if user wants to redirect it, or delete it, or just switch
	if (OldTag.IsValid() && !NewTagString.IsEmpty())
	{
		FText RedirectText = FText::Format(LOCTEXT("DeleteOldTag_Prompt", "Setting new tag: [{0}]\n\nWould you like to add a redirect from the old tag, [{1}]?"), NewTagText, FText::FromString(Tag.Get().ToString()));

		EAppReturnType::Type RequestRedirectResponse = FMessageDialog::Open(EAppMsgType::YesNoCancel, RedirectText);

		switch (RequestRedirectResponse)
        {
        	case EAppReturnType::Yes:
        	{
        		IGameplayTagsEditorModule& Module = IGameplayTagsEditorModule::Get();

        		const FString TagToRename = OldTag.ToString();// GameplayTagNode->GetCompleteTag().GetTagName().ToString();

        		if (Module.RenameTagInINI(TagToRename, NewTagString))
        		{
        			//OnGameplayTagRenamed.ExecuteIfBound(TagToRename, NewTagString);
        		}

        		// We need to save the asset first. The tags manager finds referenced tags through saved assets only. Without doing this it will still think the old tag is in use.
        		//Yap::EditorFuncs::SaveAsset(Asset.Get());
    
        		//IGameplayTagsEditorModule::Get().DeleteTagFromINI(OldTagNode);
        		
        		break;
        	}
        	case EAppReturnType::Cancel:
        	{
        		return;
        	}
			case EAppReturnType::No:
        	{
        		goto RequestDelete;
        		break;
        	}
        }
	}

	return;
	
	RequestDelete:
	if (OldTag.IsValid() && !NewTagString.IsEmpty())
	{
		
	}
	*/
	
	/*
	FYapScopedTransaction Transaction("TODO", INVTEXT("TODO"), nullptr);

	FGameplayTag OldTag = Tag.Get();

	// Check if chosen tag exists, add if required
	if (!NewTagString.IsEmpty())
	{
		FGameplayTag NewTagSource = UGameplayTagsManager::Get().RequestGameplayTag(FName(NewTagString), false);

		if (!NewTagSource.IsValid())
		{
			IGameplayTagsEditorModule::Get().AddNewGameplayTagToINI(NewTagString, "", Yap::FileUtilities::GetTagConfigFileName());
		}
	}

	// Find the actual tag instance and set it
	TSharedPtr<FGameplayTagNode> Node = UGameplayTagsManager::Get().FindTagNode(*NewTagString);

	if (Node.IsValid())
	{
		(void)OnTagChanged.ExecuteIfBound(Node->GetCompleteTag());
	}
	else
	{
		(void)OnTagChanged.ExecuteIfBound(FGameplayTag::EmptyTag);
	}

	UObject* AssetPtr = Asset.Get();

	// Delete the old tag
	if (AssetPtr && OldTag.IsValid())
	{
		TSharedPtr<FGameplayTagNode> OldTagNode = UGameplayTagsManager::Get().FindTagNode(OldTag);

		if (OldTagNode.IsValid())
		{
			FText EmptyTagText = LOCTEXT("Tag_None", "<None>");
			FText NewTagText = NewTagString.IsEmpty() ? EmptyTagText : FText::FromString(NewTagString);
			EAppReturnType::Type Response = FMessageDialog::Open(EAppMsgType::YesNoCancel, FText::Format(LOCTEXT("DeleteOldTag_Prompt", "Setting new tag: {0}\n\nWould you like to attempt to delete old tag: {1}?\n\nNOTE: The asset will be saved if you press yes!"), NewTagText, FText::FromString(Tag.Get().ToString())));
			// Would you like to create tag redirectors for all of the fragments?
			// Would you like to attempt to delete the old tags?
			switch (Response)
			{
				case EAppReturnType::Yes:
				{
					// We need to save the asset first. The tags manager finds referenced tags through saved assets only. Without doing this it will still think the old tag is in use.
					Yap::EditorFuncs::SaveAsset(AssetPtr);
		
					IGameplayTagsEditorModule::Get().DeleteTagFromINI(OldTagNode);
					
					break;
				}
				case EAppReturnType::Cancel:
				{
					return;
				}
				default:
				{
					break;
				}
			}
		}
	}
	*/