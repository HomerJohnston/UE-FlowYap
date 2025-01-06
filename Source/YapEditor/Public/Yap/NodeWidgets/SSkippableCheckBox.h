// Copyright Ghost Pepper Games, Inc. All Rights Reserved.
// This work is MIT-licensed. Feel free to use it however you wish, within the confines of the MIT license.

#pragma once

#include "Widgets/SCompoundWidget.h"

namespace YapEditor
{
	TSharedRef<SCheckBox> CreateSkippableCheckBox
	(
		const TAttribute<ECheckBoxState>& IsCheckedAtt,
		const FOnCheckStateChanged& OnCheckStateChangedEvt,
		const TAttribute<FSlateColor>& ColorAndOpacityAtt,
		const TAttribute<EVisibility>& VisibilityOffIconAtt
	);
}