// Copyright Epic Games, Inc. All Rights Reserved

#include "Yap/SlateWidgets/SGameplayTagComboFiltered.h"

#include "SGameplayTagPicker.h"
#include "Framework/Application/SlateApplication.h"
#include "ScopedTransaction.h"
#include "Editor.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Yap/FlowYapUtil.h"
#include "Yap/YapEditorStyle.h"
#include "Yap/NodeWidgets/GameplayTagFilteredEditorUtilities.h"
#include "Yap/NodeWidgets/GameplayTagFilteredStyle.h"

#define LOCTEXT_NAMESPACE "GameplayTagCombo"

//------------------------------------------------------------------------------
// SGameplayTagComboFiltered
//------------------------------------------------------------------------------

SLATE_IMPLEMENT_WIDGET(SGameplayTagComboFiltered)
void SGameplayTagComboFiltered::PrivateRegisterAttributes(FSlateAttributeInitializer& AttributeInitializer)
{
	SLATE_ADD_MEMBER_ATTRIBUTE_DEFINITION_WITH_NAME(AttributeInitializer, "Tag", TagAttribute, EInvalidateWidgetReason::Layout);
}

SGameplayTagComboFiltered::SGameplayTagComboFiltered()
	: TagAttribute(*this)
{
}

FSlateColor SGameplayTagComboFiltered::ColorAndOpacity_TagIcon() const
{
	return IsValueValid() ? YapColor::Gray_SemiTrans : YapColor::Gray_Glass;
}

void SGameplayTagComboFiltered::Construct(const FArguments& InArgs)
{
	TagAttribute.Assign(*this, InArgs._Tag);
	Filter = InArgs._Filter;
	SettingsName = InArgs._SettingsName;
	bIsReadOnly = InArgs._ReadOnly;
	OnTagChanged = InArgs._OnTagChanged;
	PropertyHandle = InArgs._PropertyHandle;

	if (PropertyHandle.IsValid())
	{
		PropertyHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &SGameplayTagComboFiltered::RefreshTagsFromProperty));
		RefreshTagsFromProperty();

		if (Filter.IsEmpty())
		{
			Filter = UGameplayTagsManager::Get().GetCategoriesMetaFromPropertyHandle(PropertyHandle);
		}
		bIsReadOnly = PropertyHandle->IsEditConst();
	}
	
	ChildSlot
	.Padding(0, 1, 0, 1)
	[
		/*
		SNew(SHorizontalBox) // Extra box to make the combo hug the chip
						
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[*/
			SAssignNew(ComboButton, SComboButton)
			/*.ComboButtonStyle(FAppStyle::Get(), "SimpleComboButton")*/ .ComboButtonStyle(FGameplayTagFilteredStyle::Get(), "GameplayTags.ComboButton")
			.HasDownArrow(false)
			.ContentPadding(FMargin(3, 0, 3, 0))
			.IsEnabled(this, &SGameplayTagComboFiltered::IsValueEnabled)
			.Clipping(EWidgetClipping::OnDemand)
			.OnMenuOpenChanged(this, &SGameplayTagComboFiltered::OnMenuOpenChanged)
			.OnGetMenuContent(this, &SGameplayTagComboFiltered::OnGetMenuContent)
			.ButtonContent()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(2, 0, 2, 0)
				[
					SNew(STextBlock)
						.Text(this, &SGameplayTagComboFiltered::GetText)
						.ColorAndOpacity(YapColor::DimGray)
						.Font(FCoreStyle::GetDefaultFontStyle("Normal", 10))
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(2, -1, -2, 0)
				[
					SNew(SImage)
						.DesiredSizeOverride(FVector2D(16, 16))
						.Image(FYapEditorStyle::GetImageBrush(YapBrushes.Icon_Tag))
						.ColorAndOpacity(this, &SGameplayTagComboFiltered::ColorAndOpacity_TagIcon)
				]
			]
		//]
	];
}

