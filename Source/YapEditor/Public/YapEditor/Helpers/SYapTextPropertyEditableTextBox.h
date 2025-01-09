// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license.

#pragma once

#include "Containers/Array.h"
#include "Containers/BitArray.h"
#include "Containers/Set.h"
#include "Containers/SparseArray.h"
#include "Containers/UnrealString.h"
#include "CoreMinimal.h"
#include "STextPropertyEditableTextBox.h"
#include "Delegates/Delegate.h"
#include "Fonts/SlateFontInfo.h"
#include "HAL/Platform.h"
#include "HAL/PlatformCrt.h"
#include "Input/Reply.h"
#include "Internationalization/Text.h"
#include "Layout/Visibility.h"
#include "Misc/Attribute.h"
#include "Misc/Optional.h"
#include "Styling/AppStyle.h"
#include "Styling/CoreStyle.h"
#include "Styling/ISlateStyle.h"
#include "Styling/SlateColor.h"
#include "Styling/SlateTypes.h"
#include "Styling/SlateWidgetStyleAsset.h"
#include "Templates/SharedPointer.h"
#include "Templates/TypeHash.h"
#include "Templates/UnrealTemplate.h"
#include "Types/SlateEnums.h"
#include "Types/SlateStructs.h"
#include "UObject/NameTypes.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/SWidget.h"
#include "Widgets/Views/SListView.h"
#include "YapEditor/YapEditorStyle.h"

class SComboButton;
class SEditableTextBox;
class SMultiLineEditableTextBox;
class SSearchBox;
class SWidget;
class UObject;
class UPackage;
struct FFocusEvent;
struct FGeometry;
struct FSlateBrush;

#define LOCTEXT_NAMESPACE "YapEditor"

/** A widget that can be used for editing FText instances */
class SYapTextPropertyEditableTextBox : public SCompoundWidget
{
	SLATE_BEGIN_ARGS(SYapTextPropertyEditableTextBox)
		: _Style(&FYapEditorStyle::Get().GetWidgetStyle<FEditableTextBoxStyle>(YapStyles.EditableTextBoxStyle_Dialogue))
		, _Font()
		, _ForegroundColor()
		, _WrapTextAt(0.0f)
		, _AutoWrapText(false)
		, _MinDesiredWidth()
		, _MaxDesiredHeight(300.0f)
		, _Text()
		, _OnTextCommitted()
		{}
		/** The styling of the textbox */
		SLATE_STYLE_ARGUMENT(FEditableTextBoxStyle, Style)
		/** Font color and opacity (overrides Style) */
		SLATE_ATTRIBUTE(FSlateFontInfo, Font)
		/** Text color and opacity (overrides Style) */
		SLATE_ATTRIBUTE(FSlateColor, ForegroundColor)
		/** Whether text wraps onto a new line when it's length exceeds this width; if this value is zero or negative, no wrapping occurs */
		SLATE_ATTRIBUTE(float, WrapTextAt)
		/** Whether to wrap text automatically based on the widget's computed horizontal space */
		SLATE_ATTRIBUTE(bool, AutoWrapText)
		/** When specified, will report the MinDesiredWidth if larger than the content's desired width */
		SLATE_ATTRIBUTE(FOptionalSize, MinDesiredWidth)
		/** When specified, will report the MaxDesiredHeight if smaller than the content's desired height */
		SLATE_ATTRIBUTE(FOptionalSize, MaxDesiredHeight)
		/**  */
		SLATE_ATTRIBUTE(FText, Text)
		/**  */
		SLATE_EVENT(FOnTextCommitted, OnTextCommitted)
	SLATE_END_ARGS()
	
public:
	void Construct(const FArguments& Arguments, const TSharedRef<IEditableTextProperty>& InEditableTextProperty);
	virtual bool SupportsKeyboardFocus() const override;
	virtual FReply OnFocusReceived(const FGeometry& MyGeometry, const FFocusEvent& InFocusEvent) override;

private:
	void GetDesiredWidth(float& OutMinDesiredWidth, float& OutMaxDesiredWidth);
	bool CanEdit() const;
	bool IsCultureInvariantFlagEnabled() const;
	bool IsSourceTextReadOnly() const;
	bool IsIdentityReadOnly() const;
	FText GetToolTipText() const;
	bool IsTextLocalizable() const;

	FText GetTextValue() const;
	void OnTextChanged(const FText& NewText);
	void OnTextCommitted(const FText& NewText, ETextCommit::Type CommitInfo);
	void SetTextError(const FText& InErrorMsg);

	FText GetNamespaceValue() const;
	void OnNamespaceChanged(const FText& NewText);
	void OnNamespaceCommitted(const FText& NewText, ETextCommit::Type CommitInfo);

	FText GetKeyValue() const;
#if USE_STABLE_LOCALIZATION_KEYS
	void OnKeyChanged(const FText& NewText);
	void OnKeyCommitted(const FText& NewText, ETextCommit::Type CommitInfo);

	FText GetPackageValue() const;
#endif // USE_STABLE_LOCALIZATION_KEYS

	ECheckBoxState GetLocalizableCheckState() const;

	void HandleLocalizableCheckStateChanged(ECheckBoxState InCheckboxState);

	FText GetAdvancedTextSettingsComboToolTip() const;
	const FSlateBrush* GetAdvancedTextSettingsComboImage() const;

	bool IsValidIdentity(const FText& InIdentity, FText* OutReason = nullptr, const FText* InErrorCtx = nullptr) const;

	TSharedPtr<IEditableTextProperty> EditableTextProperty;

	TSharedPtr<class SWidget> PrimaryWidget;

	TSharedPtr<SMultiLineEditableTextBox> MultiLineWidget;

	TSharedPtr<SEditableTextBox> SingleLineWidget;

	TSharedPtr<SEditableTextBox> NamespaceEditableTextBox;

	TSharedPtr<SEditableTextBox> KeyEditableTextBox;

	TAttribute<FText> BoundText;
	
	bool bIsMultiLine = false;

	static FText MultipleValuesText;

	bool bChanged = false;
};

#undef LOCTEXT_NAMESPACE