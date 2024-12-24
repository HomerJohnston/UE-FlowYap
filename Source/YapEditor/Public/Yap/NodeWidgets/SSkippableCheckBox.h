#pragma once

#include "Widgets/SCompoundWidget.h"

TSharedRef<SCheckBox> CreateSkippableCheckbox
(
	const TAttribute<ECheckBoxState>& IsCheckedAtt,
	const FOnCheckStateChanged& OnCheckStateChangedEvt,
	const TAttribute<FSlateColor>& ColorAndOpacityAtt,
	const TAttribute<EVisibility>& VisibilityOffIconAtt
);
