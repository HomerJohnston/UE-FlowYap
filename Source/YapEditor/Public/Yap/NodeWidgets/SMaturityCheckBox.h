#pragma once

#include "Widgets/SCompoundWidget.h"

namespace YapEditor
{
	TSharedRef<SCheckBox> CreateMaturityCheckBox
	(
		const TAttribute<ECheckBoxState>& IsCheckedAtt,
		const FOnCheckStateChanged& OnCheckStateChangedEvt,
		const TAttribute<FSlateColor>& ColorAndOpacityAtt,
		const TAttribute<EVisibility>& ActiveIndicatorAtt
	);
}