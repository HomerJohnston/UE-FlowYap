// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license.

#pragma once

#include "Widgets/Input/SCheckBox.h"

#define LOCTEXT_NAMESPACE "YapEditor"

DECLARE_DELEGATE_OneParam( FOnCheckStateChanged, ECheckBoxState );

enum class EYapDialogueSkippable : uint8;

/** Simple wrapper for checkbox, for reuse in multiple places. */
class YAPEDITOR_API SYapSkippableCheckBox : public SCheckBox
{
	SLATE_BEGIN_ARGS( SYapSkippableCheckBox ) :
		_OnCheckStateChanged()
	{}

		/**  */
		SLATE_ATTRIBUTE( bool, IsSkippable )
		
		/**  */
		SLATE_ATTRIBUTE( EYapDialogueSkippable, SkippableSetting )
			
		/**  */
		SLATE_EVENT( FOnCheckStateChanged, OnCheckStateChanged )
			
	SLATE_END_ARGS()

	void Construct( const FArguments& InArgs );

	TAttribute<bool> IsSkippable;
	TAttribute<EYapDialogueSkippable> SkippableSetting;
	
	FSlateColor ColorAndOpacity_SkipIcon() const;
	FSlateColor ColorAndOpacity_NoEntryIcon() const;
	EVisibility Visibility_NoSkipIcon() const;

	ECheckBoxState SkippableIsChecked() const;
};

#undef LOCTEXT_NAMESPACE