bool SGameplayTagComboFiltered::IsValueEnabled() const
{
	if (PropertyHandle.IsValid())
	{
		return !PropertyHandle->IsEditConst();
	}

	return !bIsReadOnly;
}

FReply SGameplayTagComboFiltered::OnEditTag() const
{
	FReply Reply = FReply::Handled();
	if (ComboButton->ShouldOpenDueToClick())
	{
		ComboButton->SetIsOpen(true);
		if (TagPicker->GetWidgetToFocusOnOpen())
		{
			Reply.SetUserFocus(TagPicker->GetWidgetToFocusOnOpen().ToSharedRef());
		}
	}
	else
	{
		ComboButton->SetIsOpen(false);
	}
	
	return Reply;
}

bool SGameplayTagComboFiltered::ShowClearButton() const
{
	// Show clear button is we have multiple values, or the tag is other than None.
	if (PropertyHandle.IsValid())
	{
		if (bHasMultipleValues)
		{
			return true;
		}
		const FGameplayTag GameplayTag = TagsFromProperty.IsEmpty() ? FGameplayTag() : TagsFromProperty[0]; 
		return GameplayTag.IsValid();
	}
	const FGameplayTag GameplayTag = TagAttribute.Get();
	return GameplayTag.IsValid();
}

FText SGameplayTagComboFiltered::GetText() const
{
	//return LOCTEXT("GameplayTagCombo", "Pick Tag");

	if (TagAttribute.Get() == FGameplayTag::EmptyTag)
	{
		return FText::GetEmpty();
	}
	
	return FText::FromString(FlowYapUtil::GetFilteredSubTag(Filter, TagAttribute.Get()));
}

FText SGameplayTagComboFiltered::GetToolTipText() const
{
	if (PropertyHandle.IsValid())
	{
		return TagsFromProperty.IsEmpty() ? FText::GetEmpty() : FText::FromName(TagsFromProperty[0].GetTagName());
	}
	return FText::FromName(TagAttribute.Get().GetTagName());
}

bool SGameplayTagComboFiltered::IsSelected() const
{
	// Show in selected state if we have one value and value is valid.
	if (PropertyHandle.IsValid())
	{
		if (bHasMultipleValues)
		{
			return false;
		}
		const FGameplayTag GameplayTag = TagsFromProperty.IsEmpty() ? FGameplayTag() : TagsFromProperty[0]; 
		return GameplayTag.IsValid();
	}
	const FGameplayTag GameplayTag = TagAttribute.Get();
	return GameplayTag.IsValid();
}

FReply SGameplayTagComboFiltered::OnClearPressed()
{
	OnClearTag();
	return FReply::Handled();
}

void SGameplayTagComboFiltered::OnMenuOpenChanged(const bool bOpen) const
{
	if (bOpen && TagPicker.IsValid())
	{
		const FGameplayTag TagToHilight = GetCommonTag();
		TagPicker->RequestScrollToView(TagToHilight);
							
		ComboButton->SetMenuContentWidgetToFocus(TagPicker->GetWidgetToFocusOnOpen());
	}
}

TSharedRef<SWidget> SGameplayTagComboFiltered::OnGetMenuContent()
{
	// If property is not set, well put the edited tag into a container and use that for picking.
	TArray<FGameplayTagContainer> TagContainers;
	if (!PropertyHandle.IsValid())
	{
		const FGameplayTag TagToEdit = TagAttribute.Get();
		TagContainers.Add(FGameplayTagContainer(TagToEdit));
	}

	const bool bIsPickerReadOnly = !IsValueEnabled();
	
	TagPicker = SNew(SGameplayTagPicker)
		.Filter(Filter)
		.SettingsName(SettingsName)
		.ReadOnly(bIsPickerReadOnly)
		.ShowMenuItems(true)
		.MaxHeight(350.0f)
		.MultiSelect(false)
		.OnTagChanged(this, &SGameplayTagComboFiltered::OnTagSelected)
		.Padding(2)
		.PropertyHandle(PropertyHandle)
		.TagContainers(TagContainers);

	if (TagPicker->GetWidgetToFocusOnOpen())
	{
		ComboButton->SetMenuContentWidgetToFocus(TagPicker->GetWidgetToFocusOnOpen());
	}

	return TagPicker.ToSharedRef();
}

