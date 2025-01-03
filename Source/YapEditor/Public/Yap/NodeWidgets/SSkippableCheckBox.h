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