void SGameplayTagComboFiltered::OnTagSelected(const TArray<FGameplayTagContainer>& TagContainers)
{
	if (OnTagChanged.IsBound())
	{
		const FGameplayTag NewTag = TagContainers.IsEmpty() ? FGameplayTag() : TagContainers[0].First();
		OnTagChanged.Execute(NewTag);
	}
}

FGameplayTag SGameplayTagComboFiltered::GetCommonTag() const
{
	if (PropertyHandle.IsValid())
	{
		return TagsFromProperty.IsEmpty() ? FGameplayTag() : TagsFromProperty[0]; 
	}
	else
	{
		return TagAttribute.Get();
	}
}

FReply SGameplayTagComboFiltered::OnTagMenu(const FPointerEvent& MouseEvent)
{
	FMenuBuilder MenuBuilder(/*bShouldCloseWindowAfterMenuSelection=*/ true, /*CommandList=*/ nullptr);

	const FGameplayTag GameplayTag = GetCommonTag();
	
	auto IsValidTag = [GameplayTag]()
	{
		return GameplayTag.IsValid();		
	};

	MenuBuilder.AddMenuEntry(
		LOCTEXT("GameplayTagCombo_SearchForReferences", "Search For References"),
		FText::Format(LOCTEXT("GameplayTagCombo_SearchForReferencesTooltip", "Find references to the tag {0}"), FText::AsCultureInvariant(GameplayTag.ToString())),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Search"),
		FUIAction(FExecuteAction::CreateLambda([GameplayTag]()
		{
			// Single tag search
			const FName TagFName = GameplayTag.GetTagName();
			if (FEditorDelegates::OnOpenReferenceViewer.IsBound() && !TagFName.IsNone())
			{
				TArray<FAssetIdentifier> AssetIdentifiers;
				AssetIdentifiers.Emplace(FGameplayTag::StaticStruct(), TagFName);
				FEditorDelegates::OnOpenReferenceViewer.Broadcast(AssetIdentifiers, FReferenceViewerParams());
			}
		}))
		);

	MenuBuilder.AddSeparator();

	MenuBuilder.AddMenuEntry(
	NSLOCTEXT("PropertyView", "CopyProperty", "Copy"),
	FText::Format(LOCTEXT("GameplayTagCombo_CopyTagTooltip", "Copy tag {0} to clipboard"), FText::AsCultureInvariant(GameplayTag.ToString())),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "GenericCommands.Copy"),
		FUIAction(FExecuteAction::CreateSP(this, &SGameplayTagComboFiltered::OnCopyTag, GameplayTag), FCanExecuteAction::CreateLambda(IsValidTag)));

	MenuBuilder.AddMenuEntry(
	NSLOCTEXT("PropertyView", "PasteProperty", "Paste"),
	LOCTEXT("GameplayTagCombo_PasteTagTooltip", "Paste tags from clipboard."),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "GenericCommands.Paste"),
		FUIAction(FExecuteAction::CreateSP(this, &SGameplayTagComboFiltered::OnPasteTag),FCanExecuteAction::CreateSP(this, &SGameplayTagComboFiltered::CanPaste)));

	MenuBuilder.AddMenuEntry(
	LOCTEXT("GameplayTagCombo_ClearTag", "Clear Gameplay Tag"),
		FText::GetEmpty(),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.X"),
		FUIAction(FExecuteAction::CreateSP(this, &SGameplayTagComboFiltered::OnClearTag), FCanExecuteAction::CreateLambda(IsValidTag)));

	// Spawn context menu
	FWidgetPath WidgetPath = MouseEvent.GetEventPath() != nullptr ? *MouseEvent.GetEventPath() : FWidgetPath();
	FSlateApplication::Get().PushMenu(AsShared(), WidgetPath, MenuBuilder.MakeWidget(), MouseEvent.GetScreenSpacePosition(), FPopupTransitionEffect(FPopupTransitionEffect::ContextMenu));

	return FReply::Handled();
}

void SGameplayTagComboFiltered::OnClearTag()
{
	if (PropertyHandle.IsValid())
	{
		FScopedTransaction Transaction(LOCTEXT("GameplayTagCombo_ClearTag", "Clear Gameplay Tag"));
		PropertyHandle->SetValueFromFormattedString(UE::GameplayTagsFiltered::EditorUtilities::GameplayTagExportText(FGameplayTag()));
	}
				
	OnTagChanged.ExecuteIfBound(FGameplayTag());
}

void SGameplayTagComboFiltered::OnCopyTag(const FGameplayTag TagToCopy) const
{
	// Copy tag as a plain string, GameplayTag's import text can handle that.
	// TODO FPlatformApplicationMisc::ClipboardCopy(*TagToCopy.ToString());
}

void SGameplayTagComboFiltered::OnPasteTag()
{
	// TODO
	/*
	FString PastedText;
	FPlatformApplicationMisc::ClipboardPaste(PastedText);
	const FGameplayTag PastedTag = UE::GameplayTagsFiltered::EditorUtilities::GameplayTagTryImportText(PastedText);
	
	if (PastedTag.IsValid())
	{
		if (PropertyHandle.IsValid())
		{
			FScopedTransaction Transaction(LOCTEXT("GameplayTagCombo_PasteTag", "Paste Gameplay Tag"));
			PropertyHandle->SetValueFromFormattedString(PastedText);
			RefreshTagsFromProperty();
		}
		
		OnTagChanged.ExecuteIfBound(PastedTag);
	}
	*/
}

bool SGameplayTagComboFiltered::CanPaste() const
{
	// TODO
	return false;
	//FString PastedText;
	//FPlatformApplicationMisc::ClipboardPaste(PastedText);
	//FGameplayTag PastedTag = UE::GameplayTagsFiltered::EditorUtilities::GameplayTagTryImportText(PastedText);

	//return PastedTag.IsValid();
}

bool SGameplayTagComboFiltered::IsValueValid() const
{
	FGameplayTag TempTag = TagAttribute.Get();

	if (TempTag.IsValid())
	{
		return true;
	}

	return false;
}

void SGameplayTagComboFiltered::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	if (!PropertyHandle.IsValid()
		|| !PropertyHandle->IsValidHandle())
	{
		return;
	}

	// Check if cached data has changed, and update it.
	bool bShouldUpdate = false;
	
	TArray<const void*> RawStructData;
	PropertyHandle->AccessRawData(RawStructData);

	if (RawStructData.Num() == TagsFromProperty.Num())
	{
		for (int32 Idx = 0; Idx < RawStructData.Num(); ++Idx)
		{
			if (RawStructData[Idx])
			{
				const FGameplayTag& CurrTag = *(FGameplayTag*)RawStructData[Idx];
				if (CurrTag != TagsFromProperty[Idx])
				{
					bShouldUpdate = true;
					break;
				}
			}
		}
	}

	if (bShouldUpdate)
	{
		RefreshTagsFromProperty();
	}
}

void SGameplayTagComboFiltered::RefreshTagsFromProperty()
{
	if (PropertyHandle.IsValid()
		&& PropertyHandle->IsValidHandle())
	{
		bHasMultipleValues = false;
		TagsFromProperty.Reset();
		
		SGameplayTagPicker::EnumerateEditableTagContainersFromPropertyHandle(PropertyHandle.ToSharedRef(), [this](const FGameplayTagContainer& TagContainer)
		{
			const FGameplayTag TagFromProperty = TagContainer.IsEmpty() ? FGameplayTag() : TagContainer.First(); 
			if (TagsFromProperty.Num() > 0 && TagsFromProperty[0] != TagFromProperty)
			{
				bHasMultipleValues = true;
			}
			TagsFromProperty.Add(TagFromProperty);

			return true;
		});
	}
}

#undef LOCTEXT_NAMESPACE
